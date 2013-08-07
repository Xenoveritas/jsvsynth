#pragma once

#include <v8.h>

namespace jsv {
	v8::Handle<v8::String> ReadFile(const char* filename);
};
