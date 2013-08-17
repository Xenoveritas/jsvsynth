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
	JSSimpleRenderingContext(void);
	~JSSimpleRenderingContext(void);
	static v8::Handle<v8::ObjectTemplate> CreateTemplate(v8::Isolate*);
	virtual void FillRect(UINT32 color, int x, int y, int width, int height) = 0;
	virtual void DrawImage(PVideoFrame otherFrame, int x, int y) = 0;
	// TODO: virtual void DrawImage(PVideoFrame otherFrame, int x, int y, int width, int height) = 0;
	// TODO: virtual void DrawImage(PVideoFrame otherFrame, int x, int y, int width, int height) = 0;
	// TODO: virtual void DrawImage(PVideoFrame otherFrame, int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh) = 0;
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
	RGB32SimpleRenderingContext(PVideoFrame frame, v8::Handle<v8::Object> self);
	~RGB32SimpleRenderingContext();
private:
	BYTE* frameData;
};

};
