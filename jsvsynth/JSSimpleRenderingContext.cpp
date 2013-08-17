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
#include "JSSimpleRenderingContext.h"

namespace jsv {
	
JSSimpleRenderingContext::JSSimpleRenderingContext(void) { }
JSSimpleRenderingContext::~JSSimpleRenderingContext(void) { }

v8::Handle<v8::ObjectTemplate> JSSimpleRenderingContext::CreateTemplate(v8::Isolate* isolate) {
	v8::HandleScope scope(isolate);
	v8::Handle<v8::ObjectTemplate> templ = v8::ObjectTemplate::New();
	templ->Set("fillRect", v8::FunctionTemplate::New(JSFillRect));
	templ->Set("drawImage", v8::FunctionTemplate::New(JSDrawImage));
	templ->SetInternalFieldCount(1);
	return scope.Close(templ);
}

void JSSimpleRenderingContext::JSFillRect(const v8::FunctionCallbackInfo<v8::Value>& args) {
}

void JSSimpleRenderingContext::JSDrawImage(const v8::FunctionCallbackInfo<v8::Value>& args) {
}

};
