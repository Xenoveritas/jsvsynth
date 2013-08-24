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

#include <iostream>

OptionParser::OptionParser() { }
OptionParser::~OptionParser() {
	// FIXME: Free strings?
}

void OptionParser::AddOption(Option& option, wchar_t const* longName, wchar_t shortName) {
	longOptions.insert(std::pair<std::wstring,Option&>(std::wstring(longName), option));
	shortOptions.insert(std::pair<wchar_t,Option&>(shortName, option));
}

void OptionParser::AddOption(Option& option, wchar_t const* longName) {
	longOptions.insert(std::pair<std::wstring,Option&>(std::wstring(longName), option));
}

void OptionParser::AddOption(Option& option, wchar_t shortName) {
	shortOptions.insert(std::pair<wchar_t,Option&>(shortName, option));
}

Option* OptionParser::GetShortOption(wchar_t name) {
	std::map<wchar_t,Option&>::iterator it = shortOptions.find(name);
	if (it == shortOptions.end()) {
		return NULL;
	}
	return &(it->second);
}

Option* OptionParser::GetLongOption(std::wstring name) {
	std::map<std::wstring,Option&>::iterator it = longOptions.find(name);
	if (it == longOptions.end()) {
		return NULL;
	}
	return &(it->second);
}

bool OptionParser::HandleOptionNoneGivenArgument(const std::wstring optionName, const std::wstring optionValue) {
	std::wcerr << L"Option " << optionName << L" doesn't take an argument but was given \"" << optionValue << L"\"." << std::endl;
	return false;
}

bool OptionParser::HandleOptionRequiredMissingArgument(const std::wstring optionName) {
	std::wcerr << L"Option " << optionName << L" requires an argument but didn't receive one." << std::endl;
	return false;
}

bool OptionParser::HandleUnknownOption(std::wstring unknownOption, OptionType* type) {
	std::wcerr << L"Unknown option " << unknownOption << L"." << std::endl;
	return false;
}

bool OptionParser::Parse(int argc, wchar_t const* const argv[]) {
	arguments.clear();
	// Always make sure we can hold all the arguments given. We probably won't
	// need to, but pointers are fairly small.
	arguments.reserve(argc - 1);
	bool parsedOK = true;
	int i;
	for (i = 1; i < argc; i++) {
		wchar_t const* arg = argv[i];
		// Check to see if this is an argument...
		if (arg[0] == L'/') {
			// Might be a Windows style argument.
			// NOTE: Windows-style says that a thing like:
			// DIR /O:S/W
			// Is perfectly OK and works. Which means that we have to check for that.
			// Right now ... I don't. Screw that.
			parsedOK &= HandleFlag(std::wstring(arg + 1), L':', true, &i, argc, argv);
		} else if (arg[0] == L'-') {
			if (arg[1] == L'-') {
				if (arg[2] == L'\0') {
					// Quit checking arguments
					break;
				}
				// Long option.
				parsedOK &= HandleFlag(std::wstring(arg + 2), L'=', true, &i, argc, argv);
			} else {
				// Short option, which means we go through all the characters. Only one
				// short option can require arguments, in which case, we move on to the
				// next option.
			}
		} else {
			// Plain option, add to the arguments list
			arguments.push_back(std::wstring(arg));
		}
	}
	// Simply add the rest of the arguments to the arguments list
	for (; i < argc; i++) {
		arguments.push_back(std::wstring(argv[i]));
	}
	return parsedOK;
}

bool OptionParser::HandleFlag(std::wstring flag, wchar_t valueSeparator, bool canAdvance, int* currentArg, int argc, wchar_t const* const argv[]) {
	bool hasValue = false;
	std::wstring value;
	std::wstring::size_type found = flag.find_first_of(valueSeparator);
	if (found != std::wstring::npos) {
		// Value is attached to the flag
		hasValue = true;
		value.assign(flag, found + 1, std::wstring::npos);
		flag.resize(found);
	}
	Option* opt = GetLongOption(flag);
	OptionType type = OptionTypeNone;
	if (opt == NULL) {
		HandleUnknownOption(flag, &type);
	} else {
		type = opt->GetOptionType();
	}
	switch (type) {
	case OptionTypeNone:
		if (hasValue) {
			if (!HandleOptionNoneGivenArgument(flag, value))
				return false;
		}
		break;
	case OptionTypeOptional:
		// In this case, either we have the value, in which case we immediately
		// pass it
		break;
	case OptionTypeRequired:
		if (!hasValue) {
			// If we don't have a value. I'm not sure if I should check to see
			// if the next argument is a value of a flag.
			// FIXME: Decide what correct handling is
			if ((*currentArg) + 1 < argc) {
				(*currentArg)++;
				// Grab the value...
				value.assign(argv[(*currentArg)]);
			}
		}
	}
	if (opt == NULL) {
		if (hasValue) {
			return ApplyUnknownOption(flag, value);
		} else {
			return ApplyUnknownOption(flag);
		}
	} else {
		if (hasValue) {
			return opt->ApplyOption(value);
		} else {
			return opt->ApplyOption();
		}
	}
}
