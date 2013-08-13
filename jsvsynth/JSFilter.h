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
#include "JSVEnvironment.h"
#include "jsutil.h"

#if 0
namespace jsv {

/**
 * JSFilter is an AviSynth filter that defers to JavaScript code for its actual
 * processing.
 *
 * FIXME: This class should really extend JSClip - or at least its JavaScript
 * object should. Right now it doesn't and likely never will, but the exposed
 * properties should - for the most part - be identical to JSClip, except
 * mutable.
 */
class JSFilter : public IClip
{
public:
	JSFilter();
	~JSFilter();
	v8::Handle<v8::Object> GetInstance();
	//
	// AviSynth API
	//
	virtual PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env);
	virtual bool __stdcall GetParity(int n);
	virtual void __stdcall GetAudio(void* buf, __int64 start, __int64 count, IScriptEnvironment* env) { /* Does nothing for now */ }
	virtual void __stdcall SetCacheHints(int cachehints,int frame_range) { /* Ignored for now */ }
	virtual const VideoInfo& __stdcall GetVideoInfo() { return vi; }
	//
	// JSVSynth methods
	//
	static v8::Handle<v8::FunctionTemplate> CreateFilterConstructor(v8::Isolate* isolate);
private:
	static void FilterConstructor(const v8::FunctionCallbackInfo<v8::Value>&);
	JS_PROPERTY_DECL(Width);
	JS_PROPERTY_DECL(Height);
	JS_PROPERTY_DECL(FrameCount);
	JS_PROPERTY_DECL(FrameRate);
	JS_PROPERTY_DECL(FrameRateNumerator);
	JS_PROPERTY_DECL(FrameRateDenominator);
	//JS_PROPERTY_DECL(AudioRate);
	//JS_PROPERTY_DECL(AudioLength);
	//JS_PROPERTY_DECL(AudioChannels);
	//JS_PROPERTY_DECL(AudioBits);
	//JS_PROPERTY_DECL(IsAudioFloat);
	//JS_PROPERTY_DECL(IsAudioInt);
	JS_PROPERTY_GETTER(IsPlanar);
	JS_PROPERTY_GETTER(IsRGB);
	JS_PROPERTY_GETTER(IsRGB24);
	JS_PROPERTY_GETTER(IsRGB32);
	JS_PROPERTY_GETTER(IsYUV);
	JS_PROPERTY_GETTER(IsYUY2);
	JS_PROPERTY_GETTER(IsYV12);
	JS_PROPERTY_GETTER(IsFieldBased);
	JS_PROPERTY_GETTER(IsFrameBased);
	JS_PROPERTY_GETTER(IsInterleaved);
	//JS_PROPERTY_DECL(HasAudio);
	JS_PROPERTY_GETTER(HasVideo);
	// And our invented propeties:
	JS_PROPERTY_GETTER(ColorSpace);
	JS_PROPERTY_GETTER(FrameRatio);
	JSVEnvironment* scriptingEnvironment;
	v8::Persistent<v8::Object> instance;
	v8::Persistent<v8::Function> function;
	VideoInfo vi;
	bool madeWeak;
};

};
#endif

