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
#pragma once

namespace jsv {

/**
 * Abstract base class for the simple rendering context, providing just the
 * bridge between JavaScript and the native code.
 */
class JSSimpleRenderingContext {
public:
	JSSimpleRenderingContext(int aWidth, int aHeight) : width(aWidth), height(aHeight) { }
	/**
	 * Deletes the rendering context.
	 */
	~JSSimpleRenderingContext(void);
	v8::Handle<v8::Object> GetInstance(v8::Isolate* isolate) { return v8::Local<v8::Object>::New(isolate, jsThis); }
	static v8::Handle<v8::ObjectTemplate> CreateTemplate(v8::Isolate*);
	/**
	 * Determine if the given frame is an allowed source for DrawImage.
	 */
	virtual bool CanDrawImageFrom(JSVideoFrame& other) = 0;
	/**
	 * Fill the given rectangle with a color. Note: this MUST be clipped within
	 * the frame rect. The JSFillRect JavaScript wrapper performs this clipping,
	 * which means that this call is allowed to assume that this call provides
	 * a rectangle that is located entirely within the frame.
	 */
	virtual void FillRect(UINT32 color, int x, int y, int width, int height) = 0;
	/**
	 * Render a chunk of a different frame onto this frame. Note: the
	 * sx,sy,sw,sh fields must define an area entirely within the source frame,
	 * and the dx,dy part must lie within this frame. The JSDrawFrame
	 * JavaScript wrapper enforces this constraint.
	 */
	virtual void DrawImage(JSVideoFrame& sourceFrame, int sx, int sy, int sw, int sh, int dx, int dy) = 0;
protected:
	/**
	 * Embed this into the given JavaScript object. This can't be done during
	 * the constructor because it needs to happen after the entire class chain
	 * has been initialized, instead it's up to the final constructor to invoke
	 * this method.
	 */
	void WrapSelf(v8::Handle<v8::Object> self);
	/**
	 * The JavaScript object this C++ class represents.
	 */
	v8::Persistent<v8::Object> jsThis;
	/**
	 * The width of the frame in pixels. This is used to properly "crop" calls
	 * to be within the boundaries of the image.
	 */
	int width;
	/**
	 * The height of the frame in pixels. This is used to properly "crop" calls
	 * to be within the boundaries of the image.
	 */
	int height;
private:
	static void JSFillRect(const v8::FunctionCallbackInfo<v8::Value>&);
	static void JSDrawImage(const v8::FunctionCallbackInfo<v8::Value>&);
};

/**
 * Implementation of the simple rendering context that can draw onto an RGB32
 * frame.
 */
class RGB32SimpleRenderingContext : public JSSimpleRenderingContext {
public:
	/**
	 * Create a new rendering context for a given video frame. Note that this
	 * does NOT use a "smart pointer" as the smart pointer would make the frame
	 * unwriteable as it's intended to be used for filters passing frames
	 * between themselves.
	 *
	 * Since AviSynth doesn't bother implementing operator*() for PVideoFrame,
	 * you instead need to do frame.operator->() to get the pointer.
	 */
	RGB32SimpleRenderingContext(VideoFrame* frame, v8::Handle<v8::Object> self);
	~RGB32SimpleRenderingContext();
	virtual bool CanDrawImageFrom(JSVideoFrame& other);
	virtual void FillRect(UINT32 color, int x, int y, int width, int height);
	virtual void DrawImage(JSVideoFrame& otherFrame, int sx, int sy, int sw, int sh, int dx, int dy);
private:
	BYTE* frameData;
	size_t pitch;
	size_t totalSize;
};

};
