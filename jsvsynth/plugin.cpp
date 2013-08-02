// Main entry point of the JVS plugin from AviSynth.
#include "stdafx.h"
#include "JSVEnvironment.h"
#include <iostream>

using namespace v8;
using namespace jsv;
using namespace std;

void __cdecl jvs_Shutdown(void* user_data, IScriptEnvironment* env) {
	// Destroy the environment.
	delete ((JSVEnvironment*)user_data);
}

AVSValue __cdecl JS_Script(AVSValue args, void* user_data, IScriptEnvironment* env) {
	cout << "JS_Script" << endl;
	// First argument is the script itself.
	// Remaining are... not allowed at present, but will probably become some way to pass stuff into JavaScript, I dunno.
	AVSValue result = ((JSVEnvironment*)user_data)->RunScript(args[0].AsString());
	cout << "Completed." << endl;
	return result;
}

extern "C" __declspec(dllexport) const char* __stdcall AvisynthPluginInit2(IScriptEnvironment* env) {
	cout << "Starting up V8..." << endl;
	// For the duration of this environment, we need to have a scripting environment.
	JSVEnvironment* jsvenv = new JSVEnvironment(env);
	cout << "V8 started." << endl;

	// Function that runs a bit of JavaScript that's given as a string argument.
	env->AddFunction("JavaScript", "s", JS_Script, jsvenv);
	// Function that loads an external bit of JavaScript and executes it as above.
	// env->AddFunction("LoadJavaScript", "s", JS_LoadScript, NULL);
	// Canvas API functions
	// env->AddFunction("Canvas", "s", JS_Canvas, NULL);
	// env->AddFunction("LoadCanvas", "s", JS_LoadCanvas, NULL);
    // The AddFunction has the following paramters:
    // AddFunction(Filtername , Arguments, Function to call,0);
    
    // Arguments is a string that defines the types and optional names of the arguments for your filter.
    // c - Video Clip
    // i - Integer number
    // f - Float number
    // s - String
    // b - boolean

	// A freeform name of the plugin.
	return "`JSVSynth' JavaScript Video Synth";
}