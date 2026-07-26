#include <ScriptRegistry.h>

#include "Scripts/SampleGameScript.h"
#include "Scripts/SampleSceneScript.h"
#include "Scripts/SampleViewScript.h"
#include "Scripts/SampleObjectScript.h"

extern "C"
#if defined(_WIN32)
__declspec(dllexport)
#endif
void RegisterAllScripts()
{
	zzz::script::ScriptRegistry::Register<zzz_assets_test_000::SampleGameScript>("zzz_assets_test_000::SampleGameScript");
	zzz::script::ScriptRegistry::Register<zzz_assets_test_000::SampleSceneScript>("zzz_assets_test_000::SampleSceneScript");
	zzz::script::ScriptRegistry::Register<zzz_assets_test_000::SampleViewScript>("zzz_assets_test_000::SampleViewScript");
	zzz::script::ScriptRegistry::Register<zzz_assets_test_000::SampleObjectScript>("zzz_assets_test_000::SampleObjectScript");
}
