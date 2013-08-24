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
#include "JSVEnvironment.h"
#include "JSSimpleRenderingContext.h"

#include <iostream>

#define JSVIDEOFRAME_HIDDEN_PROP	"jsv::VideoFrame"

namespace jsv {

JSVideoFrame::JSVideoFrame(PVideoFrame aFrame, const VideoInfo& aVI, v8::Isolate* isolate, v8::Handle<v8::ObjectTemplate> templ)
	: frame(aFrame), vi(aVI), madeWeak(false), released(false) {
	TRACE("JSVideoFrame::JSVideoFrame\n");
	v8::HandleScope scope(isolate);
	// Create a new instance
	TRACE("Creating new instance...\n");
	v8::Handle<v8::Object> inst = v8::Local<v8::ObjectTemplate>::New(isolate, templ)->NewInstance();
	inst->SetHiddenValue(v8::String::New(JSVIDEOFRAME_HIDDEN_PROP), v8::True());
	instance.Reset(isolate, inst);
	madeWeak = false;
}

JSVideoFrame::~JSVideoFrame() {
	instance.Dispose();
}

v8::Handle<v8::Object> JSVideoFrame::GetInstance(v8::Isolate* isolate) {
	v8::HandleScope scope(isolate);
	v8::Handle<v8::Object> inst = v8::Local<v8::Object>::New(isolate, instance);
	if (!madeWeak) {
		// Now that an instance has been retreived, mark our persistent weak so
		// that we will be destroyed as appropriate when the frame goes out of
		// scope.
		instance.MakeWeak(isolate, this, DestroySelf<JSVideoFrame>);
		madeWeak = true;
	}
	return scope.Close(inst);
}

bool JSVideoFrame::IsWrappedVideoFrame(v8::Handle<v8::Object> obj) {
	v8::HandleScope scope(v8::Isolate::GetCurrent());
	v8::Handle<v8::Value> value = obj->GetHiddenValue(v8::String::New(JSVIDEOFRAME_HIDDEN_PROP));
	return (!value.IsEmpty()) && value->IsBoolean() && value->IsTrue();
}

JSVideoFrame* JSVideoFrame::UnwrapVideoFrame(v8::Handle<v8::Object> obj) {
	v8::HandleScope scope(v8::Isolate::GetCurrent());
	v8::Handle<v8::Value> value = obj->GetHiddenValue(v8::String::New(JSVIDEOFRAME_HIDDEN_PROP));
	if ((!value.IsEmpty()) && value->IsBoolean() && value->IsTrue()) {
		return UnwrapSelf<JSVideoFrame>(obj);
	} else {
		return NULL;
	}	
}

void JSVideoFrame::Release() {
	if (!released) {
		frame = NULL;
		released = true;
	}
}
void JSVideoFrame::CreateTemplateDefaultFields(v8::Handle<v8::ObjectTemplate> templ) {
	templ->Set(JSPROP_NAME("getContext"), v8::FunctionTemplate::New(JSGetContext), JSPROP_READONLY);
	templ->Set(JSPROP_NAME("getSimpleContext"), v8::FunctionTemplate::New(JSGetSimpleContext), JSPROP_READONLY);
}

void JSVideoFrame::JSGetContext(const v8::FunctionCallbackInfo<v8::Value>& args) {
	// Before doing anything, parse the type.
	v8::HandleScope scope(args.GetIsolate());
	if (args.Length() < 1) {
		v8::ThrowException(v8::Exception::Error(v8::String::New("Missing type for getContext")));
		return;
	}
	v8::String::AsciiValue type(args[0]->ToString());
	TRACE("Get context [%s]\n", *type);
	if (strcmp("simple", *type) == 0) {
		// Simple context
		JSGetSimpleContext(args);
	} else {
		// Unknown, return null
		args.GetReturnValue().SetNull();
	}
}

void JSVideoFrame::JSGetSimpleContext(const v8::FunctionCallbackInfo<v8::Value>& args) {
	v8::HandleScope scope(args.GetIsolate());
	// Ignore the arguments (as we're currently called directly from GetContext)
	JSVideoFrame* self = UnwrapSelf<JSVideoFrame>(args.This());
	JSSimpleRenderingContext* renderingContext = self->GetSimpleContext();
	if (renderingContext == NULL) {
		args.GetReturnValue().SetNull();
	} else {
		v8::Handle<v8::Object> inst = renderingContext->GetInstance(args.GetIsolate());
		args.GetReturnValue().Set(inst);
	}
}

v8::Handle<v8::ArrayBuffer> JSVideoFrame::WrapData(v8::Isolate* isolate, BYTE* data, int length) {
	TRACE("Wrapping frame data at %p (%d bytes)\n", data, length);
	v8::HandleScope scope(isolate);
	v8::Context::Scope contextScope(isolate->GetCurrentContext());
	v8::Handle<v8::ArrayBuffer> result = v8::ArrayBuffer::New(data, length);
	// To ensure somewhat cleaner garbage collection, add ourselves as a property.
	// This makes sure anyone hanging onto a data buffer (don't do that!) but not the
	// frame won't have the buffer deallocated/reused out from under them.
	// It also makes for a great potential memory leak!
	result->Set(JSPROP_NAME("frame"), GetInstance(isolate), JSPROP_READONLY);
	return scope.Close(result);
}

JSInterleavedVideoFrame::JSInterleavedVideoFrame(PVideoFrame aFrame, const VideoInfo& aVI, v8::Isolate* isolate, v8::Handle<v8::ObjectTemplate> templ)
	: JSVideoFrame(aFrame, aVI, isolate, templ), simpleContext(NULL) {
	PopulateInstance(v8::Local<v8::Object>::New(isolate, instance));
}

JSInterleavedVideoFrame::~JSInterleavedVideoFrame() {
	TRACE_MEM();
	// Release ourselves if we haven't yet - this also deletes our context, if
	// there was one.
	Release();
	dataInstance.Dispose();
}

void JSInterleavedVideoFrame::Release() {
	TRACE_MEM();
	JSVideoFrame::Release();
	v8::Isolate* isolate = v8::Isolate::GetCurrent();
	v8::HandleScope scope(isolate);
	if (!dataInstance.IsEmpty()) {
		v8::Local<v8::ArrayBuffer>::New(isolate, dataInstance)->Neuter();
	}
	if (simpleContext != NULL) {
		delete simpleContext;
		simpleContext = NULL;
	}
}

void JSInterleavedVideoFrame::PopulateInstance(v8::Handle<v8::Object> inst) {
	inst->Set(JSPROP_NAME("pitch"), v8::Int32::New(frame->GetPitch()), JSPROP_READONLY);
	inst->Set(JSPROP_NAME("rowSize"), v8::Int32::New(frame->GetRowSize()), JSPROP_READONLY);
	inst->Set(JSPROP_NAME("height"), v8::Int32::New(frame->GetHeight()), JSPROP_READONLY);
	inst->Set(JSPROP_NAME("bitsPerPixel"), v8::Int32::New(vi.BitsPerPixel()), JSPROP_READONLY);
	inst->Set(JSPROP_NAME("bytesPerPixel"), v8::Int32::New(vi.BytesFromPixels(1)), JSPROP_READONLY);
	inst->SetInternalField(0, v8::External::New(this));
}

JSSimpleRenderingContext* JSInterleavedVideoFrame::GetSimpleContext() {
	if (simpleContext != NULL) {
		return simpleContext;
	}
	if (vi.IsRGB32()) {
		v8::Isolate* isolate = v8::Isolate::GetCurrent();
		MakeWriteable(isolate);
		TRACE("After MakeWriteable, frame is writeable? %s\n", frame->IsWritable() ? "yes" : "no");
		simpleContext = new RGB32SimpleRenderingContext(frame.operator->(), JSVEnvironment::GetCurrent(isolate)->NewSimpleContext());
		return simpleContext;
	} else {
		// Not implemented:
		return NULL;
	}
}


void JSInterleavedVideoFrame::JSRelease(const v8::FunctionCallbackInfo<v8::Value>& info) {
	JSInterleavedVideoFrame* self = UnwrapSelf<JSInterleavedVideoFrame>(info.This());
	self->Release();
}

void JSInterleavedVideoFrame::JSGetData(v8::Local<v8::String>, const v8::PropertyCallbackInfo<v8::Value>& info) {
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope scope(isolate);
	JSInterleavedVideoFrame* self = UnwrapSelf<JSInterleavedVideoFrame>(info.This());
	// Make sure we're writeable.
	self->MakeWriteable(isolate);
	if (self->dataInstance.IsEmpty()) {
		info.GetReturnValue().Set(v8::Null());
	} else {
		info.GetReturnValue().Set(self->dataInstance);
	}
}

bool JSInterleavedVideoFrame::MakeWriteable(v8::Isolate* isolate) {
	if (released) {
		return false;
	}
	if (dataInstance.IsEmpty()) {
		TRACE("Data instance is emtpy, building data\n");
		v8::HandleScope scope(isolate);
		// If I could make this "copy on write" I'd love to, but there doesn't
		// seem to really be a way to do that, so instead just always make it
		// writable.
		TRACE("Getting the write pointer\n");
		BYTE* dataptr;
		if (!frame->IsWritable()) {
			TRACE("Had to make the frame writeable\n");
			JSVEnvironment::GetCurrent()->GetAVSScriptEnvironment()->MakeWritable(&frame);
		}
		dataptr = frame->GetWritePtr();
		if (dataptr == NULL) {
			JSV_ERROR("Write pointer is null!");
			// Leave the handle empty
			return false;
		}
		// Create and persist the array buffer
		v8::Handle<v8::ArrayBuffer> data = WrapData(isolate, dataptr, frame->GetPitch() * frame->GetHeight());
		dataInstance.Reset(isolate, data);
	}
	return true;
}

v8::Handle<v8::ObjectTemplate> JSInterleavedVideoFrame::CreateTemplate(v8::Isolate* isolate) {
	v8::HandleScope scope(isolate);
	v8::Handle<v8::ObjectTemplate> templ = v8::ObjectTemplate::New();
	templ->Set(JSPROP_NAME("release"), v8::FunctionTemplate::New(JSRelease), JSPROP_READONLY);
	templ->Set(JSPROP_NAME("planar"), v8::False(), JSPROP_READONLY);
	templ->Set(JSPROP_NAME("interleaved"), v8::True(), JSPROP_READONLY);
	templ->SetAccessor(JSPROP_NAME("data"), JSGetData);
	CreateTemplateDefaultFields(templ);
	templ->SetInternalFieldCount(1);
	return scope.Close(templ);
}

JSPlanarVideoFrame::JSPlanarVideoFrame(PVideoFrame aFrame, const VideoInfo& vi, v8::Isolate* isolate, v8::Handle<v8::ObjectTemplate> templ)
	: JSVideoFrame(aFrame, vi, isolate, templ) {
	v8::HandleScope scope(isolate);
	PopulateInstance(v8::Local<v8::Object>::New(isolate, instance));
}

JSPlanarVideoFrame::~JSPlanarVideoFrame() {
	TRACE_MEM();
	dataYInstance.Dispose();
	dataUInstance.Dispose();
	dataVInstance.Dispose();
}

void JSPlanarVideoFrame::PopulateInstance(v8::Handle<v8::Object> inst) {
	// We really need to populate the various planes
	PopulatePlaneInstance(inst->Get(v8::String::New("y"))->ToObject(), PLANAR_Y);
	PopulatePlaneInstance(inst->Get(v8::String::New("u"))->ToObject(), PLANAR_U);
	PopulatePlaneInstance(inst->Get(v8::String::New("v"))->ToObject(), PLANAR_V);
	inst->SetInternalField(0, v8::External::New(this));
}

void JSPlanarVideoFrame::PopulatePlaneInstance(v8::Handle<v8::Object> inst, int plane) {
	inst->Set(v8::String::New("pitch"), v8::Int32::New(frame->GetPitch(plane)), v8::PropertyAttribute::ReadOnly);
	inst->Set(v8::String::New("rowSize"), v8::Int32::New(frame->GetRowSize(plane)), v8::PropertyAttribute::ReadOnly);
	inst->Set(v8::String::New("height"), v8::Int32::New(frame->GetHeight(plane)), v8::PropertyAttribute::ReadOnly);
	inst->Set(v8::String::New("bitsPerPixel"), v8::Int32::New(vi.BitsPerPixel()), v8::PropertyAttribute::ReadOnly);
	inst->Set(v8::String::New("bytesPerPixel"), v8::Int32::New(vi.BytesFromPixels(1)), v8::PropertyAttribute::ReadOnly);
	inst->SetInternalField(0, v8::External::New(this));
}

void JSPlanarVideoFrame::Release() {
	TRACE_MEM();
	JSVideoFrame::Release();
	v8::Isolate* isolate = v8::Isolate::GetCurrent();
	v8::HandleScope scope(isolate);
	if (!dataYInstance.IsEmpty()) {
		v8::Local<v8::ArrayBuffer>::New(isolate, dataYInstance)->Neuter();
		v8::Local<v8::ArrayBuffer>::New(isolate, dataUInstance)->Neuter();
		v8::Local<v8::ArrayBuffer>::New(isolate, dataVInstance)->Neuter();
	}
}

void JSPlanarVideoFrame::JSRelease(const v8::FunctionCallbackInfo<v8::Value>& info) {
	JSPlanarVideoFrame* self = UnwrapSelf<JSPlanarVideoFrame>(info.This());
	self->Release();
}

#define PLANAR_JS_GET_DATA(PLANE)		void JSPlanarVideoFrame::JSGetData ## PLANE (v8::Local<v8::String>, const v8::PropertyCallbackInfo<v8::Value>& info) { \
	v8::Isolate* isolate = info.GetIsolate(); \
	JSPlanarVideoFrame* self = UnwrapSelf<JSPlanarVideoFrame>(info.This()); \
	v8::HandleScope scope(isolate); \
	self->MakeWriteable(info.GetIsolate()); \
	if (self->data ## PLANE ## Instance.IsEmpty()) { \
		info.GetReturnValue().Set(v8::Null()); \
	} else { \
		info.GetReturnValue().Set(self->data ## PLANE ## Instance); \
	} \
}

PLANAR_JS_GET_DATA(Y)
PLANAR_JS_GET_DATA(U)
PLANAR_JS_GET_DATA(V)

void JSPlanarVideoFrame::MakeWriteable(v8::Isolate* isolate) {
	if (released) {
		return;
	}
	if (dataYInstance.IsEmpty()) {
		v8::HandleScope scope(isolate);
		if (!frame->IsWritable()) {
			TRACE("Had to make the frame writeable\n");
			JSVEnvironment::GetCurrent()->GetAVSScriptEnvironment()->MakeWritable(&frame);
		}
		// For planar data, we always grab the planes all at once. The reason
		// for this is simple: AviSynth doesn't say so, but you need to grab
		// the pointer to the Y plane *FIRST* and then the U and V planes.
		// Rather than force that down to the JavaScript level, we just create
		// wrappers for the three planes all at once.
		v8::Handle<v8::ArrayBuffer> dataY = WrapData(isolate, frame->GetWritePtr(PLANAR_Y), frame->GetRowSize(PLANAR_Y) * frame->GetHeight(PLANAR_Y));
		v8::Handle<v8::ArrayBuffer> dataU = WrapData(isolate, frame->GetWritePtr(PLANAR_U), frame->GetRowSize(PLANAR_U) * frame->GetHeight(PLANAR_U));
		v8::Handle<v8::ArrayBuffer> dataV = WrapData(isolate, frame->GetWritePtr(PLANAR_V), frame->GetRowSize(PLANAR_V) * frame->GetHeight(PLANAR_V));
		// And finally, persist everything:
		dataYInstance.Reset(isolate, dataY);
		dataUInstance.Reset(isolate, dataU);
		dataVInstance.Reset(isolate, dataV);
	}
}

v8::Handle<v8::ObjectTemplate> JSPlanarVideoFrame::CreateTemplate(v8::Isolate* isolate) {
	v8::HandleScope scope(isolate);
	v8::Handle<v8::ObjectTemplate> templ = v8::ObjectTemplate::New();
	templ->Set(v8::String::New("release"), v8::FunctionTemplate::New(JSRelease));
	templ->Set(v8::String::New("planar"), v8::True(), v8::PropertyAttribute::ReadOnly);
	templ->Set(v8::String::New("interleaved"), v8::False(), v8::PropertyAttribute::ReadOnly);
	templ->SetInternalFieldCount(1);
	v8::Handle<v8::ObjectTemplate> yPlane = v8::ObjectTemplate::New();
	yPlane->SetAccessor(v8::String::New("data"), JSGetDataY);
	yPlane->SetInternalFieldCount(1);
	templ->Set("y", yPlane);
	v8::Handle<v8::ObjectTemplate> uPlane = v8::ObjectTemplate::New();
	uPlane->SetAccessor(v8::String::New("data"), JSGetDataU);
	uPlane->SetInternalFieldCount(1);
	templ->Set("u", uPlane);
	v8::Handle<v8::ObjectTemplate> vPlane = v8::ObjectTemplate::New();
	vPlane->SetAccessor(v8::String::New("data"), JSGetDataV);
	vPlane->SetInternalFieldCount(1);
	templ->Set("v", vPlane);
	return scope.Close(templ);
}

};