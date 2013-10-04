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
#pragma once

#include "avisynth.h"
#include <v8.h>

namespace jsv {

class JSVEnvironment;

/**
 * Class that exposes an AviSynth function to JavaScript.
 */
class AVSFunction {
public:
	AVSFunction(JSVEnvironment* env, const char* name);
	~AVSFunction();
	v8::Handle<v8::Object> NewInstance(v8::Handle<v8::ObjectTemplate>);
	static v8::Handle<v8::ObjectTemplate> CreateTemplate();
private:
	static void GetAvisynthName(v8::Local<v8::String>, const v8::PropertyCallbackInfo<v8::Value>&);
	static void InvokeFunction(const v8::FunctionCallbackInfo<v8::Value>& args);
	v8::Persistent<v8::Object> jsSelf;
	const char* avsName;
	JSVEnvironment* jsvEnv;
};

typedef enum ArrayExpandMode {
	ARRAY_EXPAND_ALWAYS,
	ARRAY_EXPAND_NEVER,
	ARRAY_EXPAND_LAST
} ArrayExpandMode;

/**
 * Wrap the other way, exposing a JavaScript function to AviSynth.
 */
class JSFunction {
public:
	JSFunction(JSVEnvironment* env, v8::Handle<v8::Function> func);
	~JSFunction();
	v8::Handle<v8::Value> Invoke(int argc, v8::Handle<v8::Value> args[]);
	AVSValue Invoke(AVSValue args);
	JSVEnvironment* GetEnvironment() { return env; }
	const char* GetSignature() { return signature == NULL ? ".*" : signature; }
	ArrayExpandMode GetArrayExpandMode() { return arrayMode; }
private:
	v8::Persistent<v8::Function> func;
	JSVEnvironment* env;
	char* signature;
	ArrayExpandMode arrayMode;
};

}; // namespace jsv
