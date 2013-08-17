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
#include "JSFilter.h"
#include "JSVideoFrame.h"

namespace jsv {
	
JSFilter::JSFilter(v8::Handle<v8::Object> self, v8::Handle<v8::Object> jsChild) : JSVideoInfo(self) {
	// Make sure we can really get the clip
	child = JSClip::UnwrapClip(jsChild->ToObject());
	v8::Handle<v8::String> key = v8::String::New("child");
	if (!child) {
		// TODO: Handle properly
		JSV_ERROR("Child not available, things will not work!");
	} else {
		vi = child->GetVideoInfo();
	}
	self->Set(key, jsChild, v8::PropertyAttribute::ReadOnly);
	scriptingEnvironment = JSVEnvironment::GetCurrent();
}

JSFilter::~JSFilter() {
}

v8::Handle<v8::FunctionTemplate>JSFilter::CreateFilterConstructor(v8::Isolate* isolate) {
	v8::HandleScope scope(isolate);
	v8::Handle<v8::FunctionTemplate> filterConstructor = v8::FunctionTemplate::New(FilterConstructor);
	// And now we need to create our object template for the special properties
	v8::Handle<v8::ObjectTemplate> templ = filterConstructor->InstanceTemplate();
	templ->SetInternalFieldCount(1);
	PopulateTemplate(templ);
	// Also set our default getFrame on the template
	v8::Context::Scope context_scope(isolate->GetCurrentContext());
	v8::Handle<v8::Script> script = v8::Script::Compile(v8::String::New("(function(){return function(n){return this.child.getFrame(n)}})()"));
	v8::Handle<v8::Value> value = script->Run();
	if (value->IsFunction()) {
		templ->Set("getFrame", value);
	}
	return scope.Close(filterConstructor);
}

void JSFilter::FilterConstructor(const v8::FunctionCallbackInfo<v8::Value>& info) {
	TRACE("Create JSFilter\n");
	v8::HandleScope scope(info.GetIsolate());
	v8::Handle<v8::Object> self = info.This();
	if (info.Length() < 1) {
		TRACE("No arguments.\n");
		v8::ThrowException(v8::Exception::ReferenceError(v8::String::New("Missing arguments")));
		return;
	}
	if (info[0]->IsObject() && JSClip::IsWrappedClip(info[0]->ToObject())) {
		TRACE("First argument is a clip\n");
		new JSFilter(self, info[0]->ToObject());
		TRACE("Created internal wrapper\n");
	} else {
		v8::ThrowException(v8::Exception::TypeError(v8::String::New("First argument was not a clip")));
	}
}

PVideoFrame __stdcall JSFilter::GetFrame(int n, IScriptEnvironment* env) {
	// Before we do anything, we need to set up the V8 environment
	v8::Isolate* isolate = scriptingEnvironment->EnterIsolate();
	// The video frame that is our result will be needed later...
	PVideoFrame result;
	{
		// Now that we've entered the isolate, we need to create a new block
		// so that the various scopes we enter will clear BEFORE we exit the
		// isolate and return control back to AviSynth
		v8::HandleScope handleScope(isolate);
		// Then context scope
		v8::Context::Scope contextScope(scriptingEnvironment->GetContext());
		// And invoke ourselves
		v8::Handle<v8::Object> self = v8::Local<v8::Object>::New(isolate, instance);
		v8::Handle<v8::Value> jsGetFrame = self->Get(v8::String::New("getFrame"));
		// Start a try/catch block
		v8::TryCatch try_catch;
		// Compile the script
		// FIXME: Any sort of error handling
		if (jsGetFrame->IsFunction()) {
			// Invoke with the current frame number
			v8::Handle<v8::Function> f = v8::Handle<v8::Function>::Cast(jsGetFrame);
			v8::Handle<v8::Value> argv[] = { v8::Int32::New(n) };
			v8::Handle<v8::Value> jsResult = f->CallAsFunction(self, 1, argv);
			if (jsResult.IsEmpty()) {
				// FIXME: Handle this
				JSV_ERROR("Script threw an error\n");
			} else {
				if (jsResult->IsObject() && JSVideoFrame::IsWrappedVideoFrame(jsResult->ToObject())) {
					JSVideoFrame* jsFrame = JSVideoFrame::UnwrapVideoFrame(jsResult->ToObject());
					result = jsFrame->GetVideoFrame();
					jsFrame->Release();
				} else {
					v8::String::AsciiValue asciiResult(jsResult);
					JSV_ERROR("getFrame returned %s instead of a frame!\n", asciiResult);
					// FIXME: Handle this
				}
			}
		} else {
			JSV_ERROR("getFrame isn't a function!\n");
			// FIXME: Handle this
		}
	}
	scriptingEnvironment->ExitIsolate();
	return result;
}

bool __stdcall JSFilter::GetParity(int n) {
	if (child) {
		return child->GetParity(n);
	} else {
		return false;
	}
}

void __stdcall JSFilter::GetAudio(void* buf, __int64 start, __int64 count, IScriptEnvironment* env) {
	if (child) {
		child->GetAudio(buf, start, count, env);
	}
}

};
