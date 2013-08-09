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
#include "JSVideoFrame.h"

namespace jsv {

void JSVideoFrame::GetPitch(const v8::FunctionCallbackInfo<v8::Value>& info) {
	JSVideoFrame* self = UnwrapSelf<JSVideoFrame>(info.This());
	if (info.Length() >= 1) {
		// If we have an argument, it needs to be the plane
	} else {
		info.GetReturnValue().Set(v8::Int32::New(self->frame->GetPitch()));
	}
}

void JSVideoFrame::GetRowSize(const v8::FunctionCallbackInfo<v8::Value>& info) {
	JSVideoFrame* self = UnwrapSelf<JSVideoFrame>(info.This());
	if (info.Length() >= 1) {
		// If we have an argument, it needs to be the plane
	} else {
		info.GetReturnValue().Set(v8::Int32::New(self->frame->GetRowSize()));
	}
}

void JSVideoFrame::GetHeight(const v8::FunctionCallbackInfo<v8::Value>& info) {
	JSVideoFrame* self = UnwrapSelf<JSVideoFrame>(info.This());
	if (info.Length() >= 1) {
		// If we have an argument, it needs to be the plane
	} else {
		info.GetReturnValue().Set(v8::Int32::New(self->frame->GetHeight()));
	}
}

void JSVideoFrame::JSGetData(v8::Local<v8::String>, const v8::PropertyCallbackInfo<v8::Value>& info) {
	info.GetReturnValue().Set(v8::Null());
}

v8::Handle<v8::ObjectTemplate> JSVideoFrame::CreateTemplate(v8::Isolate* isolate) {
	v8::HandleScope scope(isolate);
	v8::Handle<v8::ObjectTemplate> templ = v8::ObjectTemplate::New();
	templ->Set(v8::String::New("getPitch"), v8::FunctionTemplate::New(GetPitch));
	templ->Set(v8::String::New("getRowSize"), v8::FunctionTemplate::New(GetRowSize));
	templ->Set(v8::String::New("getHeight"), v8::FunctionTemplate::New(GetHeight));
	templ->SetAccessor(v8::String::New("data"), JSGetData);
	return scope.Close(templ);
}

};