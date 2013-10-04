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

class JSSimpleRenderingContext;

class JSVideoFrame {
public:
	JSVideoFrame(PVideoFrame frame, const VideoInfo& vi, v8::Isolate* isolate, v8::Handle<v8::ObjectTemplate> templ);
	virtual ~JSVideoFrame();
	v8::Handle<v8::Object> GetInstance(v8::Isolate* isolate);
	/**
	 * Release the video frame data back to AviSynth. Note: it should always be
	 * safe to call this function multiple times.
	 */
	virtual void Release();
	virtual JSSimpleRenderingContext* GetSimpleContext() = 0;
	PVideoFrame GetVideoFrame() const { return frame; }
	int GetWidth() const { return vi.width; }
	int GetHeight() const { return vi.height; }
	const VideoInfo& GetVideoInfo() const { return vi; }
	static bool IsWrappedVideoFrame(v8::Handle<v8::Object> obj);
	static JSVideoFrame* UnwrapVideoFrame(v8::Handle<v8::Object> obj);
	/**
	 * Create the various default parts of the template that are common between
	 * all video frame types.
	 */
	static void CreateTemplateDefaultFields(v8::Handle<v8::ObjectTemplate>);
protected:
	v8::Handle<v8::ArrayBuffer> WrapData(v8::Isolate* isolate, BYTE* data, int length);
	v8::Persistent<v8::Object> instance;
	PVideoFrame frame;
	const VideoInfo& vi;
	bool released;
private:
	// JS implementations of getContext and getSimpleContext
	static void JSGetContext(const v8::FunctionCallbackInfo<v8::Value>&);
	static void JSGetSimpleContext(const v8::FunctionCallbackInfo<v8::Value>&);
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
	virtual JSSimpleRenderingContext* GetSimpleContext();
	static v8::Handle<v8::ObjectTemplate> CreateTemplate(v8::Isolate* isolate);
private:
	void PopulateInstance(v8::Handle<v8::Object> inst);
	/**
	 * Populate the internal data fields. Returns true if they were populated
	 * successfully, false if they were not.
	 */
	bool MakeWriteable(v8::Isolate* isolate);
	static void JSRelease(const v8::FunctionCallbackInfo<v8::Value>&);
	static void JSGetData(v8::Local<v8::String>, const v8::PropertyCallbackInfo<v8::Value>&);
	v8::Persistent<v8::ArrayBuffer> dataInstance;
	JSSimpleRenderingContext* simpleContext;
};

/**
 * JavaScript wrapper for a single VideoFrame with the planes stored separately.
 */
class JSPlanarVideoFrame : public JSVideoFrame {
public:
	JSPlanarVideoFrame(PVideoFrame frame, const VideoInfo& vi, v8::Isolate* isolate, v8::Handle<v8::ObjectTemplate> templ);
	~JSPlanarVideoFrame();
	virtual void Release();
	virtual JSSimpleRenderingContext* GetSimpleContext() { return NULL; }
	static v8::Handle<v8::ObjectTemplate> CreateTemplate(v8::Isolate* isolate);
private:
	void PopulateInstance(v8::Handle<v8::Object> inst);
	void PopulatePlaneInstance(v8::Handle<v8::Object> inst, int plane);
	void MakeWriteable(v8::Isolate* isolate);
	static void JSRelease(const v8::FunctionCallbackInfo<v8::Value>&);
	static void JSGetContext(const v8::FunctionCallbackInfo<v8::Value>&);
	static void JSGetDataY(v8::Local<v8::String>, const v8::PropertyCallbackInfo<v8::Value>&);
	static void JSGetDataU(v8::Local<v8::String>, const v8::PropertyCallbackInfo<v8::Value>&);
	static void JSGetDataV(v8::Local<v8::String>, const v8::PropertyCallbackInfo<v8::Value>&);
	v8::Persistent<v8::ArrayBuffer> dataYInstance;
	v8::Persistent<v8::ArrayBuffer> dataUInstance;
	v8::Persistent<v8::ArrayBuffer> dataVInstance;
};

};