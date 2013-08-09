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

/**
 * JavaScript wrapper for a single VideoFrame.
 */
class JSVideoFrame {
public:
	JSVideoFrame(PVideoFrame frame, v8::Isolate* isolate, v8::Persistent<v8::ObjectTemplate> templ);
	~JSVideoFrame();
	static v8::Handle<v8::ObjectTemplate> CreateTemplate(v8::Isolate* isolate);
private:
	static void GetPitch(const v8::FunctionCallbackInfo<v8::Value>&);
	static void GetRowSize(const v8::FunctionCallbackInfo<v8::Value>&);
	static void GetHeight(const v8::FunctionCallbackInfo<v8::Value>&);
	static void JSGetData(v8::Local<v8::String>, const v8::PropertyCallbackInfo<v8::Value>&);
	v8::Persistent<v8::Object> instance;
	PVideoFrame frame;
};

/**
 * Provides access to the actual data behind a video frame.
 */
class JSVideoFrameData {
};

};