#include "stdafx.h"

#include <iostream>
#include <string.h>

#include "JSVEnvironment.h"
#include "Clip.h"
#include "Function.h"

using namespace v8;
using namespace std;

namespace jsv
{

JSVEnvironment::JSVEnvironment(IScriptEnvironment* env) : avisynthEnv(env) {
	isolate = Isolate::GetCurrent();
	HandleScope scope(isolate);
	Handle<Context> context = CreateContext(isolate);
	// We'll be needing this context for the remainder of the program.
	scriptingContext.Reset(isolate, context);
	// But now that we have the context, init some global things...
	Context::Scope context_scope(context);
	context->Global()->Set(String::New("avisynth"), CreateAviSynthGlobal());
	// And create our clip template for later.
	clipTemplate.Reset(isolate, JSClip::CreateObjectTemplate(context));
	avsFuncWrapperTemplate.Reset(isolate, WrappedFunction::CreateObjectTemplate());
}
JSVEnvironment::~JSVEnvironment() {
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
void ConsoleLog(const FunctionCallbackInfo<Value>& args) {
	// The following is based on shell.cc, so I assume creating a handle
	// scope every pass through the loop is a good idea.
	bool first = true;
	for (int i = 0; i < args.Length(); i++) {
		HandleScope handle_scope(args.GetIsolate());
		if (first) {
			first = false;
		} else {
			std::cout << " ";
		}
		String::Utf8Value str(args[i]);
		std::cout << ToCString(str);
	}
	std::cout << std::endl;
	std::cout.flush();
}

Handle<Context> JSVEnvironment::CreateContext(Isolate* isolate) {
	// Create a template for the global object.
	Handle<ObjectTemplate> global = ObjectTemplate::New();

	// Create the console object
	Handle<ObjectTemplate> console = ObjectTemplate::New();
	console->Set("log", FunctionTemplate::New(ConsoleLog));
	global->Set("console", console);
	return Context::New(isolate, NULL, global);
}

Handle<Object> JSVEnvironment::CreateAviSynthGlobal() {
	HandleScope scope(isolate);
	// Our first step is to create the template. We don't need to hang onto it
	// because there's only ever the single object.
	Handle<ObjectTemplate> avisynth = ObjectTemplate::New();
	// The object does have an internal field, a reference to this class.
	avisynth->SetInternalFieldCount(1);
	avisynth->SetNamedPropertyHandler(AviSynthGet, AviSynthSet);
	Handle<Object> global = avisynth->NewInstance();
	Handle<External> wrapped = External::New(this);
	global->SetInternalField(0, wrapped);
	return scope.Close(global);
}

JSVEnvironment* JSVEnvironment::UnwrapSelf(v8::Handle<v8::Object> obj) {
	Handle<External> ext = Handle<External>::Cast(obj->GetInternalField(0));
	void* ptr = ext->Value();
	return static_cast<JSVEnvironment*>(ptr);
}

void JSVEnvironment::AviSynthGet(v8::Local<v8::String> name, const v8::PropertyCallbackInfo<v8::Value>& info) {
	JSVEnvironment *env = UnwrapSelf(info.Holder());
	String::Utf8Value utf8str(name);
	if (*utf8str) {
		HandleScope scope(env->isolate);
		// First, see if there's a function by this name, in which case we'll
		// use the function wrapper
		if (env->avisynthEnv->FunctionExists(*utf8str)) {
			// Wrap it up
			WrappedFunction* wrapped = new WrappedFunction(env, *utf8str);
			Handle<ObjectTemplate> templ = v8::Local<ObjectTemplate>::New(env->isolate, env->avsFuncWrapperTemplate);
			Handle<Object> result = wrapped->NewInstance(templ);
			info.GetReturnValue().Set(result);
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

void JSVEnvironment::AviSynthSet(v8::Local<v8::String> name, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<v8::Value>& info) {
	// Somewhat simpler (maybe)
	JSVEnvironment *env = UnwrapSelf(info.Holder());
	String::Utf8Value utf8name(name);
	AVSValue avs_value = env->ConvertToAVS(value);
	if (env->avisynthEnv->SetVar(*utf8name, avs_value)) {
		// Value was created.
		// Note: Due to the messed up way AviSynth works, variables created in
		// this fashion likely won't work later in the script.
		// This is because they "don't exist" as far as the parser is concerned
		// or something. Whatever.
	}
}

/**
 * Throw an error inside AviSynth. Strictly speaking, I think this can't return.
 */
void ThrowErrorInAviSynth(IScriptEnvironment* env, Isolate* isolate, TryCatch* try_catch) {
	HandleScope handle_scope(isolate);
	String::Utf8Value exception(try_catch->Exception());
	const char* exception_string = ToCString(exception);
	Handle<Message> message = try_catch->Message();
	if (message.IsEmpty()) {
		env->ThrowError("JavaScript error: %s", exception_string);
	} else {
		// There's a fairly good chance most of the following won't be set, but whatever.
		String::Utf8Value filename(message->GetScriptResourceName());
		const char* filename_string = ToCString(filename);
		int linenum = message->GetLineNumber();
		// Fow now, just dump this.
		env->ThrowError("JavaScript error: %s:%i: %s", filename_string, linenum, exception_string);
		// In the future we might want to concatenate the stack trace onto the response or something
	}
}

Handle<Value> JSVEnvironment::ConvertToJS(AVSValue value) {
	if (!value.Defined()) {
		return v8::Null();
	} else if (value.IsString()) {
		return String::New(value.AsString());
	} else if (value.IsClip()) {
		// Here's the fun one - wrap it up
		HandleScope scope(isolate);
		JSClip* clip = new JSClip(value.AsClip(), Local<ObjectTemplate>::New(isolate, clipTemplate));
		return scope.Close(clip->GetObject(isolate));
	} else if (value.IsInt()) {
		return Int32::New(value.AsInt());
	} else if (value.IsFloat()) {
		return Number::New(value.AsFloat());
	} else if (value.IsBool()) {
		return Boolean::New(value.AsBool());
	} else {
		return String::New("<conversion failure>");
	}
}

AVSValue JSVEnvironment::ConvertToAVS(Handle<Value> value) {
	if (value->IsString()) {
		String::Utf8Value utf8str(value);
		return AVSValue(avisynthEnv->SaveString(ToCString(utf8str)));
	} else if (value->IsInt32()) {
		return AVSValue(value->Int32Value());
	} else if (value->IsUint32()) {
		return AVSValue((int) value->Uint32Value());
	} else if (value->IsNumber()) {
		return AVSValue(value->NumberValue());
	} else if (value->IsBoolean()) {
		return AVSValue(value->BooleanValue());
	} else if (value->IsObject() && JSClip::IsWrappedClip(value->ToObject())) {
		return AVSValue(JSClip::UnwrapClip(value->ToObject()));
	} else {
		String::Utf8Value utf8str(value);
		return AVSValue(avisynthEnv->SaveString(ToCString(utf8str)));
	}
}

AVSValue JSVEnvironment::RunScript(const char* source, const char* filename) {
	TRACE("Running script [\n%s\n]\n", source);
	HandleScope scope(isolate);
	Context::Scope context_scope(isolate, scriptingContext);
	TryCatch try_catch;
	Handle<String> v8source = String::New(source);
	Handle<String> v8filename = String::New(filename);
	Handle<Script> script = Script::Compile(v8source, v8filename);
	if (script.IsEmpty()) {
		TRACE("Error compiling script.\n");
		ThrowErrorInAviSynth(avisynthEnv, isolate, &try_catch);
	} else {
		TRACE("Running script...\n");
		Handle<Value> result = script->Run();
		if (result.IsEmpty()) {
			// In this case, we should always have caught an exception
			TRACE("Script threw error.\n");
			ThrowErrorInAviSynth(avisynthEnv, isolate, &try_catch);
		} else {
			String::Utf8Value utf8str(result);
			TRACE("Script result: %s\n", *utf8str ? *utf8str : "<conversion error>");
			return ConvertToAVS(result);
		}
	}
	// If we've managed to fall through here, something has gone wrong, but return something anyway
	return AVSValue(false);
}

};