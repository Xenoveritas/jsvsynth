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
#include "stdafx.h"

#include "Function.h"
#include "JSVEnvironment.h"
#include "Clip.h"

namespace jsv {

AVSFunction::AVSFunction(JSVEnvironment* env, const char* name) : jsvEnv(env) {
	// Copy the string (the given string is from ???)
	size_t len = strlen(name) + 1;
	char* copy = new char[len];
	strcpy_s(copy, len, name);
	avsName = copy;
}

AVSFunction::~AVSFunction() {
	delete avsName;
}

v8::Handle<v8::Object> AVSFunction::NewInstance(v8::Handle<v8::ObjectTemplate> templ) {
	v8::Handle<v8::Object> obj = templ->NewInstance();
	obj->SetInternalField(0, v8::External::New(this));
	jsSelf.Reset(v8::Isolate::GetCurrent(), obj);
	jsSelf.MakeWeak<AVSFunction>(this, DestroySelf);
	return obj;
}

v8::Handle<v8::ObjectTemplate> AVSFunction::CreateTemplate() {
	v8::HandleScope scope(v8::Isolate::GetCurrent());
	v8::Handle<v8::ObjectTemplate> templ = v8::ObjectTemplate::New();
	templ->SetInternalFieldCount(1);
	templ->SetCallAsFunctionHandler(InvokeFunction);
	templ->SetAccessor(v8::String::New("avisynthName"), GetAvisynthName);
	v8::TryCatch try_catch;
	v8::Handle<v8::Script> script = v8::Script::Compile(v8::String::New("(function(){return \"function () { <AviSynth \"+this.avisynthName+\"> }\"})"));
	if (script.IsEmpty()) {
		v8::String::Utf8Value value(try_catch.Exception());
		TRACE("Failed to create toString method! Exception: %s\n", *value);
		// Throw up some sort of error
	} else {
		v8::Handle<v8::Value> res = script->Run();
		if (res.IsEmpty()) {
			TRACE("Failed to run scriptlet!\n");
		} else {
			TRACE("Setting result\n");
			templ->Set("toString", res);
		}
	}
	return scope.Close(templ);
}

AVSFunction* AVSFunction::UnwrapSelf(v8::Handle<v8::Object> obj) {
	v8::Handle<v8::External> ext = v8::Handle<v8::External>::Cast(obj->GetInternalField(0));
	void* ptr = ext->Value();
	return static_cast<AVSFunction*>(ptr);
}

void AVSFunction::DestroySelf(v8::Isolate* isolate, v8::Persistent<v8::Object>* self, AVSFunction* f) {
	v8::HandleScope scope(isolate);
	v8::Local<v8::Object>::New(isolate, (*self))->GetInternalField(0).Clear();
	self->Dispose();
	delete f;
}

void AVSFunction::GetAvisynthName(v8::Local<v8::String> name, const v8::PropertyCallbackInfo<v8::Value>& info) {
	info.GetReturnValue().Set(v8::String::New(UnwrapSelf(info.This())->avsName));
}

void AVSFunction::InvokeFunction(const v8::FunctionCallbackInfo<v8::Value>& args) {
	v8::HandleScope scope(args.GetIsolate());
	AVSFunction* wrapped = UnwrapSelf(args.This());
	int argCount = args.Length();
	AVSValue* avsArgs = NULL;
	int namedArgCount = 0;
	const char** namedArgs = NULL;
	
	// A special case is that if the final argument is an Object, we use the keys
	// to create named arguments. Unless, of course, the final object is a Clip,
	// in which case it's unwrapped.
	if (argCount > 0 && args[argCount - 1]->IsObject() && (!JSClip::IsWrappedClip(args[argCount - 1]->ToObject()))) {
		// Increase the argument count by the number of keys in the object
		TRACE("Final argument is an object, treating as named arguments\n");
		v8::Handle<v8::Object> namedArgsObject = args[argCount - 1]->ToObject();
		v8::Handle<v8::Array> nameArray = namedArgsObject->GetPropertyNames();
		namedArgCount = nameArray->Length();
		// Since we're removing this last object from the "real arguments" subtract 1 from the
		// argument count...
		argCount--;
		// And allocate space for the names and arguments
		avsArgs = (AVSValue*) malloc((argCount + namedArgCount) * sizeof(AVSValue));
		namedArgs = (const char**) malloc((argCount + namedArgCount) * sizeof(const char*));
		// And pull and convert the named arguments.
		for (int i = 0; i < argCount; i++) {
			namedArgs[i] = NULL;
		}
		for (int i = 0; i < namedArgCount; i++) {
			v8::Handle<v8::String> name = nameArray->Get(i)->ToString();
			v8::String::Utf8Value utf8Name(name);
			TRACE("[%d] = %s\n", i, *utf8Name);
			// Grab the value
			v8::Handle<v8::Value> value = namedArgsObject->Get(name);
			avsArgs[i + argCount] = wrapped->jsvEnv->ConvertToAVS(value);
			namedArgs[i + argCount] = _strdup(*utf8Name);
		}
	} else {
		// Allocate for all arguments
		TRACE("Allocating %d arguments...\n", argCount);
		avsArgs = (AVSValue*) malloc((argCount) * sizeof(AVSValue));
	}
	// Convert the values.
	for (int i = 0; i < argCount; i++) {
		TRACE("Converting %d\n", i);
		avsArgs[i] = wrapped->jsvEnv->ConvertToAVS(args[i]);
	}
	// And... GO!
	// Add the named args (if any) to the wrapped agrs
	argCount += namedArgCount;
	TRACE("Attempting to invoke %s with %d arguments (%d were named)\n", wrapped->avsName, argCount, namedArgCount);
	AVSValue result;
	try {
		result = wrapped->jsvEnv->avisynthEnv->Invoke(wrapped->avsName, AVSValue(avsArgs, argCount), namedArgs);
		TRACE("Success!\n");
	} catch (IScriptEnvironment::NotFound) {
		TRACE("Error invoking AviSynth");
		// Oops
		return;
	}
	// And convert back
	if (result.Defined()) {
		args.GetReturnValue().Set(wrapped->jsvEnv->ConvertToJS(result));
	} else {
		args.GetReturnValue().SetUndefined();
	}
	free(avsArgs);
	if (namedArgs != NULL) {
		for (int i = 0; i < argCount; i++) {
			if (namedArgs[i] != NULL) {
				free((void*)namedArgs[i]);
			}
		}
		free(namedArgs);
	}
}

JSFunction::JSFunction(JSVEnvironment* aEnv, v8::Handle<v8::Function> aFunc) :
	env(aEnv), func(aEnv->GetIsolate(), aFunc) {
}
JSFunction::~JSFunction() {
	func.Dispose();
}

v8::Handle<v8::Value> JSFunction::Invoke(int argc, v8::Handle<v8::Value> argv[]) {
	v8::HandleScope scope(env->GetIsolate());
	v8::Context::Scope context_scope(env->GetContext());
	v8::Local<v8::Function> local = v8::Local<v8::Function>::New(env->GetIsolate(), func);
	return scope.Close(local->Call(env->GetContext()->Global(), argc, argv));
}

AVSValue JSFunction::Invoke(AVSValue args) {
	v8::HandleScope scope(env->GetIsolate());
	v8::Context::Scope context_scope(env->GetContext());
	// Step 1: Convert values to JavaScript values
	v8::Handle<v8::Value>* jsargs;
	int argc = args.ArraySize();
	jsargs = (v8::Handle<v8::Value>*) malloc(argc * sizeof(v8::Handle<v8::Value>));
	for (int i = 0; i < argc; i++) {
		jsargs[i] = env->ConvertToJS(args[i]);
	}
	v8::TryCatch try_catch;
	v8::Handle<v8::Value> result = Invoke(argc, jsargs);
	// Free now, we're done with them
	free(jsargs);
	if (result.IsEmpty()) {
		TRACE("Error from JavaScript\n");
		ThrowErrorInAviSynth(env->GetAVSScriptEnvironment(), env->GetIsolate(), &try_catch);
		return AVSValue();
	}
	return env->ConvertToAVS(result);
}

}; // namespace jsv