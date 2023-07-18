#pragma once

#include "IPlug_include_in_plug_hdr.h"
#include <elementary/runtime/Runtime.h>

using namespace iplug;

const int kNumPresets = 0;

enum EParams
{
  kGain = 0,
  kBypass = 1,
  kNumParams
};

enum EControlTags
{
  kPeakLeft = 0,
  kPeakRight = 1
};

class IPlugElementary final : public Plugin
{
public:
  IPlugElementary(const InstanceInfo& info);
  
  void ProcessBlock(sample** inputs, sample** outputs, int nFrames) override;
  void OnReset() override;
  void OnIdle() override;
  bool OnMessage(int msgTag, int ctrlTag, int dataSize, const void* pData) override;
  void OnMessageFromWebView(const char* jstonStr) override;
  void SendParameterValueFromDelegate(int paramidx, double value, bool normalized) override;
  void OnParamChange(int paramIdx) override;
  void OnParamChangeUI(int paramIdx, EParamSource) override;
  void OnUIOpen() override;
  void OnRestoreState() override;

private:
  float mPeakLeft = 0.;
  float mPeakRight = 0.;
  int lastKnownBlockSize;
  double lastKnownSampleRate;
  std::unique_ptr<elem::Runtime<sample>> runtime;

  void UpdateParameters();
  void InitElementaryEngine(double sampleRate, int blockSize);
  sample** CopyAudioBlock(sample** block, int numChannels, int blockSize);
};
