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

JSVideoFrame::JSVideoFrame(PVideoFrame aFrame, v8::Isolate* isolate, v8::Handle<v8::ObjectTemplate> templ)
	: frame(aFrame), madeWeak(false), released(false) {
	v8::HandleScope scope(isolate);
	// Create a new instance
	v8::Handle<v8::Object> inst = v8::Local<v8::ObjectTemplate>::New(isolate, templ)->NewInstance();
	inst->SetInternalField(0, v8::External::New(this));
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
	return inst;
}

void JSVideoFrame::Release() {
	if (!released) {
		frame = NULL;
		released = true;
	}
}

v8::Handle<v8::ArrayBuffer> JSVideoFrame::WrapData(v8::Isolate* isolate, BYTE* data, int length) {
	v8::HandleScope scope(isolate);
	v8::Handle<v8::ArrayBuffer> result = v8::ArrayBuffer::New(data, length);
	// To ensure somewhat cleaner garbage collection, add ourselves as a property.
	// This makes sure anyone hanging onto a data buffer (don't do that!) but not the
	// frame won't have the buffer deallocated/reused out from under them.
	// It also makes for a great potential memory leak!
	result->Set(v8::String::New("frame"), GetInstance(isolate));
	return scope.Close(result);
}

JSInterleavedVideoFrame::JSInterleavedVideoFrame(PVideoFrame aFrame, v8::Isolate* isolate, v8::Handle<v8::ObjectTemplate> templ)
	: JSVideoFrame(aFrame, isolate, templ) {
}

JSInterleavedVideoFrame::~JSInterleavedVideoFrame() {
	dataBufferInstance.Dispose();
	dataInstance.Dispose();
}

void JSInterleavedVideoFrame::Release() {
	JSVideoFrame::Release();
	v8::Isolate* isolate = v8::Isolate::GetCurrent();
	v8::HandleScope scope(isolate);
	if (!dataInstance.IsEmpty()) {
		v8::Local<v8::ArrayBuffer>::New(isolate, dataBufferInstance)->Neuter();
	}
}

void JSInterleavedVideoFrame::JSRelease(const v8::FunctionCallbackInfo<v8::Value>& info) {
	JSInterleavedVideoFrame* self = UnwrapSelf<JSInterleavedVideoFrame>(info.This());
	self->Release();
}

void JSInterleavedVideoFrame::GetPitch(const v8::FunctionCallbackInfo<v8::Value>& info) {
	JSInterleavedVideoFrame* self = UnwrapSelf<JSInterleavedVideoFrame>(info.This());
	if (info.Length() >= 1) {
		// If we have an argument, it needs to be the plane (int constant)
		info.GetReturnValue().Set(v8::Int32::New(self->frame->GetPitch(info[0]->Int32Value())));
	} else {
		info.GetReturnValue().Set(v8::Int32::New(self->frame->GetPitch()));
	}
}

void JSInterleavedVideoFrame::GetRowSize(const v8::FunctionCallbackInfo<v8::Value>& info) {
	JSInterleavedVideoFrame* self = UnwrapSelf<JSInterleavedVideoFrame>(info.This());
	if (info.Length() >= 1) {
		// If we have an argument, it needs to be the plane
		info.GetReturnValue().Set(v8::Int32::New(self->frame->GetRowSize(info[0]->Int32Value())));
	} else {
		info.GetReturnValue().Set(v8::Int32::New(self->frame->GetRowSize()));
	}
}

void JSInterleavedVideoFrame::GetHeight(const v8::FunctionCallbackInfo<v8::Value>& info) {
	JSInterleavedVideoFrame* self = UnwrapSelf<JSInterleavedVideoFrame>(info.This());
	if (info.Length() >= 1) {
		// If we have an argument, it needs to be the plane
		info.GetReturnValue().Set(v8::Int32::New(self->frame->GetHeight(info[0]->Int32Value())));
	} else {
		info.GetReturnValue().Set(v8::Int32::New(self->frame->GetHeight()));
	}
}

void JSInterleavedVideoFrame::JSGetData(v8::Local<v8::String>, const v8::PropertyCallbackInfo<v8::Value>& info) {
	JSInterleavedVideoFrame* self = UnwrapSelf<JSInterleavedVideoFrame>(info.This());
	v8::HandleScope scope(info.GetIsolate());
	v8::Handle<v8::Uint8Array> result = self->GetData(info.GetIsolate());
	if (result.IsEmpty()) {
		info.GetReturnValue().Set(v8::Null());
	} else {
		info.GetReturnValue().Set(result);
	}
}

v8::Handle<v8::Uint8Array> JSInterleavedVideoFrame::GetData(v8::Isolate* isolate) {
	if (released) {
		return v8::Handle<v8::Uint8Array>();
	}
	if (dataInstance.IsEmpty()) {
		v8::HandleScope scope(isolate);
		// If I could make this "copy on write" I'd love to, but there doesn't
		// seem to really be a way to do that, so instead just always make it
		// writable.
		v8::Handle<v8::ArrayBuffer> dataBuffer = WrapData(isolate, frame->GetWritePtr(), frame->GetRowSize() * frame->GetHeight());
		// And create the views
		v8::Handle<v8::Uint8Array> data = v8::Uint8Array::New(dataBuffer, 0, dataBuffer->ByteLength());
		// And finally, persist them:
		dataBufferInstance.Reset(isolate, dataBuffer);
		dataInstance.Reset(isolate, data);
		// And, since we still have this lovely scope here, do the return here rather than fall through
		return scope.Close(data);
	}
	// If we're here, we can use a cached version.
	return v8::Local<v8::Uint8Array>::New(isolate, dataInstance);
}

v8::Handle<v8::ObjectTemplate> JSInterleavedVideoFrame::CreateTemplate(v8::Isolate* isolate) {
	v8::HandleScope scope(isolate);
	v8::Handle<v8::ObjectTemplate> templ = v8::ObjectTemplate::New();
	templ->Set(v8::String::New("getPitch"), v8::FunctionTemplate::New(GetPitch));
	templ->Set(v8::String::New("getRowSize"), v8::FunctionTemplate::New(GetRowSize));
	templ->Set(v8::String::New("getHeight"), v8::FunctionTemplate::New(GetHeight));
	templ->Set(v8::String::New("release"), v8::FunctionTemplate::New(JSRelease));
	templ->Set(v8::String::New("planar"), v8::False(), v8::PropertyAttribute::ReadOnly);
	templ->Set(v8::String::New("interleaved"), v8::True(), v8::PropertyAttribute::ReadOnly);
	templ->SetAccessor(v8::String::New("data"), JSGetData);
	return scope.Close(templ);
}

JSPlanarVideoFrame::JSPlanarVideoFrame(PVideoFrame aFrame, v8::Isolate* isolate, v8::Handle<v8::ObjectTemplate> templ)
	: JSVideoFrame(aFrame, isolate, templ) {
}

JSPlanarVideoFrame::~JSPlanarVideoFrame() {
	dataYBufferInstance.Dispose();
	dataUBufferInstance.Dispose();
	dataVBufferInstance.Dispose();
	dataYInstance.Dispose();
	dataUInstance.Dispose();
	dataVInstance.Dispose();
}

void JSPlanarVideoFrame::Release() {
	JSVideoFrame::Release();
	v8::Isolate* isolate = v8::Isolate::GetCurrent();
	v8::HandleScope scope(isolate);
	if (!dataYInstance.IsEmpty()) {
		v8::Local<v8::ArrayBuffer>::New(isolate, dataYBufferInstance)->Neuter();
		v8::Local<v8::ArrayBuffer>::New(isolate, dataUBufferInstance)->Neuter();
		v8::Local<v8::ArrayBuffer>::New(isolate, dataVBufferInstance)->Neuter();
	}
}

void JSPlanarVideoFrame::JSRelease(const v8::FunctionCallbackInfo<v8::Value>& info) {
	JSPlanarVideoFrame* self = UnwrapSelf<JSPlanarVideoFrame>(info.This());
	self->Release();
}

void JSPlanarVideoFrame::GetPitch(const v8::FunctionCallbackInfo<v8::Value>& info) {
	JSPlanarVideoFrame* self = UnwrapSelf<JSPlanarVideoFrame>(info.This());
	if (info.Length() >= 1) {
		// If we have an argument, it needs to be the plane (int constant)
		info.GetReturnValue().Set(v8::Int32::New(self->frame->GetPitch(info[0]->Int32Value())));
	} else {
		info.GetReturnValue().Set(v8::Int32::New(self->frame->GetPitch()));
	}
}

void JSPlanarVideoFrame::GetRowSize(const v8::FunctionCallbackInfo<v8::Value>& info) {
	JSPlanarVideoFrame* self = UnwrapSelf<JSPlanarVideoFrame>(info.This());
	if (info.Length() >= 1) {
		// If we have an argument, it needs to be the plane
		info.GetReturnValue().Set(v8::Int32::New(self->frame->GetRowSize(info[0]->Int32Value())));
	} else {
		info.GetReturnValue().Set(v8::Int32::New(self->frame->GetRowSize()));
	}
}

void JSPlanarVideoFrame::GetHeight(const v8::FunctionCallbackInfo<v8::Value>& info) {
	JSPlanarVideoFrame* self = UnwrapSelf<JSPlanarVideoFrame>(info.This());
	if (info.Length() >= 1) {
		// If we have an argument, it needs to be the plane
		info.GetReturnValue().Set(v8::Int32::New(self->frame->GetHeight(info[0]->Int32Value())));
	} else {
		info.GetReturnValue().Set(v8::Int32::New(self->frame->GetHeight()));
	}
}

#define PLANAR_JS_GET_DATA(PLANE)		void JSPlanarVideoFrame::JSGetData ## PLANE (v8::Local<v8::String>, const v8::PropertyCallbackInfo<v8::Value>& info) { \
	JSPlanarVideoFrame* self = UnwrapSelf<JSPlanarVideoFrame>(info.This()); \
	v8::HandleScope scope(info.GetIsolate()); \
	v8::Handle<v8::Uint8Array> result = self->GetData(info.GetIsolate(), PLANAR_ ## PLANE); \
	if (result.IsEmpty()) { \
		info.GetReturnValue().Set(v8::Null()); \
	} else { \
		info.GetReturnValue().Set(result); \
	} \
}

PLANAR_JS_GET_DATA(Y)
PLANAR_JS_GET_DATA(U)
PLANAR_JS_GET_DATA(V)

v8::Handle<v8::Uint8Array> JSPlanarVideoFrame::GetData(v8::Isolate* isolate, int plane) {
	if (released) {
		return v8::Handle<v8::Uint8Array>();
	}
	if (dataYInstance.IsEmpty()) {
		v8::HandleScope scope(isolate);
		// For planar data, we always grab the planes all at once. The reason
		// for this is simple: AviSynth doesn't say so, but you need to grab
		// the pointer to the Y plane *FIRST* and then the U and V planes.
		// Rather than force that down to the JavaScript level, we just create
		// wrappers for the three planes all at once.
		v8::Handle<v8::ArrayBuffer> dataYBuffer = WrapData(isolate, frame->GetWritePtr(PLANAR_Y), frame->GetRowSize(PLANAR_Y) * frame->GetHeight(PLANAR_Y));
		v8::Handle<v8::ArrayBuffer> dataUBuffer = WrapData(isolate, frame->GetWritePtr(PLANAR_U), frame->GetRowSize(PLANAR_U) * frame->GetHeight(PLANAR_U));
		v8::Handle<v8::ArrayBuffer> dataVBuffer = WrapData(isolate, frame->GetWritePtr(PLANAR_V), frame->GetRowSize(PLANAR_V) * frame->GetHeight(PLANAR_V));
		// And create the views...
		v8::Handle<v8::Uint8Array> dataY = v8::Uint8Array::New(dataYBuffer, 0, dataYBuffer->ByteLength());
		v8::Handle<v8::Uint8Array> dataU = v8::Uint8Array::New(dataUBuffer, 0, dataUBuffer->ByteLength());
		v8::Handle<v8::Uint8Array> dataV = v8::Uint8Array::New(dataVBuffer, 0, dataVBuffer->ByteLength());
		// And finally, persist everything:
		dataYBufferInstance.Reset(isolate, dataYBuffer);
		dataUBufferInstance.Reset(isolate, dataUBuffer);
		dataVBufferInstance.Reset(isolate, dataVBuffer);
		dataYInstance.Reset(isolate, dataY);
		dataUInstance.Reset(isolate, dataU);
		dataVInstance.Reset(isolate, dataV);
		// And, since we still have this lovely scope here, do the return here rather than fall through
		v8::Handle<v8::Uint8Array> result;
		switch (plane) {
		case PLANAR_Y:
			result = dataY;
			break;
		case PLANAR_U:
			result = dataU;
			break;
		case PLANAR_V:
			result = dataV;
			break;
		default:
			result = v8::Local<v8::Uint8Array>();
		}
		return scope.Close(result);
	}
	// If we're here, we can use a cached version.
	switch (plane) {
	case PLANAR_Y:
		return v8::Local<v8::Uint8Array>::New(isolate, dataYInstance);
	case PLANAR_U:
		return v8::Local<v8::Uint8Array>::New(isolate, dataUInstance);
	case PLANAR_V:
		return v8::Local<v8::Uint8Array>::New(isolate, dataVInstance);
	default:
		return v8::Local<v8::Uint8Array>();
	}
}

v8::Handle<v8::ObjectTemplate> JSPlanarVideoFrame::CreateTemplate(v8::Isolate* isolate) {
	v8::HandleScope scope(isolate);
	v8::Handle<v8::ObjectTemplate> templ = v8::ObjectTemplate::New();
	templ->Set(v8::String::New("getPitch"), v8::FunctionTemplate::New(GetPitch));
	templ->Set(v8::String::New("getRowSize"), v8::FunctionTemplate::New(GetRowSize));
	templ->Set(v8::String::New("getHeight"), v8::FunctionTemplate::New(GetHeight));
	templ->Set(v8::String::New("release"), v8::FunctionTemplate::New(JSRelease));
	templ->Set(v8::String::New("planar"), v8::True(), v8::PropertyAttribute::ReadOnly);
	templ->Set(v8::String::New("interleaved"), v8::False(), v8::PropertyAttribute::ReadOnly);
	templ->SetAccessor(v8::String::New("dataY"), JSGetDataY);
	templ->SetAccessor(v8::String::New("dataU"), JSGetDataU);
	templ->SetAccessor(v8::String::New("dataV"), JSGetDataV);
	return scope.Close(templ);
}

};