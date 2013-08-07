#include "stdafx.h"

#include "util.h"

#include <stdio.h>

namespace jsv {
	// Read a file, based on shell.cc
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