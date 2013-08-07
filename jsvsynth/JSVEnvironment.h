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
#include <list>

namespace jsv
{
class JSFunction;

void ThrowErrorInAviSynth(IScriptEnvironment* env, v8::Isolate* isolate, v8::TryCatch* try_catch);

class JSVEnvironment {
public:
	JSVEnvironment(IScriptEnvironment* env);
	~JSVEnvironment();
	AVSValue RunScript(const char* source, const char* filename);
	IScriptEnvironment* GetAVSScriptEnvironment() { return avisynthEnv; }
	v8::Isolate* GetIsolate() { return isolate; }
	v8::Handle<v8::Context> GetContext() { return v8::Local<v8::Context>::New(isolate, scriptingContext); }
	/**
	 * Convert a value from AviSynth into a V8 value. A handle scope must already exist.
	 */
	v8::Handle<v8::Value> ConvertToJS(AVSValue value);
	/**
	 * Convert the given value in the existing handle scope to an AviSynth value.
	 */
	AVSValue ConvertToAVS(v8::Handle<v8::Value> value);
private:
	JSFunction* WrapFunction(v8::Handle<v8::Function> func);
	v8::Handle<v8::Context> CreateContext(v8::Isolate* isolate);
	v8::Handle<v8::Object> CreateAviSynthGlobal();
	static void AviSynthGet(v8::Local<v8::String> name, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void AviSynthSet(v8::Local<v8::String> name, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<v8::Value>& info);
	static JSVEnvironment* UnwrapSelf(v8::Handle<v8::Object>);
private:
	v8::Isolate* isolate;
	v8::Persistent<v8::Context> scriptingContext;
	v8::Persistent<v8::ObjectTemplate> clipTemplate;
	v8::Persistent<v8::ObjectTemplate> avsFuncWrapperTemplate;
	/**
	 * A list of allocated memory that we can't delete until AviSynth closes.
	 * At present this is exclusively wrapped JavaScript functions.
	 */
	std::list<void*> stuffToDelete;
	IScriptEnvironment* avisynthEnv;
	friend class AVSFunction;
};

};
