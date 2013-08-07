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

void __cdecl jsv_Shutdown(void* user_data, IScriptEnvironment* env) {
	TRACE("Shutting down V8 and freeing resources...\n");
	// Destroy the environment.
	delete ((jsv::JSVEnvironment*)user_data);
	TRACE("V8 environment shutdown.\n");
}

AVSValue __cdecl JS_Script(AVSValue args, void* user_data, IScriptEnvironment* env) {
	TRACE("JavaScript with %p\n", env);
	jsv::JSVEnvironment* jsvEnv = (jsv::JSVEnvironment*) user_data;
	jsvEnv->GetIsolate()->Enter();
	// First argument is the script itself.
	// Remaining are... not allowed at present, but will probably become some way to pass stuff into JavaScript, I dunno.
	// Wouldn't it be cool if we could pull in the current script file name and line number from AviSynth?
	// Unfortunately it appears we can't use their own internal ScriptName function to get that
	AVSValue result = jsvEnv->RunScript(args[0].AsString(), "<JavaScript() call>");
	jsvEnv->GetIsolate()->Exit();
	return result;
}

extern "C" __declspec(dllexport) const char* __stdcall AvisynthPluginInit2(IScriptEnvironment* env) {
	TRACE("Plugin init with %p\n", env);
	TRACE("Creating V8 scripting environment...\n");
	// For the duration of this AviSynth environment, we need to have a scripting environment.
	jsv::JSVEnvironment* jsvenv = new jsv::JSVEnvironment(env);
	TRACE("V8 environment created.\n");

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

	env->AtExit(jsv_Shutdown, jsvenv);
	// A freeform name of the plugin.
	return "'JSVSynth' JavaScript Video Synth";
}