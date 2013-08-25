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

// This is somewhat based on the Python OptionsParser. Vaguely.

#include <tchar.h>
#include <map>
#include <vector>
#include <string>

typedef enum OptionType {
	OptionTypeNone,
	OptionTypeRequired,
	/**
	 * Value is optional. In this case, the ONLY way to pass a value is by
	 * doing either "--name=value" or "/name:value".
	 */
	OptionTypeOptional
} OptionType;

class Option {
public:
	Option(OptionType aType=OptionTypeNone) : type(aType) { }
	~Option() { }
	OptionType GetOptionType() const { return type; }
	/**
	 * Callback for "applying" the option without a value.
	 *
	 * The default implementation returns true.
	 */
	virtual bool ApplyOption() { return true; }
	/**
	 * Callback for "applying" the option with a value.
	 *
	 * The default implementation simply calls ApplyOption(), meaning that if
	 * the option never takes any values, you can simply override ApplyOption()
	 * and handle it there.
	 */
	virtual bool ApplyOption(const std::wstring value) { return ApplyOption(); }
private:
	// These are private to ensure they're immutable
	OptionType type;
};

class BooleanOption : public Option {
public:
	BooleanOption() : Option(OptionTypeNone), wasSet(false) { }
	~BooleanOption() { }
	/**
	 * This simply sets a flag indicating whether or not the option was present.
	 */
	virtual bool ApplyOption() { wasSet = true; return true; }
	bool IsSet() const { return wasSet; }
protected:
	bool wasSet;
};

class IntOption : public Option {
public:
	IntOption(int defaultValue) : Option(OptionTypeRequired), value(defaultValue) { }
	~IntOption() { }
	virtual bool ApplyOption() { return false; }
	virtual bool ApplyOption(const std::wstring optionValue);
	int GetValue() const { return value; }
protected:
	int value;
};

class OptionParser {
public:
	OptionParser();
	~OptionParser();
	void AddOption(Option& option, wchar_t const* longName, wchar_t shortName);
	void AddOption(Option& option, wchar_t const* longName);
	void AddOption(Option& option, wchar_t shortName);
	Option* GetShortOption(wchar_t name);
	Option* GetLongOption(wchar_t const* name) { return GetLongOption(std::wstring(name)); }
	Option* GetLongOption(const std::wstring name);
	/**
	 * Parse the command line. Returns true if the command line was parsed
	 * without errors, false if there was an error as reported by any of the
	 * options.
	 */
	bool Parse(int argc, wchar_t const* const argv[]);
	const std::vector<std::wstring> GetRemainingArguments() { return arguments; }
	/**
	 * Handle an option that does not take an argument being given one. The
	 * default prints an error message to std::wcerr and returns false.
	 * Return true to treat this as recoverable (the option's ApplyOption
	 * will be invoked with the value anyway).
	 */
	virtual bool HandleOptionNoneGivenArgument(const std::wstring name, const std::wstring value);
	/**
	 * Handle an option that requires an argument not being given one. The
	 * default prints an error message to std::wcerr and aborts.
	 * Return true to treat this as recoverable (the option's ApplyOption()
	 * will be invoked).
	 */
	virtual bool HandleOptionRequiredMissingArgument(const std::wstring optionName);
	/**
	 * Handle an unknown option. Return true to indicate this option is "OK"
	 * and should not cause Parse to return false. The default
	 * implementation writes an error message to std::wcerr and returns false.
	 * @param unknownOption the name of the unknown option
	 * @param type (out) a point to a value that can be used to treat the
	 * option as a valid option of that type. This can be used along with
	 * returning "true" to handle arbitrarily named options.
	 */
	virtual bool HandleUnknownOption(const std::wstring unknownOption, OptionType* type);
	/**
	 * Receive a value for an unknown option.
	 */
	virtual bool ApplyUnknownOption(const std::wstring name) { return false; }
	/**
	 * Receive a value for an unknown option.
	 */
	virtual bool ApplyUnknownOption(const std::wstring name, const std::wstring value) { return false; }
private:
	/**
	 * Do whatever's necessary to match the string with arguments.
	 * Currently this means "convert to lowercase".
	 */
	void CanonicalizeString(std::wstring&);
	wchar_t CanonicalizeCharacter(wchar_t);
	bool HandleFlag(std::wstring flag, wchar_t valueSeparator, bool canAdvance, int* currentArg, int argc, wchar_t const* const argv[]);
	std::map<std::wstring,Option&> longOptions;
	std::map<wchar_t,Option&> shortOptions;
	std::vector<std::wstring> arguments;
	std::locale loc;
};
