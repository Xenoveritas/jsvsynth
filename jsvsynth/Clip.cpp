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
	TRACE("Wrap clip\n");
	v8::Handle<v8::Object> obj = objTemplate->NewInstance();
	TRACE("Created instance\n");
	v8::Handle<v8::External> ext = v8::External::New(this);
	TRACE("Created external\n");
	obj->SetInternalField(0, ext);
	TRACE("Internal field set\n");
	obj->SetHiddenValue(v8::String::New(JSCLIP_HIDDEN_PROP), v8::True());
	TRACE("Hidden value set\n");
	jsSelf.Reset(v8::Isolate::GetCurrent(), obj);
	TRACE("Persistent set\n");
	jsSelf.MakeWeak<JSClip>(this, DestroySelf);
	TRACE("Clip wrapped.\n");
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
		JSClip* clip = UnwrapSelf(obj);
		return clip->clip;
	} else {
		return NULL;
	}
	
}

JSClip* JSClip::UnwrapSelf(v8::Handle<v8::Object> obj) {
	v8::Handle<v8::External> ext = v8::Handle<v8::External>::Cast(obj->GetInternalField(0));
	void* ptr = ext->Value();
	return static_cast<JSClip*>(ptr);
}

void JSClip::DestroySelf(v8::Isolate* isolate, v8::Persistent<v8::Object>* self, JSClip* c) {
	v8::HandleScope scope(isolate);
	v8::Local<v8::Object>::New(isolate, (*self))->GetInternalField(0).Clear();
	self->Dispose();
	delete c;
}
void JSClip::ClipConstructor(const v8::FunctionCallbackInfo<v8::Value>& info) {
	// Does nothing, probably will never do anything, but whatever
}

void JSClip::ToString(const v8::FunctionCallbackInfo<v8::Value>& info) {
	info.GetReturnValue().Set(v8::String::New("[AviSynth Clip]"));
}

#define JSCLIP_PROPERTY_GETTER_INT(FNAME, PNAME)	void JSClip:: FNAME ## Getter (v8::Local<v8::String> name, const v8::PropertyCallbackInfo<v8::Value>& info) { \
	JSClip* clip = UnwrapSelf(info.Holder()); \
	info.GetReturnValue().Set(v8::Int32::New(clip->clip->GetVideoInfo().PNAME)); \
}

#define JSCLIP_PROPERTY_GETTER_UINT(FNAME, PNAME)	void JSClip:: FNAME ## Getter (v8::Local<v8::String> name, const v8::PropertyCallbackInfo<v8::Value>& info) { \
	JSClip* clip = UnwrapSelf(info.Holder()); \
	info.GetReturnValue().Set(v8::Int32::NewFromUnsigned(clip->clip->GetVideoInfo().PNAME)); \
}

#define JSCLIP_PROPERTY_GETTER_BOOL(FNAME, PNAME)	void JSClip:: FNAME ## Getter (v8::Local<v8::String> name, const v8::PropertyCallbackInfo<v8::Value>& info) { \
	JSClip* clip = UnwrapSelf(info.Holder()); \
	info.GetReturnValue().Set(v8::Boolean::New(clip->clip->GetVideoInfo().PNAME())); \
}

#define JSCLIP_PROPERTY_GETTER_NOTBOOL(FNAME, PNAME)	void JSClip:: FNAME ## Getter (v8::Local<v8::String> name, const v8::PropertyCallbackInfo<v8::Value>& info) { \
	JSClip* clip = UnwrapSelf(info.Holder()); \
	info.GetReturnValue().Set(v8::Boolean::New(clip->clip->GetVideoInfo().PNAME())); \
}

void JSClip::widthGetter(v8::Local<v8::String> name, const v8::PropertyCallbackInfo<v8::Value>& info) {
	// Extract the C++ request object from the JavaScript wrapper.
	JSClip* clip = UnwrapSelf(info.Holder());

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

void JSClip::frameRateGetter(v8::Local<v8::String> name, const v8::PropertyCallbackInfo<v8::Value>& info) {
	JSClip* clip = UnwrapSelf(info.Holder());
	const VideoInfo& vi = clip->clip->GetVideoInfo();
	info.GetReturnValue().Set(v8::Number::New(((double)vi.fps_numerator)/((double)vi.fps_denominator)));
}

void JSClip::audioLengthGetter(v8::Local<v8::String> name, const v8::PropertyCallbackInfo<v8::Value>& info) {
	JSClip* clip = UnwrapSelf(info.Holder());
	const VideoInfo& vi = clip->clip->GetVideoInfo();
	info.GetReturnValue().Set(v8::Number::New((double)vi.num_frames));
}

void JSClip::audioBitsGetter(v8::Local<v8::String> name, const v8::PropertyCallbackInfo<v8::Value>& info) {
	JSClip* clip = UnwrapSelf(info.Holder());
	const VideoInfo& vi = clip->clip->GetVideoInfo();
	info.GetReturnValue().Set(v8::Int32::New(vi.BytesPerAudioSample() * 8));
}

void JSClip::isAudioFloatGetter(v8::Local<v8::String> name, const v8::PropertyCallbackInfo<v8::Value>& info) {
	JSClip* clip = UnwrapSelf(info.Holder());
	const VideoInfo& vi = clip->clip->GetVideoInfo();
	info.GetReturnValue().Set(v8::Boolean::New(vi.IsSampleType(SAMPLE_FLOAT)));
}

void JSClip::isAudioIntGetter(v8::Local<v8::String> name, const v8::PropertyCallbackInfo<v8::Value>& info) {
	JSClip* clip = UnwrapSelf(info.Holder());
	const VideoInfo& vi = clip->clip->GetVideoInfo();
	info.GetReturnValue().Set(v8::Boolean::New(!vi.IsSampleType(SAMPLE_FLOAT)));
}

// A made-up property, the color space as a string:
void JSClip::colorSpaceGetter(v8::Local<v8::String> name, const v8::PropertyCallbackInfo<v8::Value>& info) {
	// Extract the C++ request object from the JavaScript wrapper.
	JSClip* clip = UnwrapSelf(info.Holder());
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
void JSClip::frameRatioGetter(v8::Local<v8::String> name, const v8::PropertyCallbackInfo<v8::Value>& info) {
	// Extract the C++ request object from the JavaScript wrapper.
	JSClip* clip = UnwrapSelf(info.Holder());
	const VideoInfo& vi = clip->clip->GetVideoInfo();

	v8::Handle<v8::Array> ratio = v8::Array::New(2);
	ratio->Set(0, v8::Int32::New(vi.fps_numerator));
	ratio->Set(1, v8::Int32::New(vi.fps_denominator));

	info.GetReturnValue().Set(ratio);
}

#define JSCLIP_ADD_PROPERTY(NAME)	templ->SetAccessor(v8::String::New(#NAME), NAME ## Getter);

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
	templ->SetAccessor(v8::String::New("audioLengthF"), audioLengthGetter);
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
