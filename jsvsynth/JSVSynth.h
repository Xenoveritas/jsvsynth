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

// Current version (note: patch should be increased by 1 every commit,
// which *really* should be automated but isn't).

#define JSVSYNTH_VERSION_MAJOR			0
#define JSVSYNTH_VERSION_MINOR			0
#define JSVSYNTH_VERSION_PATCH			1

#define _MVS_STR(x)						#x
#define _MAKE_VERSION_STRING(MAJOR, MINOR, PATCH)					_MVS_STR(MAJOR) "." _MVS_STR(MINOR) "." _MVS_STR(PATCH)
#define _MAKE_VERSION_STRING_WC(MAJOR, MINOR, PATCH, CLASSIFIER)	_MVS_STR(MAJOR) "." _MVS_STR(MINOR) "." _MVS_STR(PATCH) _MVS_STR(CLASSIFIER)

#ifdef _DEBUG
# define JSVSYNTH_VERSION_STRING			_MAKE_VERSION_STRING_WC(JSVSYNTH_VERSION_MAJOR, JSVSYNTH_VERSION_MINOR, JSVSYNTH_VERSION_PATCH, -debug)
#else
# define JSVSYNTH_VERSION_STRING			_MAKE_VERSION_STRING(JSVSYNTH_VERSION_MAJOR, JSVSYNTH_VERSION_MINOR, JSVSYNTH_VERSION_PATCH)
#endif
