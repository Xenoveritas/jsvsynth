/*
 * This file is part of JSVSynth.
 *
 * JSVSynth is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * JSVSynth is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with JSVSynth.  If not, see <http://www.gnu.org/licenses/>.
 */
// Main entry point of the JVSSynth plugin from AviSynth.
#include "stdafx.h"
#include "JSVEnvironment.h"
#include <iostream>

void __cdecl jsv_Shutdown(void* user_data, IScriptEnvironment* env) {
	TRACE("Shutting down V8 and freeing resources...\n");
	jsv::JSVEnvironment* jsvenv = (jsv::JSVEnvironment*)user_data;
	// Destroy the environment.
	v8::Isolate* isolate = jsvenv->GetIsolate();
	isolate->Enter();
	delete jsvenv;
	TRACE("V8 environment shutdown complete.\n");
}

AVSValue __cdecl JS_Script(AVSValue args, void* user_data, IScriptEnvironment* env) {
	TRACE("JavaScript with %p\n", env);
	jsv::JSVEnvironment* jsvEnv = (jsv::JSVEnvironment*) user_data;
	jsvEnv->EnterIsolate();
	// First argument is the script itself.
	// Remaining are... not allowed at present, but will probably become some way to pass stuff into JavaScript, I dunno.
	// Wouldn't it be cool if we could pull in the current script file name and line number from AviSynth?
	// Unfortunately it appears we can't use their own internal ScriptName function to get that
	AVSValue result = jsvEnv->RunScript(args[0].AsString(), "<JavaScript() call>");
	jsvEnv->ExitIsolate();
	return result;
}

AVSValue __cdecl JS_Import(AVSValue args, void* user_data, IScriptEnvironment* env) {
	TRACE("Import JavaScript %s\n", args[0].AsString());
	// FIXME: There's also an "encoding" argument that isn't used at present
	// FIXME: This should do something to make sure we use the current AviSynth
	// "working directory" which we apparently can't do because that
	// information is simply never exposed.
	jsv::JSVEnvironment* jsvEnv = (jsv::JSVEnvironment*) user_data;
	jsvEnv->EnterIsolate();
	AVSValue result = jsvEnv->ImportScript(args[0].AsString());
	jsvEnv->ExitIsolate();
	return result;
}

void InitV8() {
	static bool v8Ready = false;
	if (v8Ready) {
		TRACE("Double-initialized, skipping V8 initialization.\n");
		return;
	}
	TRACE("Initializing V8 settings...\n");
	v8::V8::InitializeICU();
	// Apparently we no longer need to do this
	// Pretend we're a command line program and set some crap for V8
	// TODO (maybe): Somehow allow this to be set from AviSynth
	//int fake_argc = 2;
	//char **fake_argv = new char*[3];
	//fake_argv[0] = NULL;
	// We need to enable the typed arrays feature for our internal system
	// FIXME: This is taken from d8.cc, but why are we strdup-ing the arguments
	// we're sending to V8?
	//fake_argv[1] = _strdup("--harmony-typed-arrays");
	//v8::V8::SetFlagsFromCommandLine(&fake_argc, fake_argv, false);
	//free(fake_argv[1]);
	//delete[] fake_argv;
	v8::V8::SetArrayBufferAllocator(new jsv::JSVAllocator());
	v8Ready = true;
}

extern "C" __declspec(dllexport) const char* __stdcall AvisynthPluginInit2(IScriptEnvironment* env) {
	TRACE("Plugin init with %p\n", env);
	InitV8();
	TRACE("Creating V8 scripting environment...\n");
	// For the duration of this AviSynth environment, we need to have a scripting environment.
	jsv::JSVEnvironment* jsvenv = new jsv::JSVEnvironment(env);
	TRACE("JSVEnvironment instance is %p\n", jsvenv);
	TRACE("V8 environment created.\n");

	// Function that runs a bit of JavaScript that's given as a string argument.
	env->AddFunction("JavaScript", "s", JS_Script, jsvenv);
	// Function that loads an external bit of JavaScript and executes it as above.
	env->AddFunction("ImportJS", "s[ENCODING]s", JS_Import, jsvenv);

	env->AtExit(jsv_Shutdown, jsvenv);
	// A freeform name of the plugin.
	return "'JSVSynth' JavaScript Video Synth";
}