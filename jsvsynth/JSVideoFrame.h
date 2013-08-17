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

class JSVideoFrame {
public:
	JSVideoFrame(PVideoFrame frame, const VideoInfo& vi, v8::Isolate* isolate, v8::Handle<v8::ObjectTemplate> templ);
	virtual ~JSVideoFrame();
	v8::Handle<v8::Object> GetInstance(v8::Isolate* isolate);
	virtual void Release();
	PVideoFrame GetVideoFrame() { return frame; }
	static bool IsWrappedVideoFrame(v8::Handle<v8::Object> obj);
	static JSVideoFrame* UnwrapVideoFrame(v8::Handle<v8::Object> obj);
protected:
	v8::Handle<v8::ArrayBuffer> WrapData(v8::Isolate* isolate, BYTE* data, int length);
	v8::Persistent<v8::Object> instance;
	PVideoFrame frame;
	const VideoInfo& vi;
	bool released;
private:
	bool madeWeak;
};

/**
 * JavaScript wrapper for a single VideoFrame where the data is stored in an
 * interleaved format.
 */
class JSInterleavedVideoFrame : public JSVideoFrame {
public:
	JSInterleavedVideoFrame(PVideoFrame frame, const VideoInfo& vi, v8::Isolate* isolate, v8::Handle<v8::ObjectTemplate> templ);
	~JSInterleavedVideoFrame();
	virtual void Release();
	static v8::Handle<v8::ObjectTemplate> CreateTemplate(v8::Isolate* isolate);
private:
	void PopulateInstance(v8::Handle<v8::Object> inst);
	/**
	 * Populate the internal data fields.
	 */
	void MakeWriteable(v8::Isolate* isolate);
	static void JSRelease(const v8::FunctionCallbackInfo<v8::Value>&);
	static void JSGetData(v8::Local<v8::String>, const v8::PropertyCallbackInfo<v8::Value>&);
	v8::Persistent<v8::ArrayBuffer> dataInstance;
};

/**
 * JavaScript wrapper for a single VideoFrame with the planes stored separately.
 */
class JSPlanarVideoFrame : public JSVideoFrame {
public:
	JSPlanarVideoFrame(PVideoFrame frame, const VideoInfo& vi, v8::Isolate* isolate, v8::Handle<v8::ObjectTemplate> templ);
	~JSPlanarVideoFrame();
	virtual void Release();
	static v8::Handle<v8::ObjectTemplate> CreateTemplate(v8::Isolate* isolate);
private:
	void PopulateInstance(v8::Handle<v8::Object> inst);
	void PopulatePlaneInstance(v8::Handle<v8::Object> inst, int plane);
	void MakeWriteable(v8::Isolate* isolate);
	static void JSRelease(const v8::FunctionCallbackInfo<v8::Value>&);
	static void JSGetDataY(v8::Local<v8::String>, const v8::PropertyCallbackInfo<v8::Value>&);
	static void JSGetDataU(v8::Local<v8::String>, const v8::PropertyCallbackInfo<v8::Value>&);
	static void JSGetDataV(v8::Local<v8::String>, const v8::PropertyCallbackInfo<v8::Value>&);
	v8::Persistent<v8::ArrayBuffer> dataYInstance;
	v8::Persistent<v8::ArrayBuffer> dataUInstance;
	v8::Persistent<v8::ArrayBuffer> dataVInstance;
};

};