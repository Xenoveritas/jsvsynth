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

class JSVEnvironment;

class WrappedFunction {
public:
	WrappedFunction(JSVEnvironment* env, const char* name);
	~WrappedFunction();
	v8::Handle<v8::Object> NewInstance(v8::Handle<v8::ObjectTemplate>);
	static v8::Handle<v8::ObjectTemplate> CreateObjectTemplate();
private:
	static WrappedFunction* UnwrapSelf(v8::Handle<v8::Object>);
	static void DestroySelf(v8::Isolate* isolate, v8::Persistent<v8::Object>* self, WrappedFunction* c);
	static void InvokeFunction(const v8::FunctionCallbackInfo<v8::Value>& args);
	v8::Persistent<v8::Object> jsSelf;
	const char* avsName;
	JSVEnvironment* jsvEnv;
};

}; // namespace jsv
