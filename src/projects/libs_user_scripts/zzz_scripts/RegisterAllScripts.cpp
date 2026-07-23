#include <ScriptRegistry.h>

#include "SampleGameScript.h"
#include "SampleSceneScript.h"
#include "SampleViewScript.h"
#include "SampleObjectScript.h"

extern "C"
#if defined(_WIN32)
__declspec(dllexport)
#endif
void RegisterAllScripts()
{
	zzz::script::ScriptRegistry::Register<zzz_scripts::SampleGameScript>("zzz_scripts::SampleGameScript");
	zzz::script::ScriptRegistry::Register<zzz_scripts::SampleSceneScript>("zzz_scripts::SampleSceneScript");
	zzz::script::ScriptRegistry::Register<zzz_scripts::SampleViewScript>("zzz_scripts::SampleViewScript");
	zzz::script::ScriptRegistry::Register<zzz_scripts::SampleObjectScript>("zzz_scripts::SampleObjectScript");
}
