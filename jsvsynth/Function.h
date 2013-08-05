#pragma once

#include "avisynth.h"
#include <v8.h>

namespace jsv {

class JSVEnvironment;

class WrappedFunction {
public:
	WrappedFunction(JSVEnvironment* env, const char* name);
	~WrappedFunction();
	v8::Handle<v8::Object> NewInstance(v8::Handle<v8::ObjectTemplate>);
	static v8::Handle<v8::ObjectTemplate> CreateObjectTemplate();
private:
	static WrappedFunction* UnwrapSelf(v8::Handle<v8::Object>);
	static void DestroySelf(v8::Isolate* isolate, v8::Persistent<v8::Object>* self, WrappedFunction* c);
	static void InvokeFunction(const v8::FunctionCallbackInfo<v8::Value>& args);
	v8::Persistent<v8::Object> jsSelf;
	const char* avsName;
	JSVEnvironment* jsvEnv;
};

}; // namespace jsv
