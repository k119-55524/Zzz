#include "GlobalGameScript_000.h"
#include "GlobalGameScript_001.h"
#include "GlobalGameScript_002.h"
#include "SampleSceneScript.h"
#include "SampleObjectScript.h"
#include "SampleViewScript.h"
#include <core/Guid.h>
#include <ScriptRegistry.h>

extern "C" void RegisterAllScripts()
{
	zzz::script::ScriptRegistry::Register<GlobalGameScript_000>("GlobalGameScript_000");
	zzz::script::ScriptRegistry::Register<GlobalGameScript_001>("GlobalGameScript_001");
	zzz::script::ScriptRegistry::Register<GlobalGameScript_002>("GlobalGameScript_002");
	zzz::script::ScriptRegistry::Register<SampleSceneScript>("SampleSceneScript");
	zzz::script::ScriptRegistry::Register<SampleObjectScript>("SampleObjectScript");
	zzz::script::ScriptRegistry::Register<SampleViewScript>("SampleViewScript", zzz::common::Guid::Parse("14366409-92e5-4842-837e-c3ae45c93cf7").value_or(zzz::common::Guid{}));
}
