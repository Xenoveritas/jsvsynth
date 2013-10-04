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
 * This class provides access to the information in a VideoInfo struct to
 * JavaScript. Essentially this class provides the common implementation
 * between JSClip, which wraps an AviSynth-provided clip, and
 * JSFilter, which provides a way to create a filter via JavaScript.
 */
class JSVideoInfo {
public:
	JSVideoInfo(v8::Handle<v8::ObjectTemplate> templ);
	JSVideoInfo(v8::Handle<v8::Object> inst);
	~JSVideoInfo();
	v8::Handle<v8::Object> GetInstance(v8::Isolate* isolate);
	/**
	 * Get the video info instance in whatever manner it's gotten.
	 * (Named GetClipVideoInfo() because IClip has a GetVideoInfo()
	 * and it's easier not to be ambiguous.)
	 */
	virtual const VideoInfo& GetClipVideoInfo() = 0;
	/**
	 * Get the PClip for this object.
	 */
	virtual PClip GetClip() = 0;
protected:
	/**
	 * Populate a template with the fields needed to access a PClip.
	 */
	static void PopulateTemplate(v8::Handle<v8::ObjectTemplate>);
	/**
	 * The instances of our object.
	 */
	v8::Persistent<v8::Object> instance;
private:
	void Init(v8::Handle<v8::Object> obj);
	static void ToString(const v8::FunctionCallbackInfo<v8::Value>&);
	JS_PROPERTY_GETTER(Width);
	JS_PROPERTY_GETTER(Height);
	JS_PROPERTY_GETTER(FrameCount);
	JS_PROPERTY_GETTER(FrameRate);
	JS_PROPERTY_GETTER(FrameRateNumerator);
	JS_PROPERTY_GETTER(FrameRateDenominator);
	JS_PROPERTY_GETTER(AudioRate);
	JS_PROPERTY_GETTER(AudioLength);
	//JS_PROPERTY_GETTER(audioLengthF);
	JS_PROPERTY_GETTER(AudioChannels);
	JS_PROPERTY_GETTER(AudioBits);
	JS_PROPERTY_GETTER(IsAudioFloat);
	JS_PROPERTY_GETTER(IsAudioInt);
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
	JS_PROPERTY_GETTER(HasAudio);
	JS_PROPERTY_GETTER(HasVideo);
	// And our invented propeties:
	JS_PROPERTY_GETTER(ColorSpace);
	JS_PROPERTY_GETTER(FrameRatio);
};

/**
 * This class provides JavaScript access to a clip that AviSynth can use.
 */
class JSClip : public JSVideoInfo {
public:
	JSClip(PClip, v8::Handle<v8::ObjectTemplate>);
	~JSClip();
	virtual PClip GetClip() { return clip; }
	virtual const VideoInfo& GetClipVideoInfo() { return clip->GetVideoInfo(); }
	static v8::Handle<v8::ObjectTemplate> CreateObjectTemplate(v8::Handle<v8::Context> context);
	static bool IsWrappedClip(v8::Handle<v8::Object>);
	static PClip UnwrapClip(v8::Handle<v8::Object>);
protected:
	JSClip(PClip, v8::Handle<v8::Object>);
private:
	static void ClipConstructor(const v8::FunctionCallbackInfo<v8::Value>&);
	static void GetFrame(const v8::FunctionCallbackInfo<v8::Value>&);

	static void JSClip::JSFrameRatioGetter(v8::Local<v8::String>, const v8::PropertyCallbackInfo<v8::Value>&);
private:
	PClip clip;
};

};