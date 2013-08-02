#include "stdafx.h"

#include <iostream>
#include <string.h>

#include "JSVEnvironment.h"

using namespace v8;
using namespace std;

namespace jsv
{
// Foward declarations of stuff defined elsewhere.
Handle<Context> CreateContext(Isolate* isolate);
Handle<ObjectTemplate> CreateAviSynthTemplate();

JSVEnvironment::JSVEnvironment(IScriptEnvironment* env) : avisynthEnv(env) {
	isolate = Isolate::GetCurrent();
	HandleScope scope(isolate);
	Handle<Context> context = CreateContext(isolate);
	// We'll be needing this context for the remainder of the program.
	scriptingContext.Reset(isolate, context);
}
JSVEnvironment::~JSVEnvironment() {
	scriptingContext.Dispose();
}

// Extracts a C string from a V8 Utf8Value.
const char* ToCString(const v8::String::Utf8Value& value) {
	return *value ? *value : "<string conversion failed>";
}

// Create a new copy of the string. This lives until ???
char* CopyCStr(const v8::String::Utf8Value& value) {
	const char* cstr = ToCString(value);
	size_t length = strlen(cstr) + 1; // +1 to get the null character
	char* result = new char[length];
	strncpy_s(result, length, cstr, length);
	return result;
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

Handle<Context> CreateContext(Isolate* isolate) {
	// Create a template for the global object.
	Handle<ObjectTemplate> global = ObjectTemplate::New();

	// Create the console object
	Handle<ObjectTemplate> console = ObjectTemplate::New();
	console->Set("log", FunctionTemplate::New(ConsoleLog));
	global->Set("console", console);
	global->Set("AviSynth", CreateAviSynthTemplate());
	return Context::New(isolate, NULL, global);
}

Handle<ObjectTemplate> CreateAviSynthTemplate() {
	Handle<ObjectTemplate> avisynth = ObjectTemplate::New();
	return avisynth;
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
		// In the future we might want to concatenate all that stuff into something
	}
}

AVSValue Convert(Handle<Value> value) {
	if (value->IsString()) {
		// FIXME: I have no idea how this will work (who owns the string? I assume it vanishes when the scope dies, but I have no clue if AviSynth makes a copy)
		String::Utf8Value utf8str(value);
		return AVSValue(CopyCStr(utf8str));
	} else if (value->IsInt32()) {
		return AVSValue(value->Int32Value());
	} else if (value->IsUint32()) {
		return AVSValue((int) value->Uint32Value());
	} else if (value->IsNumber()) {
		return AVSValue(value->NumberValue());
	} else if (value->IsBoolean()) {
		return AVSValue(value->BooleanValue());
	} else {
		String::Utf8Value utf8str(value);
		return AVSValue(CopyCStr(utf8str));
	}
}

AVSValue JSVEnvironment::RunScript(const char* source) {
	cout << "Running script:" << endl << source << endl;
	HandleScope scope(isolate);
	//
	Context::Scope context_scope(isolate, scriptingContext);
	TryCatch try_catch;
	Handle<String> v8source = String::New(source);
	cout << "Compiling script..." << endl;
	Handle<Script> script = Script::Compile(v8source);
	cout << "Script compiled." << endl;
	if (script.IsEmpty()) {
		cout << "Error in script." << endl;
		ThrowErrorInAviSynth(avisynthEnv, isolate, &try_catch);
	} else {
		cout << "Running script..." << endl;
		Handle<Value> result = script->Run();
		cout << "Script run, result was ";
		String::Utf8Value utf8str(result);
		cout << (*utf8str) << endl;
		if (result.IsEmpty()) {
			// In this case, we should always have caught an exception
			ThrowErrorInAviSynth(avisynthEnv, isolate, &try_catch);
		} else {
			cout << "Converting to result..." << endl;
			return Convert(result);
		}
	}
	// If we've managed to fall through here, something has gone wrong, but return something anyway
	return AVSValue(false);
}

};