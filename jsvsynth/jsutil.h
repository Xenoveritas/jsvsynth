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

/*
 * Various proprocessor macros for dealing with wrapping V8 stuff
 */

/**
 * Constant for marking a property both read-only and don't delete.
 */
#define JSPROP_READONLY		(v8::PropertyAttribute)(v8::PropertyAttribute::ReadOnly | v8::PropertyAttribute::DontDelete)

/**
 * Define a string that's to be used as a property name in a template.
 * This macro mostly exists because I have a feeling v8::String::New is going
 * to turn out to be the wrong method.
 */
#define JSPROP_NAME(NAME)		v8::String::New(NAME)

/**
 * Declare a property getter for the given property.
 */
#define PROPERTY_GETTER(TYPE, NAME)	TYPE Get ## NAME ()

#define PROPERTY_SETTER(TYPE, NAME)	void Set ## NAME (TYPE)

/**
 * Declare a property getter and setter for the given property.
 * These are NOT V8-style getter/setters.
 */
#define PROPERTY_DECL(TYPE, NAME)	PROPERTY_GETTER(TYPE, NAME); \
									PROPERTY_SETTER(TYPE, NAME)

#define JS_PROPERTY_GETTER(NAME)	static void JSGet ## NAME (v8::Local<v8::String> name, const v8::PropertyCallbackInfo<v8::Value>& info)
#define JS_PROPERTY_SETTER(NAME)	static void JSSet ## NAME (v8::Local<v8::String> name, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info)
#define JS_NAMED_PROPERTY_GETTER(NAME)	JS_PROPERTY_GETTER(NAME)
#define JS_NAMED_PROPERTY_SETTER(NAME)	static void JSSet ## NAME (v8::Local<v8::String> name, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<v8::Value>& info)
/**
 * Declare a property getter and setter for the given property, as a V8
 * property accessor.
 */
#define JS_PROPERTY_DECL(NAME)		JS_PROPERTY_GETTER(NAME); \
									JS_PROPERTY_SETTER(NAME)
#define JS_NAMED_PROPERTY_DECL(NAME)	JS_NAMED_PROPERTY_GETTER(NAME); \
										JS_NAMED_PROPERTY_SETTER(NAME)

namespace jsv {
	template<class C>
	void DestroySelf(const v8::WeakCallbackData<v8::Object, C>& data) {
		TRACE_MEM();
		v8::HandleScope scope(data.GetIsolate());
		data.GetValue()->GetInternalField(0).Clear();
		delete data.GetParameter();
	}
	template<class C>
	C* UnwrapSelf(v8::Local<v8::Object> obj) {
		v8::Local<v8::External> ext = v8::Local<v8::External>::Cast(obj->GetInternalField(0));
		void* ptr = ext->Value();
		return static_cast<C*>(ptr);
	}
	template<class C>
	C* UnwrapExternal(v8::Local<v8::Value> v) {
		v8::Local<v8::External> ext = v8::Local<v8::External>::Cast(v);
		void* ptr = ext->Value();
		return static_cast<C*>(ptr);
	}
};