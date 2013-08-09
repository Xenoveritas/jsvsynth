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
#include "Clip.h"

#define JSCLIP_HIDDEN_PROP		"v8::Clip"

namespace jsv {

JSClip::JSClip(PClip aClip, v8::Handle<v8::ObjectTemplate> objTemplate) : clip(aClip) {
	if (objTemplate.IsEmpty()) {
		TRACE("Object template not set!\n");
	}
	v8::Handle<v8::Object> obj = objTemplate->NewInstance();
	v8::Handle<v8::External> ext = v8::External::New(this);
	obj->SetInternalField(0, ext);
	obj->SetHiddenValue(v8::String::New(JSCLIP_HIDDEN_PROP), v8::True());
	jsSelf.Reset(v8::Isolate::GetCurrent(), obj);
	jsSelf.MakeWeak<JSClip>(this, DestroySelf<JSClip>);
}

JSClip::~JSClip() {
}

v8::Handle<v8::Object> JSClip::GetObject(v8::Isolate* isolate) {
	return v8::Local<v8::Object>::New(isolate, jsSelf);
}

bool JSClip::IsWrappedClip(v8::Handle<v8::Object> obj) {
	v8::HandleScope scope(v8::Isolate::GetCurrent());
	v8::Handle<v8::Value> value = obj->GetHiddenValue(v8::String::New(JSCLIP_HIDDEN_PROP));
	return (!value.IsEmpty()) && value->IsBoolean() && value->IsTrue();
}

PClip JSClip::UnwrapClip(v8::Handle<v8::Object> obj) {
	v8::HandleScope scope(v8::Isolate::GetCurrent());
	v8::Handle<v8::Value> value = obj->GetHiddenValue(v8::String::New(JSCLIP_HIDDEN_PROP));
	if ((!value.IsEmpty()) && value->IsBoolean() && value->IsTrue()) {
		JSClip* clip = UnwrapSelf<JSClip>(obj);
		return clip->clip;
	} else {
		return NULL;
	}	
}

void JSClip::ClipConstructor(const v8::FunctionCallbackInfo<v8::Value>& info) {
	// Does nothing, probably will never do anything, but whatever
}

void JSClip::ToString(const v8::FunctionCallbackInfo<v8::Value>& info) {
	info.GetReturnValue().Set(v8::String::New("[AviSynth Clip]"));
}

#define JSCLIP_PROPERTY_GETTER_INT(FNAME, PNAME)	void JSClip::JSGet ## FNAME (v8::Local<v8::String> name, const v8::PropertyCallbackInfo<v8::Value>& info) { \
	JSClip* clip = UnwrapSelf<JSClip>(info.Holder()); \
	info.GetReturnValue().Set(v8::Int32::New(clip->clip->GetVideoInfo().PNAME)); \
}

#define JSCLIP_PROPERTY_GETTER_UINT(FNAME, PNAME)	void JSClip::JSGet ## FNAME (v8::Local<v8::String> name, const v8::PropertyCallbackInfo<v8::Value>& info) { \
	JSClip* clip = UnwrapSelf<JSClip>(info.Holder()); \
	info.GetReturnValue().Set(v8::Int32::NewFromUnsigned(clip->clip->GetVideoInfo().PNAME)); \
}

#define JSCLIP_PROPERTY_GETTER_BOOL(FNAME, PNAME)	void JSClip::JSGet ## FNAME (v8::Local<v8::String> name, const v8::PropertyCallbackInfo<v8::Value>& info) { \
	JSClip* clip = UnwrapSelf<JSClip>(info.Holder()); \
	info.GetReturnValue().Set(v8::Boolean::New(clip->clip->GetVideoInfo().PNAME())); \
}

#define JSCLIP_PROPERTY_GETTER_NOTBOOL(FNAME, PNAME)	void JSClip::JSGet ## FNAME (v8::Local<v8::String> name, const v8::PropertyCallbackInfo<v8::Value>& info) { \
	JSClip* clip = UnwrapSelf<JSClip>(info.Holder()); \
	info.GetReturnValue().Set(v8::Boolean::New(clip->clip->GetVideoInfo().PNAME())); \
}

void JSClip::JSGetwidth(v8::Local<v8::String> name, const v8::PropertyCallbackInfo<v8::Value>& info) {
	// Extract the C++ request object from the JavaScript wrapper.
	JSClip* clip = UnwrapSelf<JSClip>(info.Holder());

	info.GetReturnValue().Set(v8::Int32::New(clip->clip->GetVideoInfo().width));
}

JSCLIP_PROPERTY_GETTER_INT(height, height)
JSCLIP_PROPERTY_GETTER_INT(frameCount, num_frames);
JSCLIP_PROPERTY_GETTER_UINT(frameRateNumerator, fps_numerator);
JSCLIP_PROPERTY_GETTER_UINT(frameRateDenominator, fps_denominator);
JSCLIP_PROPERTY_GETTER_INT(audioRate, audio_samples_per_second);
JSCLIP_PROPERTY_GETTER_INT(audioChannels, AudioChannels());
JSCLIP_PROPERTY_GETTER_BOOL(isPlanar, IsPlanar);
JSCLIP_PROPERTY_GETTER_BOOL(isRGB, IsRGB);
JSCLIP_PROPERTY_GETTER_BOOL(isRGB24, IsRGB24);
JSCLIP_PROPERTY_GETTER_BOOL(isRGB32, IsRGB32);
JSCLIP_PROPERTY_GETTER_BOOL(isYUV, IsYUV);
JSCLIP_PROPERTY_GETTER_BOOL(isYUY2, IsYUY2);
JSCLIP_PROPERTY_GETTER_BOOL(isYV12, IsYV12);
JSCLIP_PROPERTY_GETTER_BOOL(isFieldBased, IsFieldBased);
// As far as I know, these are:
JSCLIP_PROPERTY_GETTER_NOTBOOL(isFrameBased, IsFieldBased);
JSCLIP_PROPERTY_GETTER_NOTBOOL(isInterleaved, IsPlanar);
JSCLIP_PROPERTY_GETTER_BOOL(hasAudio, HasAudio);
JSCLIP_PROPERTY_GETTER_BOOL(hasVideo, HasVideo);

// And the rest aren't easily macroed.

void JSClip::JSGetframeRate(v8::Local<v8::String> name, const v8::PropertyCallbackInfo<v8::Value>& info) {
	JSClip* clip = UnwrapSelf<JSClip>(info.Holder());
	const VideoInfo& vi = clip->clip->GetVideoInfo();
	info.GetReturnValue().Set(v8::Number::New(((double)vi.fps_numerator)/((double)vi.fps_denominator)));
}

void JSClip::JSGetaudioLength(v8::Local<v8::String> name, const v8::PropertyCallbackInfo<v8::Value>& info) {
	JSClip* clip = UnwrapSelf<JSClip>(info.Holder());
	const VideoInfo& vi = clip->clip->GetVideoInfo();
	info.GetReturnValue().Set(v8::Number::New((double)vi.num_frames));
}

void JSClip::JSGetaudioBits(v8::Local<v8::String> name, const v8::PropertyCallbackInfo<v8::Value>& info) {
	JSClip* clip = UnwrapSelf<JSClip>(info.Holder());
	const VideoInfo& vi = clip->clip->GetVideoInfo();
	info.GetReturnValue().Set(v8::Int32::New(vi.BytesPerAudioSample() * 8));
}

void JSClip::JSGetisAudioFloat(v8::Local<v8::String> name, const v8::PropertyCallbackInfo<v8::Value>& info) {
	JSClip* clip = UnwrapSelf<JSClip>(info.Holder());
	const VideoInfo& vi = clip->clip->GetVideoInfo();
	info.GetReturnValue().Set(v8::Boolean::New(vi.IsSampleType(SAMPLE_FLOAT)));
}

void JSClip::JSGetisAudioInt(v8::Local<v8::String> name, const v8::PropertyCallbackInfo<v8::Value>& info) {
	JSClip* clip = UnwrapSelf<JSClip>(info.Holder());
	const VideoInfo& vi = clip->clip->GetVideoInfo();
	info.GetReturnValue().Set(v8::Boolean::New(!vi.IsSampleType(SAMPLE_FLOAT)));
}

// A made-up property, the color space as a string:
void JSClip::JSGetcolorSpace(v8::Local<v8::String> name, const v8::PropertyCallbackInfo<v8::Value>& info) {
	// Extract the C++ request object from the JavaScript wrapper.
	JSClip* clip = UnwrapSelf<JSClip>(info.Holder());
	const VideoInfo& vi = clip->clip->GetVideoInfo();
	const char* res;

	if (vi.IsYV12()) {
		res = "YV12";
	} else if (vi.IsYUY2()) {
		res = "YUY2";
	} else if (vi.IsRGB32()) {
		res = "RGB32";
	} else if (vi.IsRGB24()) {
		res = "RGB24";
	} else {
		res = "unknown";
	}

	info.GetReturnValue().Set(v8::String::New(res));
}

// A made-up property, the frame rate as a pair:
void JSClip::JSGetframeRatio(v8::Local<v8::String> name, const v8::PropertyCallbackInfo<v8::Value>& info) {
	// Extract the C++ request object from the JavaScript wrapper.
	JSClip* clip = UnwrapSelf<JSClip>(info.Holder());
	const VideoInfo& vi = clip->clip->GetVideoInfo();

	v8::Handle<v8::Array> ratio = v8::Array::New(2);
	ratio->Set(0, v8::Int32::New(vi.fps_numerator));
	ratio->Set(1, v8::Int32::New(vi.fps_denominator));

	info.GetReturnValue().Set(ratio);
}

#define JSCLIP_ADD_PROPERTY(NAME)	templ->SetAccessor(v8::String::New(#NAME), JSGet ## NAME);

v8::Handle<v8::ObjectTemplate> JSClip::CreateObjectTemplate(v8::Handle<v8::Context> context) {
	v8::HandleScope scope(context->GetIsolate());
	v8::Handle<v8::FunctionTemplate> constr = v8::FunctionTemplate::New(ClipConstructor);
	constr->SetClassName(v8::String::New("Clip"));
	v8::Handle<v8::ObjectTemplate> templ = constr->PrototypeTemplate();
	// We have one internal field:
	templ->SetInternalFieldCount(1);
	// The list of clip properties from AviSynth:
	JSCLIP_ADD_PROPERTY(width);
	JSCLIP_ADD_PROPERTY(height);
	JSCLIP_ADD_PROPERTY(frameCount);
	JSCLIP_ADD_PROPERTY(frameRate);
	JSCLIP_ADD_PROPERTY(frameRateNumerator);
	JSCLIP_ADD_PROPERTY(frameRateDenominator);
	JSCLIP_ADD_PROPERTY(audioRate);
	JSCLIP_ADD_PROPERTY(audioLength);
	// audioLength and audioLengthF are literally the same thing here
	templ->SetAccessor(v8::String::New("audioLengthF"), JSGetaudioLength);
	JSCLIP_ADD_PROPERTY(audioChannels);
	JSCLIP_ADD_PROPERTY(audioBits);
	JSCLIP_ADD_PROPERTY(isAudioFloat);
	JSCLIP_ADD_PROPERTY(isAudioInt);
	JSCLIP_ADD_PROPERTY(isPlanar);
	JSCLIP_ADD_PROPERTY(isRGB);
	JSCLIP_ADD_PROPERTY(isRGB24);
	JSCLIP_ADD_PROPERTY(isRGB32);
	JSCLIP_ADD_PROPERTY(isYUV);
	JSCLIP_ADD_PROPERTY(isYUY2);
	JSCLIP_ADD_PROPERTY(isYV12);
	JSCLIP_ADD_PROPERTY(isFieldBased);
	JSCLIP_ADD_PROPERTY(isFrameBased);
	JSCLIP_ADD_PROPERTY(isInterleaved);
	JSCLIP_ADD_PROPERTY(hasAudio);
	JSCLIP_ADD_PROPERTY(hasVideo);
	// And our invented propeties:
	JSCLIP_ADD_PROPERTY(colorSpace);
	JSCLIP_ADD_PROPERTY(frameRatio);
	v8::Handle<v8::FunctionTemplate> toString = v8::FunctionTemplate::New(ToString);
	templ->Set("toString", toString);
	return scope.Close(templ);
}

}; // namespace jsv
