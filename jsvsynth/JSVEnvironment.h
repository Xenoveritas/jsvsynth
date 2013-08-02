#include "avisynth.h"
#include <v8.h>

namespace jsv
{
/**
 * Convert the given value in the existing handle scope to an AviSynth value.
 */
AVSValue Convert(v8::Handle<v8::Value> value);

class JSVEnvironment {
public:
	JSVEnvironment(IScriptEnvironment* env);
	~JSVEnvironment();
	AVSValue RunScript(const char* source);
private:
	v8::Isolate* isolate;
	v8::Persistent<v8::Context> scriptingContext;
	IScriptEnvironment* avisynthEnv;
};

};
