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
#include "jsutil.h"

namespace jsv {

/**
 * A simple rendering context that does nothing. Why would you want this?
 * Simple - when a JavaScript context is released, it is replaced with an
 * instance of the NOP context, preventing calls from doing anything.
 */
class NOPSimpleRenderingContext : public JSSimpleRenderingContext {
public:
	NOPSimpleRenderingContext() { }
	~NOPSimpleRenderingContext() { }
	void FillRect(UINT32 color, int x, int y, int width, int height) { }
	void DrawImage(PVideoFrame otherFrame, int x, int y) { }
};

// We only need the single NOP renderer, so just make it a local variable.
NOPSimpleRenderingContext nopRenderingContext;
	
JSSimpleRenderingContext::JSSimpleRenderingContext(void) { }

JSSimpleRenderingContext::~JSSimpleRenderingContext(void) {
	if (!(jsThis.IsEmpty())) {
		v8::Isolate* isolate = v8::Isolate::GetCurrent();
		v8::HandleScope scope(isolate);
		v8::Handle<v8::Object> self = v8::Local<v8::Object>::New(isolate, jsThis);
		self->SetInternalField(0, v8::External::New(&nopRenderingContext));
		// And kill our handle, we no longer hold on to this.
		jsThis.Dispose();
	}
}

void JSSimpleRenderingContext::WrapSelf(v8::Handle<v8::Object> self) {
	v8::Isolate* isolate = v8::Isolate::GetCurrent();
	v8::HandleScope scope(isolate);
	self->SetInternalField(0, v8::External::New(this));
	jsThis.Reset(isolate, self);
}

v8::Handle<v8::ObjectTemplate> JSSimpleRenderingContext::CreateTemplate(v8::Isolate* isolate) {
	v8::HandleScope scope(isolate);
	v8::Handle<v8::ObjectTemplate> templ = v8::ObjectTemplate::New();
	templ->Set("fillRect", v8::FunctionTemplate::New(JSFillRect));
	templ->Set("drawImage", v8::FunctionTemplate::New(JSDrawImage));
	templ->SetInternalFieldCount(1);
	return scope.Close(templ);
}

void JSSimpleRenderingContext::JSFillRect(const v8::FunctionCallbackInfo<v8::Value>& args) {
	v8::HandleScope scope(args.GetIsolate());
	// This simply unpacks the values and forwards them.
	if (args.Length() < 5) {
		v8::ThrowException(v8::Exception::Error(v8::String::New("Not enough arguments")));
		return;
	}
	// Unpack args. (Based on examples I'm assuming this works?)
	UINT32 color = args[0]->Uint32Value();
	int x = args[1]->Int32Value();
	int y = args[2]->Int32Value();
	int width = args[3]->Int32Value();
	int height = args[4]->Int32Value();
	JSSimpleRenderingContext* self = UnwrapSelf<JSSimpleRenderingContext>(args.This());
	self->FillRect(color, x, y, width, height);
}

void JSSimpleRenderingContext::JSDrawImage(const v8::FunctionCallbackInfo<v8::Value>& args) {
}

//
// RGB32 Implementation
//

RGB32SimpleRenderingContext::RGB32SimpleRenderingContext(VideoFrame* frame, v8::Handle<v8::Object> self)
	: frameData(frame->GetWritePtr()), pitch(frame->GetPitch()), frameWidth(frame->GetRowSize() / 4), frameHeight(frame->GetHeight()) {
	WrapSelf(self);
}

RGB32SimpleRenderingContext::~RGB32SimpleRenderingContext() {
	// Does nothing.
}

void RGB32SimpleRenderingContext::FillRect(UINT32 color, int x, int y, int width, int height) {
	TRACE("FillRect(%08X, %d, %d, %d, %d)\n", color, x, y, width, height);
	// The API wants to y to be on the top, but RGB32 frames are stored bottom to top.
	// Flip the Y and then clip as normal.
	y = frameHeight - y - height;
	// Clip.
	if (x >= frameWidth || y >= frameHeight) {
		// If we're off the edge, do nothing.
		return;
	}
	if (x < 0) {
		// Clip that way
		width += x;
		x = 0;
	}
	if (y < 0) {
		height += y;
		y = 0;
	}
	if (x + width >= frameWidth) {
		width = frameWidth - x;
	}
	if (y + height >= frameHeight) {
		height = frameHeight - y;
	}
	if (width <= 0 || height <= 0) {
		return;
	}
	TRACE("FillRect: Clipped To (%d, %d, %d, %d)\n", x, y, width, height);
	// We render in rows
	for (int r = 0; r < height; r++,y++) {
		uint32_t* data = (uint32_t*)(frameData + y * pitch + x * 4);
		for (int c = 0; c < width; c++, data++) {
			*data = color;
		}
	}
}

void RGB32SimpleRenderingContext::DrawImage(PVideoFrame otherFrame, int x, int y) {
}

};
