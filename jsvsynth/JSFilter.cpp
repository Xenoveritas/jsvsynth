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

#if 0
namespace jsv {
	
JSFilter::JSFilter() {
}

JSFilter::~JSFilter() {
}

v8::Handle<v8::FunctionTemplate>JSFilter::CreateFilterConstructor(v8::Isolate* isolate) {
	v8::HandleScope scope(isolate);
	v8::Handle<v8::FunctionTemplate> filterConstructor = v8::FunctionTemplate::New(FilterConstructor);
	// And now we need to create our object template for the special properties
	v8::Handle<v8::ObjectTemplate> templ = filterConstructor->InstanceTemplate();
#define JSFILTER_ADD_PROPERTY(JSNAME, FUNCNAME)		templ->SetAccessor(v8::String::New(#JSNAME), JSGet ## FUNCNAME, JSSet ## FUNCNAME)
	JSFILTER_ADD_PROPERTY(width, Width);
	return scope.Close(filterConstructor);
}

static void FilterConstructor(const v8::FunctionCallbackInfo<v8::Value>&);

PVideoFrame __stdcall JSFilter::GetFrame(int n, IScriptEnvironment* env) {
	// Before we do anything, we need to set up the V8 environment
	scriptingEnvironment->EnterIsolate();
	// Next up, handle scope
	v8::HandleScope handleScope(scriptingEnvironment->GetIsolate());
	// Then context scope
	v8::Context::Scope contextScope(scriptingEnvironment->GetContext());
	scriptingEnvironment->ExitIsolate();
	return NULL;
}

};
#endif