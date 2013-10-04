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
#include "JSClip.h"

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
	jsSelf.SetWeak<AVSFunction>(this, DestroySelf);
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

void AVSFunction::GetAvisynthName(v8::Local<v8::String> name, const v8::PropertyCallbackInfo<v8::Value>& info) {
	info.GetReturnValue().Set(v8::String::New(UnwrapSelf<AVSFunction>(info.This())->avsName));
}

void AVSFunction::InvokeFunction(const v8::FunctionCallbackInfo<v8::Value>& args) {
	v8::HandleScope scope(args.GetIsolate());
	AVSFunction* wrapped = UnwrapSelf<AVSFunction>(args.This());
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
	env(aEnv), func(aEnv->GetIsolate(), aFunc), signature(NULL), arrayMode(ARRAY_EXPAND_LAST) {
	v8::HandleScope scope(aEnv->GetIsolate());
	// See if there's a signature in the function we were given
	v8::Handle<v8::String> key = v8::String::New("avsSignature");
	if (aFunc->HasOwnProperty(key)) {
		v8::Handle<v8::Value> js_sig = aFunc->Get(key);
		if (!js_sig.IsEmpty()) {
			// Grab its string value
			v8::String::AsciiValue c_sig(js_sig->ToString());
			if (*c_sig) {
				TRACE("Signature from JS: %s\n", *c_sig);
				// FIXME: Check that the signature is even remotely valid.
				// Copy the string value
				signature = _strdup(*c_sig);
			}
		}
	}
	key = v8::String::New("avsExpandArrays");
	// And see if we have a expand array mode.
	if (aFunc->HasOwnProperty(key)) {
		v8::Handle<v8::Value> js_expand = aFunc->Get(key);
		if (!js_expand.IsEmpty()) {
			// And attempt to parse this.
			v8::String::AsciiValue c_expand(js_expand->ToString());
			if (*c_expand) {
				TRACE("Array expand from JS: %s\n", *c_expand);
				if (strcmp(*c_expand, "always") == 0) {
					arrayMode = ARRAY_EXPAND_ALWAYS;
				} else if (strcmp(*c_expand, "never") == 0) {
					arrayMode = ARRAY_EXPAND_NEVER;
				}
				// And just ignore all other values
			}
		}
	}
	// FIXME: At this point, we should check our final signature and expand
	// mode and pick whichever makes the most sense - if there can never be
	// arrays to expand (no * or + in the signature) always change to NEVER
	// mode.
}

JSFunction::~JSFunction() {
	func.Dispose();
	if (signature != NULL) {
		free(signature);
	}
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
	// How we convert the input values depends on the array expand mode
	v8::Handle<v8::Value>* js_argv;
	int argc = args.ArraySize();
	int js_argc = argc;
	// What we do with this depends on the expand mode.
	switch (arrayMode) {
	case ARRAY_EXPAND_ALWAYS:
		// Count up all arrays and add them to the value list.
		for (int i = 0; i < argc; i++) {
			if (args[i].IsArray()) {
				// Increase the argc size by the number of elements in this
				// list - minus 1, for the original array, which we're
				// replacing with all its elements
				js_argc += args[i].ArraySize() - 1;
			}
		}
		break;
	case ARRAY_EXPAND_LAST:
		if (argc > 0 && args[argc-1].IsArray()) {
			js_argc += args[argc-1].ArraySize() - 1;
		}
		break;
	default:
		// Do nothing
		break;
	}
	// Allocate space.
	js_argv = (v8::Handle<v8::Value>*) malloc(js_argc * sizeof(v8::Handle<v8::Value>));
	// And again, how we convert changes based on mode.
	if (arrayMode == ARRAY_EXPAND_ALWAYS) {
		// Expand all arrays given.
		for (int i = 0, j = 0; i < argc; i++) {
			if (args[i].IsArray()) {
				AVSValue arr = args[i];
				int length = arr.ArraySize();
				for (int k = 0; k < length; k++, j++) {
					js_argv[j] = env->ConvertToJS(arr[k]);
				}
			} else {
				js_argv[j] = env->ConvertToJS(args[i]);
				j++;
			}
		}
	} else {
		// Almost identical
		int length = argc;
		if (arrayMode == ARRAY_EXPAND_LAST) {
			length--;
		}
		int i;
		for (i = 0; i < length; i++) {
			js_argv[i] = env->ConvertToJS(args[i]);
		}
		if (arrayMode == ARRAY_EXPAND_LAST) {
			// Convert the final array
			AVSValue arr = args[i];
			int length = arr.ArraySize();
			for (int j = 0; j < length; j++, i++) {
				js_argv[i] = env->ConvertToJS(arr[j]);
			}
		}
	}
	v8::TryCatch try_catch;
	v8::Handle<v8::Value> result = Invoke(js_argc, js_argv);
	// Free now, we're done with them
	free(js_argv);
	if (result.IsEmpty()) {
		TRACE("Error from JavaScript\n");
		ThrowErrorInAviSynth(env->GetAVSScriptEnvironment(), env->GetIsolate(), &try_catch);
		return AVSValue();
	}
	return env->ConvertToAVS(result);
}

}; // namespace jsv