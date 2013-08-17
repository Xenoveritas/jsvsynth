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

#include <iostream>
#include <string.h>

#include "JSVEnvironment.h"
#include "JSClip.h"
#include "Function.h"
#include "JSFilter.h"
#include "JSVideoFrame.h"
#include "util.h"

AVSValue __cdecl InvokeJSFunction(AVSValue args, void* user_data, IScriptEnvironment* env);

namespace jsv
{

JSVEnvironment::JSVEnvironment(IScriptEnvironment* env) : avisynthEnv(env) {
	isolate = v8::Isolate::GetCurrent();
	// Our embedder-specific information is this class
	isolate->SetData(this);
	v8::HandleScope scope(isolate);
	v8::Handle<v8::Context> context = CreateContext(isolate);
	// We'll be needing this context for the remainder of the program.
	scriptingContext.Reset(isolate, context);
	// But now that we have the context, init some global things...
	v8::Context::Scope context_scope(context);
	CreateGlobals(context->Global());
	// And create our templates.
	clipTemplate.Reset(isolate, JSClip::CreateObjectTemplate(context));
	avsFuncWrapperTemplate.Reset(isolate, AVSFunction::CreateTemplate());
	interleavedVideoFrameTemplate.Reset(isolate, JSInterleavedVideoFrame::CreateTemplate(isolate));
	planarVideoFrameTemplate.Reset(isolate, JSPlanarVideoFrame::CreateTemplate(isolate));
}

JSVEnvironment::~JSVEnvironment() {
	TRACE("Removing stuff...\n");
	for (std::list<void*>::iterator i = stuffToDelete.begin(); i != stuffToDelete.end(); i++) {
		TRACE("Killing %p\n", *i);
		delete *i;
	}
	clipTemplate.Dispose();
	scriptingContext.Dispose();
}

// Extracts a C string from a V8 Utf8Value.
const char* ToCString(const v8::String::Utf8Value& value) {
	return *value ? *value : "<string conversion failed>";
}

/**
 * Simply dumps everything given to cout
 */
void ConsoleLog(const v8::FunctionCallbackInfo<v8::Value>& args) {
	// The following is based on shell.cc, so I assume creating a handle
	// scope every pass through the loop is a good idea.
	bool first = true;
	for (int i = 0; i < args.Length(); i++) {
		v8::HandleScope handle_scope(args.GetIsolate());
		if (first) {
			first = false;
		} else {
			std::cerr << " ";
		}
		v8::String::Utf8Value str(args[i]);
		std::cerr << ToCString(str);
	}
	std::cerr << std::endl;
	std::cerr.flush();
}

#ifdef _DEBUG

#define ALERT_MAX_SIZE		4095

void DebugShowAlert(const v8::FunctionCallbackInfo<v8::Value>& args) {
	// Create a buffer for the string message. Note that's characters, not bytes.
	WCHAR message[ALERT_MAX_SIZE + 1];
	int offset = 0;
	int length = args.Length();
	for (int i = 0; i < length && offset < ALERT_MAX_SIZE; i++) {
		v8::Handle<v8::String> str = args[i]->ToString();
		int stringLen = str->Length();
		if (offset + stringLen > ALERT_MAX_SIZE) {
			stringLen = ALERT_MAX_SIZE - offset;
		}
		str->Write((uint16_t*)(message + offset), 0, stringLen);
		offset += stringLen;
	}
	message[offset] = 0;
	// TODO: Actually pay attention to the arguments
	// This exists primarily to "pause" a running AviSynth script long enough
	// to attach the debugger to it.
	MessageBox(NULL, message, L"Debug", MB_ICONEXCLAMATION | MB_OK);
}

#endif

v8::Handle<v8::Context> JSVEnvironment::CreateContext(v8::Isolate* isolate) {
	// Create a template for the global object.
	v8::Handle<v8::ObjectTemplate> global = v8::ObjectTemplate::New();

	// Create the console object
	v8::Handle<v8::ObjectTemplate> console = v8::ObjectTemplate::New();
	console->Set("log", v8::FunctionTemplate::New(ConsoleLog));
#ifdef _DEBUG
	console->Set("alert", v8::FunctionTemplate::New(DebugShowAlert));
#endif
	global->Set("console", console);
	return v8::Context::New(isolate, NULL, global);
}

void JSVEnvironment::CreateGlobals(v8::Handle<v8::Object> global) {
	v8::HandleScope scope(isolate);
	// Wrap ourselves in an external. We'll be needed later.
	v8::Handle<v8::External> wrapped = v8::External::New(this);
	// Our first step is to create the template. We don't need to hang onto it
	// because there's only ever the single object.
	// FIXME: Because there's only one, I suspect we don't even need the template
	v8::Handle<v8::ObjectTemplate> avisynthTemplate = v8::ObjectTemplate::New();
	// Classes inside our "namespace"
	avisynthTemplate->Set(v8::String::New("Filter"), JSFilter::CreateFilterConstructor(isolate));
	// Set constants on it
	avisynthTemplate->Set(v8::String::New("PLANAR_Y"), v8::Int32::New(PLANAR_Y), v8::ReadOnly);
	avisynthTemplate->Set(v8::String::New("PLANAR_U"), v8::Int32::New(PLANAR_U), v8::ReadOnly);
	avisynthTemplate->Set(v8::String::New("PLANAR_V"), v8::Int32::New(PLANAR_V), v8::ReadOnly);
	/*
	Not exported: Not needed for now (access is always unaligned, not sure it makes a difference)
	avisynthTemplate->Set(v8::String::New("PLANAR_ALIGNED"), v8::Int32::New(PLANAR_ALIGNED), v8::ReadOnly);
	avisynthTemplate->Set(v8::String::New("PLANAR_Y_ALIGNED"), v8::Int32::New(PLANAR_Y_ALIGNED), v8::ReadOnly);
	avisynthTemplate->Set(v8::String::New("PLANAR_U_ALIGNED"), v8::Int32::New(PLANAR_U_ALIGNED), v8::ReadOnly);
	avisynthTemplate->Set(v8::String::New("PLANAR_V_ALIGNED"), v8::Int32::New(PLANAR_V_ALIGNED), v8::ReadOnly);
	*/
	// And set the two collections
	v8::Handle<v8::ObjectTemplate> functionsTemplate = v8::ObjectTemplate::New();
	functionsTemplate->SetNamedPropertyHandler(JSGetAviSynthFunctions, JSSetAviSynthFunctions);
	functionsTemplate->SetInternalFieldCount(1);
	avisynthTemplate->Set(v8::String::New("functions"), functionsTemplate);
	v8::Handle<v8::ObjectTemplate> variablesTemplate = v8::ObjectTemplate::New();
	variablesTemplate->SetNamedPropertyHandler(JSGetAviSynthVariables, JSSetAviSynthVariables);
	variablesTemplate->SetInternalFieldCount(1);
	avisynthTemplate->Set(v8::String::New("variables"), variablesTemplate);
	avisynthTemplate->SetInternalFieldCount(1);
	v8::Handle<v8::Object> avisynth = avisynthTemplate->NewInstance();
	// Populate the internal collections
	v8::Handle<v8::Object>::Cast(avisynth->Get(v8::String::New("functions")))->SetInternalField(0, wrapped);
	v8::Handle<v8::Object>::Cast(avisynth->Get(v8::String::New("variables")))->SetInternalField(0, wrapped);
	global->Set(v8::String::New("AviSynth"), avisynth);
	v8::Handle<v8::Object> jsvsynth = v8::Object::New();
	jsvsynth->Set(v8::String::New("version"), v8::String::New(JSVSYNTH_VERSION_STRING), v8::PropertyAttribute::ReadOnly);
	global->Set(v8::String::New("JSVSynth"), jsvsynth);
	v8::Handle<v8::ObjectTemplate> avsTemplate = v8::ObjectTemplate::New();
	// The object does have an internal field, a reference to this class.
	avsTemplate->SetInternalFieldCount(1);
	avsTemplate->SetNamedPropertyHandler(JSGetAviSynthAll, JSSetAviSynthAll);
	v8::Handle<v8::Object> avs = avsTemplate->NewInstance();
	avs->SetInternalField(0, wrapped);
	global->Set(v8::String::New("avs"), avs);
}

void JSVEnvironment::WrapJSFunction(v8::Handle<v8::String> name, v8::Handle<v8::Function> function) {
	v8::String::Utf8Value utf8name(name);
	if (!*utf8name) {
		// can't convert, therefore we can't add it
		TRACE("String conversion failed\n");
		return;
	}
	TRACE("Wrapping function %s from JavaScript\n", *utf8name);
	JSFunction* res = new JSFunction(this, function);
	stuffToDelete.push_back(res);
	TRACE("Wrapper has signature \"%s\"\n", res->GetSignature());
	avisynthEnv->AddFunction(*utf8name, res->GetSignature(), InvokeJSFunction, res);
}

v8::Handle<v8::Object> JSVEnvironment::WrapAVSFunction(v8::Handle<v8::String> name) {
	v8::HandleScope scope(isolate);
	v8::String::Utf8Value utf8Name(name);
	if (!*utf8Name) {
		TRACE("Unable to convert string!\n");
	}
	return scope.Close(WrapAVSFunction(*utf8Name));
}
v8::Handle<v8::Object> JSVEnvironment::WrapAVSFunction(const char* name) {
	v8::HandleScope scope(isolate);
	AVSFunction* wrapped = new AVSFunction(this, name);
	v8::Handle<v8::ObjectTemplate> templ = v8::Local<v8::ObjectTemplate>::New(isolate, avsFuncWrapperTemplate);
	v8::Handle<v8::Object> result = wrapped->NewInstance(templ);
	return scope.Close(result);
}

v8::Handle<v8::Object> JSVEnvironment::WrapClip(PClip clip) {
	TRACE("Wrap clip\n");
	v8::HandleScope scope(isolate);
	JSClip* jsclip = new JSClip(clip, v8::Local<v8::ObjectTemplate>::New(isolate, clipTemplate));
	TRACE("Converted.\n");
	return scope.Close(jsclip->GetInstance(isolate));
}

v8::Handle<v8::Object> JSVEnvironment::WrapVideoFrame(PVideoFrame frame, const VideoInfo& vi) {
	// Sadly we need the video info to know which type of video access to provide.
	JSVideoFrame* wrappedFrame;
	if (vi.IsPlanar()) {
		TRACE("Wrapping planar video frame...\n");
		wrappedFrame = new JSPlanarVideoFrame(frame, vi, isolate, v8::Local<v8::ObjectTemplate>::New(isolate, planarVideoFrameTemplate));
	} else {
		wrappedFrame = new JSInterleavedVideoFrame(frame, vi, isolate, v8::Local<v8::ObjectTemplate>::New(isolate, interleavedVideoFrameTemplate));
	}
	TRACE("Returning wrapped frame.\n");
	return wrappedFrame->GetInstance(isolate);
}

void JSVEnvironment::JSGetAviSynthAll(v8::Local<v8::String> name, const v8::PropertyCallbackInfo<v8::Value>& info) {
	JSVEnvironment *env = UnwrapSelf<JSVEnvironment>(info.Holder());
	v8::String::Utf8Value utf8str(name);
	if (*utf8str) {
		v8::HandleScope scope(env->isolate);
		// First, see if there's a function by this name, in which case we'll
		// use the function wrapper
		if (env->avisynthEnv->FunctionExists(*utf8str)) {
			// Wrap it up
			info.GetReturnValue().Set(env->WrapAVSFunction(*utf8str));
			return;
		}
		try {
			AVSValue value = env->avisynthEnv->GetVar(*utf8str);
			// If we're here, we have a value
			info.GetReturnValue().Set(env->ConvertToJS(value));
		} catch (IScriptEnvironment::NotFound) {
			// Does not exist, so do nothing (which signals the property not existing to V8)
		}
	}
}

void JSVEnvironment::JSSetAviSynthAll(v8::Local<v8::String> name, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<v8::Value>& info) {
	// Somewhat simpler (maybe)
	JSVEnvironment *env = UnwrapSelf<JSVEnvironment>(info.Holder());
	v8::String::Utf8Value utf8name(name);
	if (value->IsFunction()) {
		env->WrapJSFunction(name, v8::Handle<v8::Function>::Cast(value));
		return;
	}
	AVSValue avs_value = env->ConvertToAVS(value);
	if (env->avisynthEnv->SetVar(*utf8name, avs_value)) {
		// Value was created.
		// Note: Due to the messed up way AviSynth works, variables created in
		// this fashion likely won't work later in the script.
		// This is because they "don't exist" as far as the parser is concerned
		// or something. Whatever.
	}
}

void JSVEnvironment::JSGetAviSynthVariables(v8::Local<v8::String> name, const v8::PropertyCallbackInfo<v8::Value>& info) {
	JSVEnvironment *env = UnwrapSelf<JSVEnvironment>(info.Holder());
	v8::String::Utf8Value utf8str(name);
	if (*utf8str) {
		v8::HandleScope scope(env->isolate);
		try {
			AVSValue value = env->avisynthEnv->GetVar(*utf8str);
			// If we're here, we have a value
			info.GetReturnValue().Set(env->ConvertToJS(value));
		} catch (IScriptEnvironment::NotFound) {
			// Does not exist, so do nothing (which signals the property not existing to V8)
		}
	}
}

void JSVEnvironment::JSSetAviSynthVariables(v8::Local<v8::String> name, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<v8::Value>& info) {
	// Somewhat simpler (maybe)
	JSVEnvironment *env = UnwrapSelf<JSVEnvironment>(info.Holder());
	v8::String::Utf8Value utf8name(name);
	if (value->IsFunction()) {
		// Nope;
		v8::ThrowException(v8::Exception::TypeError(v8::String::New("Cannot add functions to the variables collection")));
		return;
	}
	AVSValue avs_value = env->ConvertToAVS(value);
	if (env->avisynthEnv->SetVar(*utf8name, avs_value)) {
		// Value was created.
		// Note: Due to the messed up way AviSynth works, variables created in
		// this fashion likely won't work later in the script.
		// This is because they "don't exist" as far as the parser is concerned
		// or something. Whatever.
	}
}

void JSVEnvironment::JSGetAviSynthFunctions(v8::Local<v8::String> name, const v8::PropertyCallbackInfo<v8::Value>& info) {
	JSVEnvironment *env = UnwrapSelf<JSVEnvironment>(info.Holder());
	v8::String::Utf8Value utf8str(name);
	if (*utf8str) {
		v8::HandleScope scope(env->isolate);
		// All we need to do is see if there's a function.
		if (env->avisynthEnv->FunctionExists(*utf8str)) {
			info.GetReturnValue().Set(env->WrapAVSFunction(*utf8str));
		}
		// Does not exist, so do nothing (which signals the property not existing to V8)
	}
}

void JSVEnvironment::JSSetAviSynthFunctions(v8::Local<v8::String> name, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<v8::Value>& info) {
	// Somewhat simpler (maybe)
	JSVEnvironment *env = UnwrapSelf<JSVEnvironment>(info.Holder());
	v8::String::Utf8Value utf8name(name);
	if (value->IsFunction()) {
		env->WrapJSFunction(name, v8::Handle<v8::Function>::Cast(value));
		return;
	}
	// Otherwise, raise error
	v8::ThrowException(v8::Exception::TypeError(v8::String::New("Cannot set non-function in functions collection")));
}

/**
 * Throw an error inside AviSynth. Strictly speaking, I think this can't return.
 */
void ThrowErrorInAviSynth(IScriptEnvironment* env, v8::Isolate* isolate, v8::TryCatch* try_catch) {
	v8::HandleScope handle_scope(isolate);
	v8::String::Utf8Value exception(try_catch->Exception());
	const char* exception_string = ToCString(exception);
	v8::Handle<v8::Message> message = try_catch->Message();
	if (message.IsEmpty()) {
		env->ThrowError("JavaScript error: %s", exception_string);
	} else {
		// There's a fairly good chance most of the following won't be set, but whatever.
		v8::String::Utf8Value filename(message->GetScriptResourceName());
		const char* filename_string = ToCString(filename);
		int linenum = message->GetLineNumber();
		// Fow now, just dump this.
		env->ThrowError("JavaScript error: %s:%i: %s", filename_string, linenum, exception_string);
		// In the future we might want to concatenate the stack trace onto the response or something
	}
}

#ifdef TRACE_CONVERSIONS
# define TRACE_CONV		TRACE
#else
# define TRACE_CONV(fmt, ...)	do {} while(0)
#endif

v8::Handle<v8::Value> JSVEnvironment::ConvertToJS(AVSValue value) {
	// FIXME: TRACE_CONVERSIONS should also dump stuff here
	if (!value.Defined()) {
		// TODO: null or undefined?
		return v8::Null();
	} else if (value.IsString()) {
		return v8::String::New(value.AsString());
	} else if (value.IsClip()) {
		return WrapClip(value.AsClip());
	} else if (value.IsInt()) {
		// Int must be tested prior to float as IsFloat() also returns true for ints
		return v8::Int32::New(value.AsInt());
	} else if (value.IsFloat()) {
		return v8::Number::New(value.AsFloat());
	} else if (value.IsBool()) {
		return v8::Boolean::New(value.AsBool());
	} else if (value.IsArray()) {
		// This is mostly used by JSFunction to convert incoming arrays when
		// the array expand mode is "never" or when they aren't the last value
		// in the arguments list. In fact, it should really never happen
		// outside converting argument lists, as that's the only time
		// AviSynth will generate arrays.
		v8::HandleScope scope(isolate);
		int count = value.ArraySize();
		v8::Handle<v8::Array> result = v8::Array::New(count);
		for (int i = 0; i < count; i++) {
			result->Set(i, ConvertToJS(value[i]));
		}
		return scope.Close(result);
	} else {
		TRACE("Unknown type of AVS value %s\n", value.AsString("bad"));
		return v8::String::New("<conversion failure>");
	}
}

AVSValue JSVEnvironment::ConvertToAVS(v8::Handle<v8::Value> value) {
#ifdef TRACE_CONVERSIONS
# ifdef _DEBUG
	{
		v8::HandleScope scope(isolate);
		v8::String::Utf8Value utf8(value);
		TRACE("Converting [%s] to AviSynth\n", *utf8);
	}
# endif
#endif
	if (value.IsEmpty()) {
		TRACE_CONV("Value is empty, going with undefined!\n");
		return AVSValue();
	}
	if (value->IsString()) {
		TRACE_CONV("-> to string\n");
		v8::String::Utf8Value utf8str(value);
		return AVSValue(avisynthEnv->SaveString(ToCString(utf8str)));
	} else if (value->IsInt32()) {
		TRACE_CONV("-> to int32\n");
		return AVSValue(value->Int32Value());
	} else if (value->IsUint32()) {
		TRACE_CONV("-> to uint32\n");
		return AVSValue((int) value->Uint32Value());
	} else if (value->IsNumber()) {
		TRACE_CONV("-> to number\n");
		return AVSValue(value->NumberValue());
	} else if (value->IsBoolean()) {
		TRACE_CONV("-> to boolean\n");
		return AVSValue(value->BooleanValue());
	} else if (value->IsObject() && JSClip::IsWrappedClip(value->ToObject())) {
		TRACE_CONV("-> unwrapping clip\n");
		return AVSValue(JSClip::UnwrapClip(value->ToObject()));
	} else {
		TRACE_CONV("Conversion failed!\n");
		v8::String::Utf8Value utf8str(value);
		return AVSValue(avisynthEnv->SaveString(ToCString(utf8str)));
	}
}

AVSValue JSVEnvironment::RunScript(const char* source, const char* filename) {
	TRACE("Running script [\n%s\n]\n", source);
	// We need to create scopes to do the conversions to/from
	v8::HandleScope scope(isolate);
	v8::Context::Scope context_scope(isolate, scriptingContext);
	// And then throw it to the generic handler
	v8::Handle<v8::Value> value = RunScript(v8::String::New(source), v8::String::New(filename), true);
	TRACE("Script completed.\n");
	AVSValue avsvalue = ConvertToAVS(value);
	TRACE("Conversion complete.\n");
	return avsvalue;
}

v8::Handle<v8::Value> JSVEnvironment::RunScript(v8::Handle<v8::String> source, v8::Handle<v8::String> filename, bool raiseErrorsInAviSynth) {
	// Create the scopes
	v8::HandleScope scope(isolate);
	v8::Context::Scope context_scope(isolate, scriptingContext);
	// Start a try/catch block
	v8::TryCatch try_catch;
	// Compile the script
	v8::Handle<v8::Script> script = v8::Script::Compile(source, filename);
	if (script.IsEmpty()) {
		TRACE("Error compiling script.\n");
		if (raiseErrorsInAviSynth) {
			ThrowErrorInAviSynth(avisynthEnv, isolate, &try_catch);
		} else {
			try_catch.ReThrow();
		}
	} else {
		TRACE("Running script...\n");
		v8::Handle<v8::Value> result = script->Run();
		if (result.IsEmpty()) {
			// In this case, we should always have caught an exception
			TRACE("Script threw error.\n");
			if (raiseErrorsInAviSynth) {
				ThrowErrorInAviSynth(avisynthEnv, isolate, &try_catch);
			} else {
				try_catch.ReThrow();
			}
		} else {
			v8::String::Utf8Value utf8str(result);
			TRACE("Script result: %s\n", *utf8str ? *utf8str : "<conversion error>");
			return scope.Close(result);
		}
	}
	// If we've managed to fall through here, something has gone wrong, but return something anyway
	return v8::Undefined();
}

v8::Handle<v8::Value> JSVEnvironment::ImportScript(v8::Handle<v8::String> filename, bool raiseErrorsInAviSynth) {
	// Load the script from the file system.
	// Need a handle scope for the strings we're about to allocate
	v8::HandleScope scope(isolate);
	v8::String::Utf8Value utf8filename(filename);
	v8::Handle<v8::String> source = ReadFile(*utf8filename);
	if (source.IsEmpty()) {
		// Indicates that the file load failed. TODO: Handle this appropriately.
		if (raiseErrorsInAviSynth) {
			avisynthEnv->ThrowError("Unable to load file \"%s\"", *utf8filename);
		}
	} else {
		// FIXME: Strictly speaking, we should be running the script in a new context.
		// We don't.
		return scope.Close(RunScript(source, filename, raiseErrorsInAviSynth));
	}
	return v8::Undefined();
}

AVSValue JSVEnvironment::ImportScript(const char* filename) {
	v8::HandleScope scope(isolate);
	// We also need a context scope to do the conversion
	v8::Context::Scope context_scope(isolate, scriptingContext);
	return ConvertToAVS(ImportScript(v8::String::New(filename), true));
}

void* JSVAllocator::Allocate(size_t length) {
	void* result = malloc(length);
	memset(result, 0, length);
	return result;
}

void* JSVAllocator::AllocateUninitialized(size_t length) {
	return malloc(length);
}

void JSVAllocator::Free(void* data, size_t length) {
	free(data);
}

};

/**
 * Callback for AviSynth in order to invoke our wrapped JavaScript function.
 */
AVSValue __cdecl InvokeJSFunction(AVSValue args, void* user_data, IScriptEnvironment* env) {
	TRACE("Invoke JavaScript from AviSynth\n");
	// Grab the jsfunc...
	jsv::JSFunction* jsfunc = (jsv::JSFunction*) user_data;
	jsfunc->GetEnvironment()->EnterIsolate();
	// User data (should) be a JSFunction, allowing us to just do this:
	AVSValue result = static_cast<jsv::JSFunction*>(user_data)->Invoke(args);
	TRACE("Returning back to AviSynth\n");
	jsfunc->GetEnvironment()->ExitIsolate();
	return result;
}