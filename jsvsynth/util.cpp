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

#include "util.h"

#include <stdio.h>

namespace jsv {
	// Read a file, based on shell.cc
	// FIXME: Deal with character encodings
	v8::Handle<v8::String> ReadFile(const char* name) {
		FILE* file;
		errno_t err = fopen_s(&file, name, "rb");
		if (err != 0 || file == NULL) {
			// TODO: Throw an exception or something
			return v8::Handle<v8::String>();
		}

		// Find out how large the file is...
		fseek(file, 0, SEEK_END);
		int size = ftell(file);
		rewind(file);

		// Allocate space for the entire file
		char* chars = new char[size + 1];
		// Throw in a null terminator
		chars[size] = '\0';
		// Read in the file
		for (int i = 0; i < size;) {
			int read = static_cast<int>(fread(&chars[i], 1, size - i, file));
			i += read;
		}
		fclose(file);
		// And throw it into a V8 string
		v8::Handle<v8::String> result = v8::String::New(chars, size);
		delete[] chars;
		return result;
	}
};