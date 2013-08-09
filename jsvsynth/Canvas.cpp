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
#include "Canvas.h"

/*
 * The C++ side of the AviSynth Canvas implementation.
 */

namespace jsv {

CanvasFilter::CanvasFilter(v8::Handle<v8::Function> callback, v8::Handle<v8::ObjectTemplate> templ, PClip aChild) {
}

CanvasFilter::~CanvasFilter() {
	javascriptCallback.Dispose();
	canvasContext.Dispose();
}

v8::Handle<v8::ObjectTemplate> CanvasFilter::CreateTemplate() {
	v8::Handle<v8::ObjectTemplate> templ = v8::ObjectTemplate::New();
	return templ;
}

//
// AviSynth APIs
//

PVideoFrame __stdcall CanvasFilter::GetFrame(int n, IScriptEnvironment* env) {
	//
	return child->GetFrame(n, env);
}

bool __stdcall CanvasFilter::GetParity(int n) {
	return true; // FIXME: ????
}

void __stdcall CanvasFilter::GetAudio(void* buf, __int64 start, __int64 count, IScriptEnvironment* env) {
	if (!!child) {
		child->GetAudio(buf, start, count, env);
	}
}

void __stdcall CanvasFilter::SetCacheHints(int cachehints, int frame_range) {
	// At present, we entirely ignore this.
}

// Rendering context

JSCanvasRenderingContext2D::JSCanvasRenderingContext2D(v8::Isolate*, v8::Persistent<v8::ObjectTemplate>) {
}

JSCanvasRenderingContext2D::~JSCanvasRenderingContext2D() {
}

// Attribute getters/setter wrappers

void JSCanvasRenderingContext2D::JSGetFillStyle(v8::Local<v8::String> name, const v8::PropertyCallbackInfo<v8::Value>& info) {
	JSCanvasRenderingContext2D* self = UnwrapSelf<JSCanvasRenderingContext2D>(info.Holder());
	info.GetReturnValue().Set(self->GetFillStyle());
}

void JSCanvasRenderingContext2D::JSSetFillStyle(v8::Local<v8::String> name, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info) {
	JSCanvasRenderingContext2D* self = UnwrapSelf<JSCanvasRenderingContext2D>(info.Holder());
	info.GetReturnValue().Set(self->GetFillStyle());
}

v8::Handle<v8::ObjectTemplate> JSCanvasRenderingContext2D::CreateTemplates(v8::Isolate* isolate) {
	// assume we're already in a context handle
	v8::HandleScope scope(isolate);
	// And start populating crap
	v8::Handle<v8::ObjectTemplate> templ = v8::ObjectTemplate::New();
	// We require an internal field to point back to ourselves
	templ->SetInternalFieldCount(1);
	// And now for the long set of wrapper methods
	templ->SetAccessor(v8::String::New("fillStyle"), JSGetFillStyle, JSSetFillStyle);
	//templ->SetAccessor(v8::String::New("font"), JSVCRC_GetFont, JSVCRC_SetFont);
	//templ->SetAccessor(v8::String::New("globalAlpha"), JSVCRC_GetFont, JSVCRC_SetFont);
	//templ->SetAccessor(v8::String::New("globalCompositeOperation"), JSVCRC_GetFont, JSVCRC_SetFont);
	//templ->SetAccessor(v8::String::New("lineCap"), JSVCRC_GetFont, JSVCRC_SetFont);
	//templ->SetAccessor(v8::String::New("lineDashOffset"), JSVCRC_GetFont, JSVCRC_SetFont);
	//templ->SetAccessor(v8::String::New("lineJoin"), JSVCRC_GetFont, JSVCRC_SetFont);
	//templ->SetAccessor(v8::String::New("lineWidth"), JSVCRC_GetFont, JSVCRC_SetFont);
	//templ->SetAccessor(v8::String::New("miterLimit"), JSVCRC_GetFont, JSVCRC_SetFont);
	//templ->SetAccessor(v8::String::New("shadowBlur"), JSVCRC_GetFont, JSVCRC_SetFont);
	//templ->SetAccessor(v8::String::New("shadowColor"), JSVCRC_GetFont, JSVCRC_SetFont);
	//templ->SetAccessor(v8::String::New("shadowOffsetX"), JSVCRC_GetFont, JSVCRC_SetFont);
	//templ->SetAccessor(v8::String::New("shadowOffsetY"), JSVCRC_GetFont, JSVCRC_SetFont);
	//templ->SetAccessor(v8::String::New("strokeStyle"), JSVCRC_GetFont, JSVCRC_SetFont);
	//templ->SetAccessor(v8::String::New("textAlign"), JSVCRC_GetFont, JSVCRC_SetFont);
	//templ->SetAccessor(v8::String::New("textBaseline"), JSVCRC_GetFont, JSVCRC_SetFont);
	// Functions
	//templ->SetAccessor(v8::String::New("arc"), v8::FunctionTemplate::New(JSVCRC_Arc));
	//templ->SetAccessor(v8::String::New("arcTo"), v8::FunctionTemplate::New(JSVCRC_Arc));
	//templ->SetAccessor(v8::String::New("beginPath"), v8::FunctionTemplate::New(JSVCRC_Arc));
	//templ->SetAccessor(v8::String::New("bezierCurveTo"), v8::FunctionTemplate::New(JSVCRC_Arc));
	//templ->SetAccessor(v8::String::New("clearRect"), v8::FunctionTemplate::New(JSVCRC_Arc));
	//templ->SetAccessor(v8::String::New("clip"), v8::FunctionTemplate::New(JSVCRC_Arc));
	//templ->SetAccessor(v8::String::New("closePath"), v8::FunctionTemplate::New(JSVCRC_Arc));
	//templ->SetAccessor(v8::String::New("createImageData"), v8::FunctionTemplate::New(JSVCRC_Arc));
	//templ->SetAccessor(v8::String::New("createLinearGradient"), v8::FunctionTemplate::New(JSVCRC_Arc));
	//templ->SetAccessor(v8::String::New("createPattern"), v8::FunctionTemplate::New(JSVCRC_Arc));
	//templ->SetAccessor(v8::String::New("createRadialGradient"), v8::FunctionTemplate::New(JSVCRC_Arc));
	//templ->SetAccessor(v8::String::New("drawImage"), v8::FunctionTemplate::New(JSVCRC_Arc));
	//templ->SetAccessor(v8::String::New("drawCustomFocusRing"), v8::FunctionTemplate::New(JSVCRC_Arc));
	//templ->SetAccessor(v8::String::New("drawSystemFocusRing"), v8::FunctionTemplate::New(JSVCRC_Arc));
	//templ->SetAccessor(v8::String::New("fill"), v8::FunctionTemplate::New(JSVCRC_Fill));
	//templ->SetAccessor(v8::String::New("fillRect"), v8::FunctionTemplate::New(JSFillRect));
	//templ->SetAccessor(v8::String::New("fillText"), v8::FunctionTemplate::New(JSVCRC_Arc));
	//templ->SetAccessor(v8::String::New("getImageData"), v8::FunctionTemplate::New(JSVCRC_Arc));
	//templ->SetAccessor(v8::String::New("getLineDash"), v8::FunctionTemplate::New(JSVCRC_Arc));
	//templ->SetAccessor(v8::String::New("isPointInPath"), v8::FunctionTemplate::New(JSVCRC_Arc));
	//templ->SetAccessor(v8::String::New("isPointInStroke"), v8::FunctionTemplate::New(JSVCRC_Arc));
	//templ->SetAccessor(v8::String::New("lineTo"), v8::FunctionTemplate::New(JSVCRC_Arc));
	//templ->SetAccessor(v8::String::New("measureText"), v8::FunctionTemplate::New(JSVCRC_Arc));
	//templ->SetAccessor(v8::String::New("moveTo"), v8::FunctionTemplate::New(JSVCRC_Arc));
	//templ->SetAccessor(v8::String::New("putImageData"), v8::FunctionTemplate::New(JSVCRC_Arc));
	//templ->SetAccessor(v8::String::New("quadraticCurveTo"), v8::FunctionTemplate::New(JSVCRC_Arc));
	//templ->SetAccessor(v8::String::New("rect"), v8::FunctionTemplate::New(JSVCRC_Arc));
	//templ->SetAccessor(v8::String::New("restore"), v8::FunctionTemplate::New(JSVCRC_Arc));
	//templ->SetAccessor(v8::String::New("rotate"), v8::FunctionTemplate::New(JSVCRC_Arc));
	//templ->SetAccessor(v8::String::New("save"), v8::FunctionTemplate::New(JSVCRC_Arc));
	//templ->SetAccessor(v8::String::New("scale"), v8::FunctionTemplate::New(JSVCRC_Arc));
	// Never happening, but keep around anyway:
	//templ->SetAccessor(v8::String::New("scrollPathIntoView"), v8::FunctionTemplate::New(JSVCRC_Arc));
	//templ->SetAccessor(v8::String::New("setLineDash"), v8::FunctionTemplate::New(JSVCRC_Arc));
	//templ->SetAccessor(v8::String::New("setTransform"), v8::FunctionTemplate::New(JSVCRC_Arc));
	//templ->SetAccessor(v8::String::New("stroke"), v8::FunctionTemplate::New(JSVCRC_Arc));
	//templ->SetAccessor(v8::String::New("strokeRect"), v8::FunctionTemplate::New(JSVCRC_Arc));
	//templ->SetAccessor(v8::String::New("strokeText"), v8::FunctionTemplate::New(JSVCRC_Arc));
	//templ->SetAccessor(v8::String::New("transform"), v8::FunctionTemplate::New(JSVCRC_Arc));
	//templ->SetAccessor(v8::String::New("translate"), v8::FunctionTemplate::New(JSVCRC_Arc));
	return scope.Close(templ);
}

v8::Handle<v8::Object> JSCanvasRenderingContext2D::GetInstance(v8::Isolate* isolate) {
	return v8::Local<v8::Object>::New(isolate, instance);
}

v8::Handle<v8::Value> JSCanvasRenderingContext2D::GetFillStyle() {
	return v8::Null();
}

};