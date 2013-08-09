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

#include "avisynth.h"
#include <v8.h>
#include "jsutil.h"

namespace jsv {

/**
 * Class that handles wrapping an AviSynth Clip with a JavaScript object that
 * V8 can use.
 */
class JSClip {
public:
	JSClip(PClip, v8::Handle<v8::ObjectTemplate>);
	~JSClip();
	v8::Handle<v8::Object> GetObject(v8::Isolate* isolate);
	static v8::Handle<v8::ObjectTemplate> CreateObjectTemplate(v8::Handle<v8::Context> context);
	static bool IsWrappedClip(v8::Handle<v8::Object>);
	static PClip UnwrapClip(v8::Handle<v8::Object>);
private:
	static void ClipConstructor(const v8::FunctionCallbackInfo<v8::Value>&);
	static void ToString(const v8::FunctionCallbackInfo<v8::Value>&);
	v8::Persistent<v8::Object> jsSelf;

	JS_PROPERTY_GETTER(width);
	JS_PROPERTY_GETTER(height);
	JS_PROPERTY_GETTER(frameCount);
	JS_PROPERTY_GETTER(frameRate);
	JS_PROPERTY_GETTER(frameRateNumerator);
	JS_PROPERTY_GETTER(frameRateDenominator);
	JS_PROPERTY_GETTER(audioRate);
	JS_PROPERTY_GETTER(audioLength);
	//JS_PROPERTY_GETTER(audioLengthF);
	JS_PROPERTY_GETTER(audioChannels);
	JS_PROPERTY_GETTER(audioBits);
	JS_PROPERTY_GETTER(isAudioFloat);
	JS_PROPERTY_GETTER(isAudioInt);
	JS_PROPERTY_GETTER(isPlanar);
	JS_PROPERTY_GETTER(isRGB);
	JS_PROPERTY_GETTER(isRGB24);
	JS_PROPERTY_GETTER(isRGB32);
	JS_PROPERTY_GETTER(isYUV);
	JS_PROPERTY_GETTER(isYUY2);
	JS_PROPERTY_GETTER(isYV12);
	JS_PROPERTY_GETTER(isFieldBased);
	JS_PROPERTY_GETTER(isFrameBased);
	JS_PROPERTY_GETTER(isInterleaved);
	JS_PROPERTY_GETTER(hasAudio);
	JS_PROPERTY_GETTER(hasVideo);
	// And our invented propeties:
	JS_PROPERTY_GETTER(colorSpace);
	JS_PROPERTY_GETTER(frameRatio);
	static void JSClip::JSFrameRatioGetter(v8::Local<v8::String>, const v8::PropertyCallbackInfo<v8::Value>&);
private:
	PClip clip;
};

};