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

// This file is used to run a simple benchmark.

#include "stdafx.h"
#include <Psapi.h>
#include "avisynth_clipinfo.h"

// Frequency of our clock (see QueryPerformanceFrequency)
// This is pulled during main and then used by formatTime to format actual
// time values.
LARGE_INTEGER clockFrequency;

TCHAR* formatTime(LARGE_INTEGER start, LARGE_INTEGER end) {
	// For the sake of this function, we just keep a small, pre-allocated
	// buffer around to dump the characters into.
	static TCHAR formatted[256];
	__int64 totalTime = end.QuadPart - start.QuadPart;
	// Calculate the total number of milliseconds
	__int64 millis = totalTime / (clockFrequency.QuadPart / 1000);
	__int64 seconds = millis / 1000;
	millis -= (seconds * 1000);
	if (seconds < 60) {
		// Format immediately.
		swprintf_s(formatted, L"%I64d.%03I64d", seconds, millis);
		return formatted;
	}
	// Split out minutes.
	__int64 minutes = seconds / 60;
	seconds -= minutes * 60;
	if (minutes < 60) {
		// Format immediately.
		swprintf_s(formatted, L"%I64d:%02I64d.%03I64d", minutes, seconds, millis);
		return formatted;
	}
	__int64 hours = minutes / 60;
	minutes -= hours * 60;
	swprintf_s(formatted, L"%I64d:%02I64d:%02I64d.%03I64d", hours, minutes, seconds, millis);
	return formatted;
}

bool benchmarkFile(const LPWSTR filename) {
	HRESULT hr;
	PAVIFILE pAvi;
	AVIFILEINFO aviInfo;
	IAvisynthClipInfo *aviSynthInfo;
	bool result = true;

	wprintf(L"Loading %s...\n", filename);
	hr = AVIFileOpen(&pAvi, filename, OF_SHARE_DENY_WRITE, NULL);
	if (hr != 0) {
		fwprintf(stderr, L"Failed to open %s\n", filename);
		return false;
	}
	wprintf(L"OK\n");
	// FIXME: Should set a processor afinity to make sure times are read off
	// the same processor.
	// Grab the current time we started trying to read everything NOW so we
	// include start-up and spin-down times.
	LARGE_INTEGER totalStart;
	QueryPerformanceCounter(&totalStart);
	// Then open the file...
	wprintf(L"Getting file info...\n");
	hr = AVIFileInfo(pAvi, &aviInfo, sizeof(aviInfo));
	if (hr != 0) {
		fwprintf(stderr, L"Failed to get file info for %s\n", filename);
		AVIFileRelease(pAvi);
		return false;
	}
	// AviSynth doesn't bother running the script until you get the file info.
	// Did you know AVIFile is a COM object? I didn't, and the Microsoft API
	// documentation didn't mention that fact.
	if (wcscmp(aviInfo.szFileType, L"Avisynth") == 0) {
		hr = pAvi->QueryInterface(IID_IAvisynthClipInfo, (void**)&aviSynthInfo);
		if (hr != 0) {
			fwprintf(stderr, L"Unable to retrieve AviSynth information (can't check error info)\n");
			AVIFileRelease(pAvi);
			return false;
		}
		char const* errorMessage;
		if (aviSynthInfo->GetError(&errorMessage)) {
			fwprintf(stderr, L"Unable to open AviSynth file: script returned an error: ");
			fputs(errorMessage, stderr);
			fwprintf(stderr, L"\n");
			AVIFileRelease(pAvi);
			return false;
		}
	}
	wprintf(L"Opened %s (%d streams)\n", filename, aviInfo.dwStreams);
	wprintf(L"File type: %s\n", aviInfo.szFileType);
	for (DWORD stream = 0; stream < aviInfo.dwStreams; stream++) {
		LARGE_INTEGER start;
		PAVISTREAM pStream;
		AVISTREAMINFO streamInfo;
		QueryPerformanceCounter(&start);
		hr = AVIFileGetStream(pAvi, &pStream, 0, stream);
		if (hr != 0) {
			fwprintf(stderr, L"Unable to read stream %ld!\n", stream);
			result = false;
			continue;
		}
		AVIStreamInfo(pStream, &streamInfo, sizeof(streamInfo));
		if (streamInfo.fccType == streamtypeVIDEO) {
			BITMAPINFOHEADER formatInfo;
			LONG streamFormatSize;
			LPVOID frameData;
			wprintf(L"Stream %ld: video stream \"%s\": reading through entire stream...\n", stream, streamInfo.szName);
			AVIStreamFormatSize(pStream, 0, &streamFormatSize);
			if (streamFormatSize > sizeof(formatInfo)) {
				wprintf(L"Skipping stream: format information is too large to store in BITMAPINFOHEADER.\n");
				result = false;
				continue;
			}
			streamFormatSize = sizeof(formatInfo);
			AVIStreamReadFormat(pStream, 0, &formatInfo, &streamFormatSize);
			frameData = malloc(formatInfo.biSizeImage);
			LONG endPos = streamInfo.dwStart + streamInfo.dwLength - 1;
			for (LONG pos = streamInfo.dwStart; pos <= endPos; pos++) {
				wprintf(L"\rFrame %ld/%ld...", pos, endPos);
				hr = AVIStreamRead(pStream, pos, 1, frameData, formatInfo.biSizeImage, NULL, NULL);
				if (frameData == NULL || hr != 0) {
					result = false;
					wprintf(L"\rRead frame %ld failed!\n", pos);
				}
			}
			free(frameData);
			wprintf(L"\n");
		} else if (streamInfo.fccType == streamtypeAUDIO) {
			wprintf(L"Skipping stream %ld for now - audio stream\n", stream);
			// FIXME: Really, we should try and do this
		} else {
			wprintf(L"Skipping stream %ld (not audio or video)\n", stream);
		}
		AVIStreamRelease(pStream);
		LARGE_INTEGER end;
		QueryPerformanceCounter(&end);
		wprintf(L"Read through stream %ld: %s\n", stream, formatTime(start, end));
	}
	AVIFileRelease(pAvi);
	LARGE_INTEGER totalEnd;
	QueryPerformanceCounter(&totalEnd);
	wprintf(L"Entire process: %s\n", formatTime(totalStart, totalEnd));
	return result;
}

void usage(FILE* fp, _TCHAR* name=NULL) {
	if (name == NULL) {
		name = L"benchmark";
	}
	fwprintf(fp, L"usage: %s [OPTIONS] <INPUT>\n", name);
}

void showHelp(FILE *fp, _TCHAR* name=NULL) {
	usage(fp, name);
	//             --------10--------20--------30--------40--------50--------60--------70--------80
	fwprintf(fp, L"Run a benchmark test on the given input AVI/AVS, reading through all the\n"
				 L"available frames as fast as possible.\n\n"
				 L"Options:\n"
				 L"    /M, /MEMORY   Show memory statistics prior to and after releasing the\n"
				 L"                  AVIFile.\n"
				 L"    /T, /THREADS  After loading the file in a main thread, run through the\n"
				 L"                  frames in a background thread.\n"
				 L"    /?, /HELP     Show this help.\n");
}

#define KILOBYTE	(1024)
#define MEGABYTE	(KILOBYTE*KILOBYTE)
#define GIGABYTE	(KILOBYTE*MEGABYTE)

/**
 * Formats memory for display. (This uses a static 8-character buffer, so
 * multiple calls will overwrite the result.)
 */
wchar_t* formatMemory(size_t size) {
	// Maximum length of a result is "1023 bytes" - 11 characters (including null)
	static wchar_t result[11];
	wchar_t suffix = '\0';
	if (size >= GIGABYTE) {
		// Convert to gigabytes
		swprintf_s(result, L"%d.%03d", size / GIGABYTE, (size % GIGABYTE) / MEGABYTE);
		suffix = 'G';
	} else if (size >= MEGABYTE) {
		// Convert to megabytes
		swprintf_s(result, L"%d.%03d", size / MEGABYTE, (size % MEGABYTE) / KILOBYTE);
		suffix = 'M';
	} else if (size >= KILOBYTE) {
		// Convert to kilobytes
		swprintf_s(result, L"%d.%03d", size / KILOBYTE, size % KILOBYTE);
		suffix = 'K';
	} else {
		swprintf_s(result, L"%d bytes", size);
		// Never do any special processing, so just return immediately.
		return result;
	}
	size_t len = wcsnlen(result, 8);
	if (len > 5) {
		// Up to four digits
		len = 5;
	}
	result[len] = ' ';
	result[len+1] = suffix;
	result[len+2] = 'B';
	result[len+3] = '\0';
	return result;
}

void showMemoryUsage() {
	PROCESS_MEMORY_COUNTERS pmc;
	GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc));
	wprintf(L"              Page Fault Count: %d\n", pmc.PageFaultCount);
	wprintf(L"         Peak Working Set Size: %s\n", formatMemory(pmc.PeakWorkingSetSize));
	wprintf(L"              Working Set Size: %s\n", formatMemory(pmc.WorkingSetSize));
	wprintf(L"   Quota Peak Paged Pool Usage: %s\n", formatMemory(pmc.QuotaPeakPagedPoolUsage));
	wprintf(L"        Quota Paged Pool Usage: %s\n", formatMemory(pmc.QuotaPagedPoolUsage));
	wprintf(L"Quota Peak Nonpaged Pool Usage: %s\n", formatMemory(pmc.QuotaPeakNonPagedPoolUsage));
	wprintf(L"     Quota Nonpaged Pool Usage: %s\n", formatMemory(pmc.QuotaNonPagedPoolUsage));
	wprintf(L"                Pagefile Usage: %s\n", formatMemory(pmc.PagefileUsage));
	wprintf(L"           Peak Pagefile Usage: %s\n", formatMemory(pmc.PeakPagefileUsage));
	// Normally we'd have to close the handle, but the current process handle is special
}

int _tmain(int argc, _TCHAR* argv[]) {
	BooleanOption showMemoryStats;
	BooleanOption multithreaded;
	BooleanOption help;
	OptionParser parser;
	parser.AddOption(showMemoryStats, L"memory", L'm');
	parser.AddOption(multithreaded, L"threads", L't');
	parser.AddOption(help, L"help", L'h');
	parser.AddOption(help, L'?');
	if (!parser.Parse(argc, (wchar_t**) argv)) {
		if (help.IsSet()) {
			showHelp(stdout, argv[0]);
		} else {
			usage(stderr, argv[0]);
		}
		return 1;
	}
	if (help.IsSet()) {
		showHelp(stdout, argv[0]);
		return 0;
	}
	// Grab the arguments.
	std::vector<std::wstring> arguments = parser.GetRemainingArguments();
	if (arguments.empty()) {
		fwprintf(stderr, L"Missing input file\n");
		usage(stderr, argc > 0 ? argv[0] : NULL);
		return 1;
	}
	// Check to see if we can even do anything.
	BOOL hasClock = QueryPerformanceFrequency(&clockFrequency);
	if ((!hasClock) || (clockFrequency.QuadPart == 0)) {
		//                 --------10--------20--------30--------40--------50--------60--------70--------80
		fwprintf(stderr, L"Unable to get a high-resolution performance counter. Your system may not support\nhigh-resolution timers.\n");
		return 255;
	} else {
		wprintf(L"Clock frequency: %dl ticks per second\n", clockFrequency.QuadPart);
	}
	// If we're here, we have something to do.
	if (showMemoryStats.IsSet()) {
		wprintf(L"Current memory usage (prior to loading AVI):\n----------------------------------------\n");
		showMemoryUsage();
		wprintf(L"----------------------------------------\n");
	}
	AVIFileInit();
	if (showMemoryStats.IsSet()) {
		wprintf(L"Post-load memory usage:\n----------------------------------------\n");
		showMemoryUsage();
		wprintf(L"----------------------------------------\n");
	}
	// Open up our input file
	bool result = benchmarkFile((LPWSTR) arguments[0].c_str());
	if (showMemoryStats.IsSet()) {
		wprintf(L"Post-benchmark memory usage:\n----------------------------------------\n");
		showMemoryUsage();
		wprintf(L"----------------------------------------\n");
	}
	AVIFileExit();
	if (showMemoryStats.IsSet()) {
		wprintf(L"After closing AVI memory usage:\n----------------------------------------\n");
		showMemoryUsage();
		wprintf(L"----------------------------------------\n");
	}
	return result ? 0 : 2;
}

