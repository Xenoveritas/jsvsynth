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

#include <v8.h>
#include "avisynth.h"
#include "jsutil.h"

namespace jsv {

/**
 * The AviSynth filter portion of a clip rendered using JavaScript.
 */
class CanvasFilter : public IClip {
public:
	CanvasFilter(v8::Handle<v8::Function> callback, v8::Handle<v8::ObjectTemplate> templ, PClip aChild);
	~CanvasFilter();
	/**
	 * Create the object template for canvas contexts (needed by the constructor).
	 */
	static v8::Handle<v8::ObjectTemplate> CreateTemplate();
	//
	// AviSynth APIs
	//
	PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env);
	bool __stdcall GetParity(int n);
	void __stdcall GetAudio(void* buf, __int64 start, __int64 count, IScriptEnvironment* env);
	void __stdcall SetCacheHints(int cachehints, int frame_range);
	const VideoInfo& __stdcall GetVideoInfo() { return vi; }
private:
	static void DestroySelf(v8::Isolate* isolate, v8::Persistent<v8::Object>* self, CanvasFilter* c);
	v8::Persistent<v8::Function> javascriptCallback;
	v8::Persistent<v8::Object> canvasContext;
	/**
	 * The child clip, if any. (If set, this clip provides the base for all drawing operations.)
	 */
	PClip child;
	/**
	 * Video info for this clip.
	 */
	VideoInfo vi;
};

/**
 * Implementation of ImageData.
 */
class JSImageData {
};

/**
 * Implementation of CanvasGradient.
 */
class JSCanvasGradient {
};

class JSCanvasPattern {
};

class JSTextMetrics {
};

/**
 * The rendering context for a canvas. Note: next to none of this is
 * impemented at present.
 */
class JSCanvasRenderingContext2D {
public:
	JSCanvasRenderingContext2D(v8::Isolate*, v8::Persistent<v8::ObjectTemplate>);
	~JSCanvasRenderingContext2D();
	static v8::Handle<v8::ObjectTemplate> CreateTemplates(v8::Isolate*);
	v8::Handle<v8::Object> GetInstance(v8::Isolate*);
	//
	// CanvasRenderingContext2D Methods
	//
	void arc(float x, float y, float radius, float startAngle, float endAngle, bool anticlockwise=false);
	void arcTo(float x1, float y1, float x2, float y2, float radius);
	void beginPath();
	void bezierCurveTo(float cp1x, float cp1y, float cp2x, float cp2y, float x, float y);
	void clearRect(float x, float y, float width, float height);
	void clip();
	void closePath();
	JSImageData createImageData(float width, float height);
	JSImageData createImageData(JSImageData imagedata);
	JSCanvasGradient createLinearGradient(float x0, float y0, float x1, float y1);
	//FIXME: DOM-related CanvasPattern createPattern(HTMLElement image, DOMString repetition);
	JSCanvasGradient createRadialGradient(float x0, float y0, float r0, float x1, float y1, float r1);
	//FIXME: DOM-related void drawImage(Element image, float a1, float a2, float a3, float a4, float a5, float a6, float a7, float a8);
	//FIXME: DOM-related boolean drawCustomFocusRing(Element element);
	//FIXME: DOM-related void drawSystemFocusRing(Element element);
	void fill();
	void fillRect(float x, float y, float width, float height);
	void fillText(v8::Handle<v8::String> text, float x, float y, float maxWidth);
	JSImageData getImageData(float x, float y, float width, float height);
	// ???? sequence <unrestricted double> getLineDash()
	bool isPointInPath(float x, float y);
	bool isPointInStroke(float x, float y);
	void lineTo(float x, float y);
	JSTextMetrics measureText(v8::Handle<v8::String> text);
	void moveTo(float x, float y);
	void putImageData(JSImageData imagedata, float dx, double dy, float dirtyX, float dirtyY, float dirtyWidth, float dirtyHeight);
	void quadraticCurveTo(float cpx, float cpy, float x, float y);
	void rect(float x, float y, float width, float height);
	void restore();
	void rotate(float angle);
	void save();
	void scale(float x, float y);
	// Doesn't make sense, so we're going to ignore it
	//void scrollPathIntoView();
	//void setLineDash(sequence <unrestricted double> segments);
	void setTransform(float m11, float m12, float m21, float m22, float dx, float dy);
	void stroke();
	void strokeRect(float x, float y, float w, float h);
	void strokeText(v8::Handle<v8::String> text, float x, float y, float maxWidth);
	void transform(float m11, float m12, float m21, float m22, float dx, float dy);
	void translate(float x, float y);
	//
	// CanvasRendering2DContext Attributes
	//
	// HTMLCanvasElement canvas - not happening for obvious reasons
	PROPERTY_DECL(v8::Handle<v8::Value>, FillStyle);
	//PROPERTY_DECL(v8::Handle<v8::String>, Font);
	//PROPERTY_DECL(float, GlobalAlpha);
	//PROPERTY_DECL(v8::Handle<v8::String>, GlobalCompositeOperation);
	//PROPERTY_DECL(v8::Handle<v8::String>, LineCap);
	//PROPERTY_DECL(float, LineDashOffset);
	//PROPERTY_DECL(v8::Handle<v8::String>, LineJoin);
	//PROPERTY_DECL(float, LineWidth);
	//PROPERTY_DECL(float, MiterLimit);
	//PROPERTY_DECL(float, ShadowBlur);
	//PROPERTY_DECL(v8::Handle<v8::String>, ShadowColor);
	//PROPERTY_DECL(float, ShadowOffsetX);
	//PROPERTY_DECL(float, ShadowOffsetY);
	//PROPERTY_DECL(v8::Handle<v8::Value>, StrokeStyle);
	//PROPERTY_DECL(v8::Handle<v8::String>, TextAlign);
	//PROPERTY_DECL(v8::Handle<v8::String>, TextBaseline);
private:
	v8::Persistent<v8::Object> instance;
	JS_PROPERTY_DECL(FillStyle);
	static void JSFillRect(const v8::FunctionCallbackInfo<v8::Value>&);
};

};
