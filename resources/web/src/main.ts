import './app.css'
import App from './App.svelte'
import { Renderer, el } from '@elemaudio/core'
import { reconcile } from './audio/main';
import { bypass, gain, meterLeft, meterRight } from './ui/stores/stores';
import { get } from 'svelte/store';

// *************************
// ** iPlug specific code **
// *************************
// @ts-ignore
function SPVFD(paramIdx, val) { /*console.log(`SPVFD called with parameters ${paramIdx}, ${val}`);*/ }
// @ts-ignore
function SCVFD(ctrlTag, val) { 
    if (ctrlTag === 0) meterLeft.set(val);
    if (ctrlTag === 1) meterRight.set(val);
}
// @ts-ignore
function SCMFD(ctrlTag, msgTag, msg) { console.log(`SCMFD called with parameters ${ctrlTag}, ${msgTag}, ${msg}`); }
// @ts-ignore
function SAMFD(msgTag, dataSize, msg) { console.log(`SAMFD called with parameters ${msgTag}, ${dataSize}. ${msg}`); }
// @ts-ignore
function SMMFD(statusByte, dataByte1, dataByte2) { console.log(`SMMFD called with parameters ${statusByte}, ${dataByte1}, ${dataByte2}`); }
// @ts-ignore
function SSMFD(offset, size, msg) { console.log(`SSMFD called with parameters ${offset}, ${size}, ${msg}`); }

// register the function to globalThis, such that the delegate can see them
// depending on whether you intend to send any of these types of messages,
// you could leave this part out
globalThis.SPVFD = SPVFD;
globalThis.SCVFD = SCVFD;
globalThis.SCMFD = SCMFD;
globalThis.SAMFD = SAMFD;
globalThis.SMMFD = SMMFD;
globalThis.SSMFD = SSMFD;

// The IPlugSendMsg is created within JS when IPlug creates its Webview + JS engine
// However, that means that our IDE will not be able to see this function
//   and thus will give us an error stating that the function is not defined.
// Within VS Code, we can ignore this using the `@ts-ignore` comment.
// I have created an alias function for this so we will not have to write
// `@ts-ignore` every single time we want to send something to IPlug.
export function SendMsgToIPlug(message) { 
    // @ts-ignore
    IPlugSendMsg(message);
}

// Function to send a parameter value back to iPlug
export function SendParameterValueToIPlug(paramIdx: number, value: number) {
    const message = {
        "msg": "SPVFUI",
        "paramIdx": paramIdx,
        "value": (value as number)
    };
    SendMsgToIPlug(message);
}

// ******************************
// ** Elementary specific code **
// ******************************
// Below, we create an instance of the Elementary Renderer.
// This however, requires the sampleRate as an input value.
// To solve this, we create a function to initialize the Renderer,
//   hook this function up to the `globalThis` object and then
//   call that function from the C++ code with the sampleRate
//   as parameter.
// Since the Elementary core relies on the supplied sampleRate,
//   we have to create a new instance of the Renderer every time
//   the sampleRate of the project/host changes.
let core: Renderer | null = null;

function initElementary(sampleRate: number) {
    console.log(`Initializing the Elementary Renderer with sampleRate ${sampleRate}!`);
    // The second parameter in the constructor is a callback function that will be
    //   invoked everytime we call `core.render` and contains the instructions
    //   for the C++ Elementary Runtime to update its internal structure.
    // Hence, we send it forward to our iPlug code.
    core = new Renderer(sampleRate, (instructions) => {
        sendInstructionsToIPlug(JSON.stringify(instructions));
    })
    // render our audio graph
    reconcile(core, get(gain), get(bypass));
}
globalThis.initElementary = initElementary;

function sendInstructionsToIPlug(instructions: string) {
    const message = {
        "msg": "ELEMRENDER", // our custom msg value to signify no base64 encoding/decoding
        "msgTag": 0, // we used msgTag == 0 for instructions
        "ctrlTag": -1, // this is not so important
        "data": instructions
    }
    SendMsgToIPlug(message);
}

// This gets called whenever a parameter changes value
// This one (possibly) comes from the realtime audio thread
//   so we only use it to update the audio graph (Elementary code)
function onParamChange(paramIdx: number, value: number): void {
    if (paramIdx === 0) reconcile(core, value, get(bypass));
    if (paramIdx === 1) reconcile(core, get(gain), Boolean(value));
}
// add it to the global scope so we can invoke it from our C++ code
globalThis.onParamChange = onParamChange;

// **************************
// ** Svelte specific code **
// **************************

// Just like `onParamChange`, this also gets called whenever a parameter changes value
// However, this one always comes from a low-priority thread and should be used to update the UI
// Therefore, only use this function to update the UI part of your code (in this case, Svelte code)
function onParamChangeUI(paramIdx: number, value: number): void {
    if (paramIdx === 0) gain.set(value as number);
    if (paramIdx === 1) bypass.set(Boolean(value));
}
globalThis.onParamChangeUI = onParamChangeUI;

// We let the C++ code know our JS has loaded
SendMsgToIPlug({"msg":"SAMFUI", "msgTag":1,"ctrlTag":-1})

const app = new App({
  target: document.getElementById('app'),
})

export default app
