#include "GlobalGameScript_000.h"
#include "GlobalGameScript_001.h"
#include "GlobalGameScript_002.h"
#include "SampleSceneScript.h"
#include "SampleObjectScript.h"
#include "SampleViewScript.h"
#include <core/Core.h>
#include <ScriptRegistry.h>

extern "C" void RegisterAllScripts()
{
	zzz::core::ScriptRegistry::Register<GlobalGameScript_000>("GlobalGameScript_000", zzz::core::Guid::Parse("a8f94d12-e5b1-4c92-bf38-71e4029410ad").value_or(zzz::core::Guid{}));
	zzz::core::ScriptRegistry::Register<GlobalGameScript_001>("GlobalGameScript_001", zzz::core::Guid::Parse("b73c891e-42f0-410a-9d66-88c9a3b01e74").value_or(zzz::core::Guid{}));
	zzz::core::ScriptRegistry::Register<GlobalGameScript_002>("GlobalGameScript_002", zzz::core::Guid::Parse("f4710b65-c9e8-4632-8419-3d027f918e2c").value_or(zzz::core::Guid{}));
	zzz::core::ScriptRegistry::Register<SampleSceneScript>("SampleSceneScript");
	zzz::core::ScriptRegistry::Register<SampleObjectScript>("SampleObjectScript");
	zzz::core::ScriptRegistry::Register<SampleViewScript>("SampleViewScript", zzz::core::Guid::Parse("14366409-92e5-4842-837e-c3ae45c93cf7").value_or(zzz::core::Guid{}));
}
