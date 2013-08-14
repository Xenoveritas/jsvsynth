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
#include "JSClip.h"
#include "jsutil.h"

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
class JSFilter : public IClip, public JSVideoInfo
{
public:
	JSFilter(v8::Handle<v8::Object> self, v8::Handle<v8::Object> aChild);
	~JSFilter();
	//
	// AviSynth API
	//
	virtual PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env);
	virtual bool __stdcall GetParity(int n);
	virtual void __stdcall GetAudio(void* buf, __int64 start, __int64 count, IScriptEnvironment* env);
	virtual void __stdcall SetCacheHints(int cachehints,int frame_range) { /* Ignored for now */ }
	virtual const VideoInfo& __stdcall GetVideoInfo() { return vi; }
	virtual const VideoInfo& GetClipVideoInfo() { return vi; }
	//
	// JSVSynth methods
	//
	virtual PClip GetClip() { return this; }
	static v8::Handle<v8::FunctionTemplate> CreateFilterConstructor(v8::Isolate* isolate);
private:
	static void FilterConstructor(const v8::FunctionCallbackInfo<v8::Value>&);
	JSVEnvironment* scriptingEnvironment;
	VideoInfo vi;
	PClip child;
	bool madeWeak;
};

};
