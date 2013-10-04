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
#include "JSVEnvironment.h"
#include "JSVideoFrame.h"
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
	NOPSimpleRenderingContext() : JSSimpleRenderingContext(0,0) { }
	~NOPSimpleRenderingContext() { }
	// FIXME: Returning false here is probably wrong, as it will cause spurious errors
	// (fillRect will silently do nothing, but any drawImage will fail with a warning about
	// colorspaces.)
	bool CanDrawImageFrom(JSVideoFrame& otherFrame) { return false; }
	void FillRect(UINT32 color, int x, int y, int width, int height) { }
	void DrawImage(JSVideoFrame& otherFrame, int sx, int sy, int sw, int sh, int dx, int dy) { }
};

// We only need the single NOP renderer, so just make it a local variable.
NOPSimpleRenderingContext nopRenderingContext;

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
	int w = args[3]->Int32Value();
	int h = args[4]->Int32Value();
	TRACE("FillRect(0x%08X, %d, %d, %d, %d)\n", color, x, y, w, h);
	JSSimpleRenderingContext* self = UnwrapSelf<JSSimpleRenderingContext>(args.This());
	int width = self->width;
	int height = self->height;
	// If the rect is completely outside the frame, do nothing.
	if (x >= width || y >= height) {
		return;
	}
	// Clip the rect to fit within the frame.
	if (x < 0) {
		// This is actually subtracting the part we're off from the width, as x
		// is negative here.
		w += x;
		x = 0;
	}
	if (y < 0) {
		// See above.
		h += y;
		y = 0;
	}
	// Clip against the edges
	if (x + w >= width) {
		w = width - x;
	}
	if (y + h >= height) {
		h = height - y;
	}
	// If we've shrunk the rect to nothing, then do nothing.
	if (w <= 0 || h <= 0) {
		return;
	}
	TRACE("FillRect: Clipped To (%d, %d, %d, %d)\n", x, y, w, h);
	self->FillRect(color, x, y, w, h);
}

void JSSimpleRenderingContext::JSDrawImage(const v8::FunctionCallbackInfo<v8::Value>& args) {
	v8::HandleScope scope(args.GetIsolate());
	JSSimpleRenderingContext* self = UnwrapSelf<JSSimpleRenderingContext>(args.This());
	JSVideoFrame* source;
	int sx, sy, sw, sh, dx, dy;
	int argc = args.Length();
	// Before doing anything, make sure the first argument 1) exists and 2) is a frame
	if (argc < 1) {
		v8::ThrowException(v8::Exception::Error(v8::String::New("Missing frame")));
		return;
	}
	if (!(args[0]->IsObject() && JSVideoFrame::IsWrappedVideoFrame(args[0]->ToObject()))) {
		v8::ThrowException(v8::Exception::TypeError(v8::String::New("frame must be a frame")));
		return;
	}
	source = JSVideoFrame::UnwrapVideoFrame(args[0]->ToObject());
	// We need to figure out which "overload" we are. The signatures allowed are:
	if (argc == 3) {
		// drawImage(frame, x, y)
		sx = 0;
		sy = 0;
		sw = source->GetWidth();
		sh = source->GetHeight();
		dx = args[1]->Int32Value();
		dy = args[2]->Int32Value();
	} else if (argc == 7) {
		// drawImage(frame, sx, sy, sw, sh, dx, dy)
		sx = args[1]->Int32Value();
		sy = args[2]->Int32Value();
		sw = args[3]->Int32Value();
		sh = args[4]->Int32Value();
		dx = args[5]->Int32Value();
		dy = args[6]->Int32Value();
	} else {
		v8::ThrowException(v8::Exception::Error(v8::String::New("Bad number of arguments to drawImage")));
		return;
	}
	TRACE("DrawImage (%d, %d) [%d x %d] => (%d, %d)\n", sx, sy, sw, sh, dx, dy);
	// Determine if we can draw using the source image here, so that a
	// completely incorrect call is reported as an error first, rather than
	// "hiding" that fact because the source image is invalid.
	if (!self->CanDrawImageFrom(*source)) {
		v8::ThrowException(v8::Exception::Error(v8::String::New("Cannot drawImage from the given frame, colorspaces must match")));
		return;
	}
	// Now do some basic checks.
	int width = self->width;
	int height = self->height;
	int srcWidth = source->GetWidth();
	int srcHeight = source->GetHeight();
	// First off, we need to flip the ys to match the expected dimensions
	sy = srcHeight - sy - sh;
	dy = height - dy - sh;
	TRACE("Clipping within source %dx%d, destination %dx%d\n", srcWidth, srcHeight, width, height);
	// Clip the requested region to the source image.
	if (sx < 0) {
		// Effectively subtract the region off the edge from the width.
		sw += sx;
		sx = 0;
	}
	if (sy < 0) {
		sh += sy;
		sy = 0;
	}
	// Next see if we're drawing off the edge of the destination
	if (dx < 0) {
		// In this case, we want to increase the source x to where it'd be at
		// 0.
		sx -= dx;
		dx = 0;
	}
	if (dy < 0) {
		// Same as above.
		sy -= dy;
		dy = 0;
	}
	// Now see if we're off the far edge.
	if ((dx + sw) > width) {
		// Drop the source width to what's available.
		sw = width - dx;
	}
	if ((dy + sh) > height) {
		// Drop the source height to what's available.
		sh = height - dy;
	}
	// Now check to make sure the source width/height is available...
	if ((sx + sw) > srcWidth) {
		sw = srcWidth - sx;
	}
	if ((sy + sh) > srcHeight) {
		sh = srcHeight - sy;
	}
	TRACE("DrawImage clipped to (%d, %d) [%d x %d] => (%d, %d)\n", sx, sy, sw, sh, dx, dy);
	// And finally see if we're left with anything at all
	if (sw <= 0 || sh <= 0) {
		return;
	}
	self->DrawImage(*source, sx, sy, sw, sh, dx, dy);
}

//
// RGB32 Implementation
//

RGB32SimpleRenderingContext::RGB32SimpleRenderingContext(VideoFrame* frame, v8::Handle<v8::Object> self)
	: JSSimpleRenderingContext(frame->GetRowSize() / 4, frame->GetHeight()), frameData(frame->GetWritePtr()), pitch(frame->GetPitch()),
	totalSize(frame->GetPitch() * (frame->GetHeight() - 1) + frame->GetRowSize()) {
	WrapSelf(self);
}

RGB32SimpleRenderingContext::~RGB32SimpleRenderingContext() {
	// Does nothing.
}

void RGB32SimpleRenderingContext::FillRect(UINT32 color, int x, int y, int w, int h) {
	// The API wants to y to be on the top, but RGB32 frames are stored bottom
	// to top, so flip the Y.
	y = height - y - h;
	// We render in rows
	for (int r = 0; r < h; r++,y++) {
		UINT32* data = (UINT32*)(frameData + y * pitch + x * 4);
		for (int c = 0; c < w; c++, data++) {
			*data = color;
		}
	}
}

bool RGB32SimpleRenderingContext::CanDrawImageFrom(JSVideoFrame& otherFrame) {
	return otherFrame.GetVideoInfo().IsRGB32();
}

void RGB32SimpleRenderingContext::DrawImage(JSVideoFrame& sourceFrame, int sx, int sy, int sw, int sh, int dx, int dy) {
	// NOTE: By this point, CanDrawImageFrom has already been queried to ensure
	// that the source frame is also RGB32. In the future we might also support
	// RGB24 sources, but we're never going to allow YUV sources - instead
	// force the user to decide how they intend to convert a YUV source to RGB
	// themselves rather than do some form of terrible auto-convert.
	// We need to check is if we're being asked to draw on ourselves, which is
	// going to be somewhat interesting to do but technically possible (because
	// we can't use bitblt if we're overlapping). Basically, there's a single
	// special case, and that's rendering from ourselves to an area that
	// overlaps ourselves. Still allowed, but a special case.
	const BYTE* sourceData = sourceFrame.GetVideoFrame()->GetReadPtr();
	size_t sourcePitch = sourceFrame.GetVideoFrame()->GetPitch();
	if (sourceData == frameData) {
		// We're looking at the same frame data, which means that if the
		// source and destination overlap, we can't use memcpy as the
		// results are undefined in that case.
		if (sx == dx && sy == dy) {
			// er... ok
			return;
		}
		// FIXME: Check to see if the areas REALLY overlap.
		for (int y = 0; y < sh; y++, sy++, dy++) {
			UINT32* srcData = (UINT32*)(frameData + sy * pitch + sx * 4);
			UINT32* destData = (UINT32*)(frameData + dy * pitch + dx * 4);
			for (int x = 0; x < sw; x++) {
				*destData = *srcData;
				srcData++;
				destData++;
			}
		}
		return;
	}
	TRACE("Render at %d,%d to %d,%d (%dx%d)\n", sx, sy, dx, dy, sw, sh);
	// If we're here, we can safely memcpy, so let's do that.
	for (int y = 0; y < sh; y++) {
		memcpy_s(frameData + ((dy+y) * pitch + dx * 4), totalSize, sourceData + ((sy+y) * sourcePitch + sx * 4), sw*4);
	}
}

};
