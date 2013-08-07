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
	static JSClip* UnwrapSelf(v8::Handle<v8::Object>);
	static void DestroySelf(v8::Isolate* isolate, v8::Persistent<v8::Object>* self, JSClip* c);
	static void ClipConstructor(const v8::FunctionCallbackInfo<v8::Value>&);
	static void ToString(const v8::FunctionCallbackInfo<v8::Value>&);
	v8::Persistent<v8::Object> jsSelf;

#define JSCLIP_PROP_GETTER(NAME)	static void JSClip:: NAME ## Getter(v8::Local<v8::String>, const v8::PropertyCallbackInfo<v8::Value>&)

	JSCLIP_PROP_GETTER(width);
	JSCLIP_PROP_GETTER(height);
	JSCLIP_PROP_GETTER(frameCount);
	JSCLIP_PROP_GETTER(frameRate);
	JSCLIP_PROP_GETTER(frameRateNumerator);
	JSCLIP_PROP_GETTER(frameRateDenominator);
	JSCLIP_PROP_GETTER(audioRate);
	JSCLIP_PROP_GETTER(audioLength);
	//JSCLIP_PROP_GETTER(audioLengthF);
	JSCLIP_PROP_GETTER(audioChannels);
	JSCLIP_PROP_GETTER(audioBits);
	JSCLIP_PROP_GETTER(isAudioFloat);
	JSCLIP_PROP_GETTER(isAudioInt);
	JSCLIP_PROP_GETTER(isPlanar);
	JSCLIP_PROP_GETTER(isRGB);
	JSCLIP_PROP_GETTER(isRGB24);
	JSCLIP_PROP_GETTER(isRGB32);
	JSCLIP_PROP_GETTER(isYUV);
	JSCLIP_PROP_GETTER(isYUY2);
	JSCLIP_PROP_GETTER(isYV12);
	JSCLIP_PROP_GETTER(isFieldBased);
	JSCLIP_PROP_GETTER(isFrameBased);
	JSCLIP_PROP_GETTER(isInterleaved);
	JSCLIP_PROP_GETTER(hasAudio);
	JSCLIP_PROP_GETTER(hasVideo);
	// And our invented propeties:
	JSCLIP_PROP_GETTER(colorSpace);
	JSCLIP_PROP_GETTER(frameRatio);
	static void JSClip::FrameRatioGetter(v8::Local<v8::String>, const v8::PropertyCallbackInfo<v8::Value>&);
private:
	PClip clip;
};

};