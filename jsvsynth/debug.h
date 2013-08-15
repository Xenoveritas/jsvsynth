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

#include <stdio.h>

#ifdef _DEBUG

#define TRACE(FORMAT, ...)	fprintf(stderr, FORMAT, __VA_ARGS__);fflush(stderr)

#else

#define TRACE(FORMAT, ...)	do { } while(0)

#endif

#define JSV_ERROR(FORMAT, ...)	fprintf(stderr, "ERROR (%s): " FORMAT "\n", __FUNCTION__, __VA_ARGS__);fflush(stderr)

//#define TRACE_CONVERSIONS
