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

WrappedFunction::WrappedFunction(JSVEnvironment* env, const char* name) : jsvEnv(env) {
	// Copy the string (the given string is from ???)
	size_t len = strlen(name) + 1;
	char* copy = new char[len];
	strcpy_s(copy, len, name);
	avsName = copy;
}

WrappedFunction::~WrappedFunction() {
	delete avsName;
}

v8::Handle<v8::Object> WrappedFunction::NewInstance(v8::Handle<v8::ObjectTemplate> templ) {
	v8::Handle<v8::Object> obj = templ->NewInstance();
	obj->SetInternalField(0, v8::External::New(this));
	return obj;
}

v8::Handle<v8::ObjectTemplate> WrappedFunction::CreateObjectTemplate() {
	v8::Handle<v8::ObjectTemplate> templ = v8::ObjectTemplate::New();
	templ->SetInternalFieldCount(1);
	templ->SetCallAsFunctionHandler(InvokeFunction);
	return templ;
}

WrappedFunction* WrappedFunction::UnwrapSelf(v8::Handle<v8::Object> obj) {
	v8::Handle<v8::External> ext = v8::Handle<v8::External>::Cast(obj->GetInternalField(0));
	void* ptr = ext->Value();
	return static_cast<WrappedFunction*>(ptr);
}

void WrappedFunction::DestroySelf(v8::Isolate* isolate, v8::Persistent<v8::Object>* self, WrappedFunction* f) {
	v8::HandleScope scope(isolate);
	v8::Local<v8::Object>::New(isolate, (*self))->GetInternalField(0).Clear();
	self->Dispose();
	delete f;
}

void WrappedFunction::InvokeFunction(const v8::FunctionCallbackInfo<v8::Value>& args) {
	v8::HandleScope scope(args.GetIsolate());
	WrappedFunction* wrapped = UnwrapSelf(args.This());
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
	TRACE("Freeing %p...\n", avsArgs);
	free(avsArgs);
	TRACE("Frred.\n");
	if (namedArgs != NULL) {
		for (int i = 0; i < argCount; i++) {
			TRACE("Checking %d...\n", i);
			if (namedArgs[i] != NULL) {
				TRACE("Freeing %i (%s)\n", i, namedArgs[i]);
				free((void*)namedArgs[i]);
				TRACE("Freed.\n");
			}
		}
		free(namedArgs);
	}
	TRACE("All done.\n");
}

}; // namespace jsv