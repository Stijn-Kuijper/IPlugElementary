#include "IPlugElementary.h"
#include "IPlug_include_in_plug_src.h"

IPlugElementary::IPlugElementary(const InstanceInfo& info)
: Plugin(info, MakeConfig(kNumParams, kNumPresets))
{
  GetParam(kGain)->InitGain("Gain", -10., -70, 0.);
  GetParam(kBypass)->InitBool("Bypass", false);

  // Hard-coded paths must be modified!
  mEditorInitFunc = [&]() {
#ifdef OS_WIN
//    LoadFile(R"(C:\Users\oli\Dev\iPlug2\Examples\IPlugElementary\resources\web\index.html)", nullptr);
    LoadURL("http://localhost:5173");             // for development
#else
//    LoadFile("dist/index.html", GetBundleID()); // for production
    LoadURL("http://localhost:5173");             // for development
#endif
    
    EnableScroll(false);
  };
  
}

void IPlugElementary::ProcessBlock(sample** inputs, sample** outputs, int nFrames)
{
  // check if the Elementary engine exists
  if (runtime == nullptr)
    return;
  
  // We copy the initial buffer to separate it from the output buffer
  //   otherwise the buffer will clobber itself
  sample** scratchBlock = CopyAudioBlock(inputs, 2, nFrames);
  // tell Elementary to process
  runtime->process(const_cast<const sample**>(scratchBlock), 2, outputs, 2, nFrames);
  
  // Some (very terrible) level metering code, written in C++ to show
  //   how you can still write C++ DSP and combine it with Elementary
  sample maxValL = 0.;
  sample maxValR = 0.;
  for (int s = 0; s < nFrames; s++)
  {
    if (std::fabs(outputs[0][s]) > maxValL) maxValL = std::fabs(outputs[0][s]);
    if (std::fabs(outputs[1][s]) > maxValR) maxValR = std::fabs(outputs[1][s]);
  }
  mPeakLeft = static_cast<float>(maxValL);
  mPeakRight = static_cast<float>(maxValR);
}

void IPlugElementary::OnReset()
{
  auto sr = GetSampleRate();
  auto bs = GetBlockSize();
  // In case either our sample rate or block size changes,
  //   we should create a new Elementary engine with the new settings
  if (sr != lastKnownSampleRate || bs != lastKnownBlockSize)
  {
    lastKnownSampleRate = sr;
    lastKnownBlockSize = bs;
    InitElementaryEngine(sr, bs);
  }
}

bool IPlugElementary::OnMessage(int msgTag, int ctrlTag, int dataSize, const void* pData)
{
  if (msgTag == 0) // I use msgTag == 0 for communicating Elementary rendering instructions
  {
    if (runtime == nullptr)
      return false;
    const char* charPtr = reinterpret_cast<const char*>(pData);
    std::string str(charPtr);
    runtime->applyInstructions(elem::js::parseJSON(str));
  }
  else if (msgTag == 1) // msgTag == 1 for initializing new runtime
  {
    auto sr = GetSampleRate();
    auto bs = GetBlockSize();
    InitElementaryEngine(sr, bs);
  }
  
  return false;
}

// We override the OnMessageFromWebView function, which is the function
//   that sorts the different kind of messages coming in and calls the
//   the appropriate method in the Plugin class
// The reason we override this is because iPlug expects the message to
//   be base64 encoded and will then decode it itself.
// However, this gives rise to some issues, which we can circumvent by
//   simply sending the json string directly with a special msg value.
void IPlugElementary::OnMessageFromWebView(const char* jsonStr)
{
  // call the overridden method to make sure other messages arrive properly
  WebViewEditorDelegate::OnMessageFromWebView(jsonStr);
  auto json = nlohmann::json::parse(jsonStr, nullptr, false);
  // send the string directly (without the base64 encoding/decoding) if the msg is ELEMRENDER
  if (json["msg"] == "ELEMRENDER") {
    // make sure the data is in the right format
    if (json.count("data") > 0 && json["data"].is_string())
    {
      std::string dStr = json["data"].get<std::string>();
      // forward the data; this will eventually call the OnMessage method above
      SendArbitraryMsgFromUI(json["msgTag"], json["ctrlTag"], static_cast<int>(dStr.size()), dStr.data());
    }
  }
}

// Send the measured peak levels when there is time to do so
void IPlugElementary::OnIdle()
{
  if (mPeakLeft  > 0.01) SendControlValueFromDelegate(0, mPeakLeft);
  if (mPeakRight > 0.01) SendControlValueFromDelegate(1, mPeakRight);
}

// Whenever the value of a parameter gets changed, there are two
//   things we must update in the JS environment:
// 1. Our Elementary audio graph
// 2. Our UI
// These concerns should be separated and for the Elementary part
//   we want the update call to happen as accurately as possible
// For the UI part, we do not care so much.
// Luckily, iPlug already makes this distrinction for us.
// There are two methods we can override: OnParamChange and OnParamChangeUI
// The former should only update DSP and can sometimes be called on the main/audio thread
// The latter will only be called from other threads and should
//   therefore be used to update the UI.
void IPlugElementary::OnParamChange(int paramIdx)
{
  WDL_String str;
  str.SetFormatted(mMaxJSStringLength, "onParamChange(%i, %f)", paramIdx, GetParam(paramIdx)->Value());
  EvaluateJavaScript(str.Get());
}

void IPlugElementary::OnParamChangeUI(int paramIdx, EParamSource source = kUnknown)
{
  WDL_String str;
  str.SetFormatted(mMaxJSStringLength, "onParamChangeUI(%i, %f)", paramIdx, GetParam(paramIdx)->Value());
  EvaluateJavaScript(str.Get());
}

// iPlug has built in functionality for sending parameter values to the JS
//   environment when working with the Webview UI.
// However, this is only meant for updating the UI and since we also need
//   to update part of our DSP when a parameter value changes, we send these
//   messages manually with the two methods above.
// Hence, we override and simply return iPlug's built in method for parameter value
//   forwarding.
void IPlugElementary::SendParameterValueFromDelegate(int paramIdx, double value, bool normalized)
{
  return;
}

// The following two methods will be called at times where multiple parameter
//   values might have changed / should be updated in the UI/DSP
void IPlugElementary::OnUIOpen()
{
  UpdateParameters();
}

void IPlugElementary::OnRestoreState()
{
  UpdateParameters();
}

// *********************
// ** Private methods **
// *********************
void IPlugElementary::UpdateParameters()
{
  for (int i = 0; i < NParams(); ++i)
  {
    OnParamChange(i);
    OnParamChangeUI(i);
  }
}

// This methods will create an instance of the Elementary runtime within C++
//   and then make a call to the JS code to tell it to create an instance of
//   the Elementary core within JS.
void IPlugElementary::InitElementaryEngine(double sampleRate, int blockSize)
{
  runtime = std::make_unique<elem::Runtime<sample>>(sampleRate, blockSize);
  WDL_String str;
  str.SetFormatted(mMaxJSStringLength, "initElementary(%f)", sampleRate);
  EvaluateJavaScript(str.Get());
  // We add an additional UpdateParameters call after initializing the Elementary core
  //   to ensure that we will use the correct parameter values.
}

sample** IPlugElementary::CopyAudioBlock(sample** block, int numChannels, int blockSize)
{
  // create a new buffer with the same dimension
  sample** copy = new sample*[numChannels];
  for (int i = 0; i < numChannels; i++)
  {
    // populate the copy with the values from the original
    copy[i] = new sample[blockSize];
    std::copy(block[i], block[i] + blockSize, copy[i]);
  }
  return copy;
}
