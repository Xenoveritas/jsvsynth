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
#include "jsutil.h"

namespace jsv
{
class JSFunction;

void ThrowErrorInAviSynth(IScriptEnvironment* env, v8::Isolate* isolate, v8::TryCatch* try_catch);

class JSVEnvironment {
public:
	JSVEnvironment(IScriptEnvironment* env);
	~JSVEnvironment();
	AVSValue RunScript(const char* source, const char* filename);
	v8::Handle<v8::Value> RunScript(v8::Handle<v8::String> source, v8::Handle<v8::String> filename, bool raiseErrorsInAviSynth=false);
	v8::Handle<v8::Value> ImportScript(v8::Handle<v8::String> filename, bool raiseErrorsInAviSynth=false);
	AVSValue ImportScript(const char* filename);
	IScriptEnvironment* GetAVSScriptEnvironment() { return avisynthEnv; }
	v8::Isolate* GetIsolate() { return isolate; }
	/**
	 * Enters the isolate and returns it.
	 */
	v8::Isolate* EnterIsolate() { isolate->Enter(); return isolate; }
	void ExitIsolate() { isolate->Exit(); }
	v8::Handle<v8::Context> GetContext() { return v8::Local<v8::Context>::New(isolate, scriptingContext); }
	/**
	 * Convert a value from AviSynth into a V8 value. A handle scope must already exist.
	 */
	v8::Handle<v8::Value> ConvertToJS(AVSValue value);
	/**
	 * Convert the given value in the existing handle scope to an AviSynth value.
	 */
	AVSValue ConvertToAVS(v8::Handle<v8::Value> value);
	void WrapJSFunction(v8::Handle<v8::String> name, v8::Handle<v8::Function> function);
	v8::Handle<v8::Object> WrapAVSFunction(v8::Handle<v8::String> name);
	v8::Handle<v8::Object> WrapAVSFunction(const char* name);
	/**
	 * Wrap a clip for access from JavaScript.
	 */
	v8::Handle<v8::Object> WrapClip(PClip);
	/**
	 * Wrap a video frame for access from JavaScript.
	 */
	v8::Handle<v8::Object> WrapVideoFrame(PVideoFrame, const VideoInfo& vi);
	/**
	 * Gets the current JSVEnvironment if there is one.
	 * Note that is returns whatever's in the "data" slot of the current
	 * v8::Isolate.
	 */
	static JSVEnvironment* GetCurrent() { return static_cast<JSVEnvironment*>(v8::Isolate::GetCurrent()->GetData()); }
private:
	JSFunction* WrapFunction(v8::Handle<v8::Function> func);
	v8::Handle<v8::Context> CreateContext(v8::Isolate* isolate);
	void CreateGlobals(v8::Handle<v8::Object> global);
	JS_NAMED_PROPERTY_DECL(AviSynthAll);
	JS_NAMED_PROPERTY_DECL(AviSynthVariables);
	JS_NAMED_PROPERTY_DECL(AviSynthFunctions);
private:
	v8::Isolate* isolate;
	v8::Persistent<v8::Context> scriptingContext;
	v8::Persistent<v8::ObjectTemplate> clipTemplate;
	v8::Persistent<v8::ObjectTemplate> avsFuncWrapperTemplate;
	v8::Persistent<v8::ObjectTemplate> interleavedVideoFrameTemplate;
	v8::Persistent<v8::ObjectTemplate> planarVideoFrameTemplate;
	/**
	 * A list of allocated memory that we can't delete until AviSynth closes.
	 * At present this is exclusively wrapped JavaScript functions.
	 */
	std::list<void*> stuffToDelete;
	IScriptEnvironment* avisynthEnv;
	friend class AVSFunction;
};

class JSVAllocator : public v8::ArrayBuffer::Allocator {
public:
	JSVAllocator() { }
	~JSVAllocator() { }
	virtual void* Allocate(size_t length);
	virtual void* AllocateUninitialized(size_t length);
	virtual void Free(void* data, size_t length);
};

};
