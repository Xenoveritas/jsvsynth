#pragma once

#ifdef _DEBUG
#include <stdio.h>

#define TRACE(fmt, ...)	fprintf(stderr, fmt, __VA_ARGS__)

#else

#define TRACE(fmt, ...)	do { } while(0)

#endif
