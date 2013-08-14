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
#include "JSVEnvironment.h"
#include "JSClip.h"

#define JSCLIP_HIDDEN_PROP		"jsv::Clip"

namespace jsv {

JSVideoInfo::JSVideoInfo(v8::Handle<v8::ObjectTemplate> templ) {
	Init(templ->NewInstance());
}

JSVideoInfo::JSVideoInfo(v8::Handle<v8::Object> inst) {
	Init(inst);
}

void JSVideoInfo::Init(v8::Handle<v8::Object> obj) {
	TRACE("Initializing JSVideoInfo...\n");
	v8::Handle<v8::External> ext = v8::External::New(this);
	obj->SetInternalField(0, ext);
	obj->SetHiddenValue(v8::String::New(JSCLIP_HIDDEN_PROP), v8::True());
	instance.Reset(v8::Isolate::GetCurrent(), obj);
	instance.MakeWeak<JSVideoInfo>(this, DestroySelf<JSVideoInfo>);
}

JSVideoInfo::~JSVideoInfo() {
	instance.Dispose();
}

v8::Handle<v8::Object> JSVideoInfo::GetInstance(v8::Isolate* isolate) {
	return v8::Local<v8::Object>::New(isolate, instance);
}

void JSVideoInfo::PopulateTemplate(v8::Handle<v8::ObjectTemplate> templ) {
	#define JSVI_ADD_PROPERTY(JSNAME, CPPNAME)	templ->SetAccessor(v8::String::New(#JSNAME), JSGet ## CPPNAME);
	// The list of clip properties from AviSynth:
	JSVI_ADD_PROPERTY(width, Width);
	JSVI_ADD_PROPERTY(height, Height);
	JSVI_ADD_PROPERTY(frameCount, FrameCount);
	JSVI_ADD_PROPERTY(frameRate, FrameRate);
	JSVI_ADD_PROPERTY(frameRateNumerator, FrameRateNumerator);
	JSVI_ADD_PROPERTY(frameRateDenominator, FrameRateDenominator);
	JSVI_ADD_PROPERTY(audioRate, AudioRate);
	JSVI_ADD_PROPERTY(audioLength, AudioLength);
	// audioLength and audioLengthF are literally the same thing here
	templ->SetAccessor(v8::String::New("audioLengthF"), JSGetAudioLength);
	JSVI_ADD_PROPERTY(audioChannels, AudioChannels);
	JSVI_ADD_PROPERTY(audioBits, AudioBits);
	JSVI_ADD_PROPERTY(isAudioFloat, IsAudioFloat);
	JSVI_ADD_PROPERTY(isAudioInt, IsAudioInt);
	JSVI_ADD_PROPERTY(isPlanar, IsPlanar);
	JSVI_ADD_PROPERTY(isRGB, IsRGB);
	JSVI_ADD_PROPERTY(isRGB24, IsRGB24);
	JSVI_ADD_PROPERTY(isRGB32, IsRGB32);
	JSVI_ADD_PROPERTY(isYUV, IsYUV);
	JSVI_ADD_PROPERTY(isYUY2, IsYUY2);
	JSVI_ADD_PROPERTY(isYV12, IsYV12);
	JSVI_ADD_PROPERTY(isFieldBased, IsFieldBased);
	JSVI_ADD_PROPERTY(isFrameBased, IsFrameBased);
	JSVI_ADD_PROPERTY(isInterleaved, IsInterleaved);
	JSVI_ADD_PROPERTY(hasAudio, HasAudio);
	JSVI_ADD_PROPERTY(hasVideo, HasVideo);
	// And our invented propeties:
	JSVI_ADD_PROPERTY(colorSpace, ColorSpace);
	JSVI_ADD_PROPERTY(frameRatio, FrameRatio);
	v8::Handle<v8::FunctionTemplate> toString = v8::FunctionTemplate::New(ToString);
	templ->Set("toString", toString);
}

void JSVideoInfo::ToString(const v8::FunctionCallbackInfo<v8::Value>& info) {
	info.GetReturnValue().Set(v8::String::New("[AviSynth Clip]"));
}

#define JSVI_PROPERTY_GETTER_INT(FNAME, PNAME)	void JSVideoInfo::JSGet ## FNAME (v8::Local<v8::String> name, const v8::PropertyCallbackInfo<v8::Value>& info) { \
	JSVideoInfo* vi = UnwrapSelf<JSVideoInfo>(info.Holder()); \
	info.GetReturnValue().Set(v8::Int32::New(vi->GetClipVideoInfo().PNAME)); \
}

#define JSVI_PROPERTY_GETTER_UINT(FNAME, PNAME)	void JSVideoInfo::JSGet ## FNAME (v8::Local<v8::String> name, const v8::PropertyCallbackInfo<v8::Value>& info) { \
	JSVideoInfo* vi = UnwrapSelf<JSVideoInfo>(info.Holder()); \
	info.GetReturnValue().Set(v8::Int32::NewFromUnsigned(vi->GetClipVideoInfo().PNAME)); \
}

#define JSVI_PROPERTY_GETTER_BOOL(FNAME, PNAME)	void JSVideoInfo::JSGet ## FNAME (v8::Local<v8::String> name, const v8::PropertyCallbackInfo<v8::Value>& info) { \
	JSVideoInfo* vi = UnwrapSelf<JSVideoInfo>(info.Holder()); \
	info.GetReturnValue().Set(v8::Boolean::New(vi->GetClipVideoInfo().PNAME())); \
}

#define JSVI_PROPERTY_GETTER_NOTBOOL(FNAME, PNAME)	void JSVideoInfo::JSGet ## FNAME (v8::Local<v8::String> name, const v8::PropertyCallbackInfo<v8::Value>& info) { \
	JSVideoInfo* vi = UnwrapSelf<JSVideoInfo>(info.Holder()); \
	info.GetReturnValue().Set(v8::Boolean::New(vi->GetClipVideoInfo().PNAME())); \
}

JSVI_PROPERTY_GETTER_INT(Width, width);
JSVI_PROPERTY_GETTER_INT(Height, height)
JSVI_PROPERTY_GETTER_INT(FrameCount, num_frames);
JSVI_PROPERTY_GETTER_UINT(FrameRateNumerator, fps_numerator);
JSVI_PROPERTY_GETTER_UINT(FrameRateDenominator, fps_denominator);
JSVI_PROPERTY_GETTER_INT(AudioRate, audio_samples_per_second);
JSVI_PROPERTY_GETTER_INT(AudioChannels, AudioChannels());
JSVI_PROPERTY_GETTER_BOOL(IsPlanar, IsPlanar);
JSVI_PROPERTY_GETTER_BOOL(IsRGB, IsRGB);
JSVI_PROPERTY_GETTER_BOOL(IsRGB24, IsRGB24);
JSVI_PROPERTY_GETTER_BOOL(IsRGB32, IsRGB32);
JSVI_PROPERTY_GETTER_BOOL(IsYUV, IsYUV);
JSVI_PROPERTY_GETTER_BOOL(IsYUY2, IsYUY2);
JSVI_PROPERTY_GETTER_BOOL(IsYV12, IsYV12);
JSVI_PROPERTY_GETTER_BOOL(IsFieldBased, IsFieldBased);
// As far as I know, these are:
JSVI_PROPERTY_GETTER_NOTBOOL(IsFrameBased, IsFieldBased);
JSVI_PROPERTY_GETTER_NOTBOOL(IsInterleaved, IsPlanar);
JSVI_PROPERTY_GETTER_BOOL(HasAudio, HasAudio);
JSVI_PROPERTY_GETTER_BOOL(HasVideo, HasVideo);

// And the rest aren't easily macroed.

void JSVideoInfo::JSGetFrameRate(v8::Local<v8::String> name, const v8::PropertyCallbackInfo<v8::Value>& info) {
	JSVideoInfo* vinfo = UnwrapSelf<JSVideoInfo>(info.Holder());
	const VideoInfo& vi = vinfo->GetClipVideoInfo();
	info.GetReturnValue().Set(v8::Number::New(((double)vi.fps_numerator)/((double)vi.fps_denominator)));
}

void JSVideoInfo::JSGetAudioLength(v8::Local<v8::String> name, const v8::PropertyCallbackInfo<v8::Value>& info) {
	JSVideoInfo* vinfo = UnwrapSelf<JSVideoInfo>(info.Holder());
	const VideoInfo& vi = vinfo->GetClipVideoInfo();
	info.GetReturnValue().Set(v8::Number::New((double)vi.num_frames));
}

void JSVideoInfo::JSGetAudioBits(v8::Local<v8::String> name, const v8::PropertyCallbackInfo<v8::Value>& info) {
	JSVideoInfo* vinfo = UnwrapSelf<JSVideoInfo>(info.Holder());
	const VideoInfo& vi = vinfo->GetClipVideoInfo();
	info.GetReturnValue().Set(v8::Int32::New(vi.BytesPerAudioSample() * 8));
}

void JSVideoInfo::JSGetIsAudioFloat(v8::Local<v8::String> name, const v8::PropertyCallbackInfo<v8::Value>& info) {
	JSVideoInfo* vinfo = UnwrapSelf<JSVideoInfo>(info.Holder());
	const VideoInfo& vi = vinfo->GetClipVideoInfo();
	info.GetReturnValue().Set(v8::Boolean::New(vi.IsSampleType(SAMPLE_FLOAT)));
}

void JSVideoInfo::JSGetIsAudioInt(v8::Local<v8::String> name, const v8::PropertyCallbackInfo<v8::Value>& info) {
	JSVideoInfo* vinfo = UnwrapSelf<JSVideoInfo>(info.Holder());
	const VideoInfo& vi = vinfo->GetClipVideoInfo();
	info.GetReturnValue().Set(v8::Boolean::New(!vi.IsSampleType(SAMPLE_FLOAT)));
}

// A made-up property, the color space as a string:
void JSVideoInfo::JSGetColorSpace(v8::Local<v8::String> name, const v8::PropertyCallbackInfo<v8::Value>& info) {
	// Extract the C++ request object from the JavaScript wrapper.
	JSVideoInfo* vinfo = UnwrapSelf<JSVideoInfo>(info.Holder());
	const VideoInfo& vi = vinfo->GetClipVideoInfo();
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
void JSVideoInfo::JSGetFrameRatio(v8::Local<v8::String> name, const v8::PropertyCallbackInfo<v8::Value>& info) {
	// Extract the C++ request object from the JavaScript wrapper.
	JSVideoInfo* vinfo = UnwrapSelf<JSVideoInfo>(info.Holder());
	const VideoInfo& vi = vinfo->GetClipVideoInfo();

	v8::Handle<v8::Array> ratio = v8::Array::New(2);
	ratio->Set(0, v8::Int32::New(vi.fps_numerator));
	ratio->Set(1, v8::Int32::New(vi.fps_denominator));

	info.GetReturnValue().Set(ratio);
}

JSClip::JSClip(PClip aClip, v8::Handle<v8::ObjectTemplate> objTemplate) : JSVideoInfo(objTemplate), clip(aClip) {
	if (objTemplate.IsEmpty()) {
		TRACE("Object template not set!\n");
	}
}

JSClip::~JSClip() {
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
		JSVideoInfo* clip = UnwrapSelf<JSVideoInfo>(obj);
		return clip->GetClip();
	} else {
		return NULL;
	}	
}

void JSClip::ClipConstructor(const v8::FunctionCallbackInfo<v8::Value>& info) {
	// Does nothing, probably will never do anything, but whatever
}

v8::Handle<v8::ObjectTemplate> JSClip::CreateObjectTemplate(v8::Handle<v8::Context> context) {
	v8::HandleScope scope(context->GetIsolate());
	v8::Handle<v8::FunctionTemplate> constr = v8::FunctionTemplate::New(ClipConstructor);
	constr->SetClassName(v8::String::New("Clip"));
	v8::Handle<v8::ObjectTemplate> templ = constr->PrototypeTemplate();
	// We have one internal field:
	templ->SetInternalFieldCount(1);
	// Populate fields from parent
	PopulateTemplate(templ);
	templ->Set("getFrame", v8::FunctionTemplate::New(GetFrame));
	return scope.Close(templ);
}

void JSClip::GetFrame(const v8::FunctionCallbackInfo<v8::Value>& info) {
	v8::Isolate* isolate = v8::Isolate::GetCurrent();
	JSClip* self = UnwrapSelf<JSClip>(info.This());
	v8::HandleScope scope(isolate);
	if (info.Length() < 1) {
		TRACE("JSClip::GetFrame Throwing exception (not enough arguments)\n");
		v8::ThrowException(v8::Exception::Error(v8::String::New("Missing frame number")));
	}
	// Grab the first argument as an int
	int n = info[0]->ToInt32()->Int32Value();
	JSVEnvironment* jsenv = JSVEnvironment::GetCurrent();
	IScriptEnvironment* env = jsenv->GetAVSScriptEnvironment();
	PVideoFrame frame = self->clip->GetFrame(n, env);
	const VideoInfo& vi = self->clip->GetVideoInfo();
	v8::Handle<v8::Object> result = jsenv->WrapVideoFrame(frame, vi);
	if (result.IsEmpty()) {
		TRACE("WRAPPED FRAME IS EMPTY!\n");
	}
	TRACE("Wrapped up frame\n");
	info.GetReturnValue().Set(result);
	v8::String::AsciiValue res(result->ToString());
	TRACE("Returning %s...\n", res);
}

}; // namespace jsv
