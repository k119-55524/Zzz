#include "GlobalGameScript_000.h"
#include "GlobalGameScript_001.h"
#include "GlobalGameScript_002.h"
#include "SampleSceneScript.h"
#include "SampleObjectScript.h"
#include "SampleViewScript.h"
#include <core/utils/Guid.h>
#include <ScriptRegistry.h>

extern "C" void RegisterAllScripts()
{
	zzz::script::ScriptRegistry::Register<GlobalGameScript_000>("GlobalGameScript_000", zzz::common::Guid::Parse("a8f94d12-e5b1-4c92-bf38-71e4029410ad").value_or(zzz::common::Guid{}));
	zzz::script::ScriptRegistry::Register<GlobalGameScript_001>("GlobalGameScript_001", zzz::common::Guid::Parse("b73c891e-42f0-410a-9d66-88c9a3b01e74").value_or(zzz::common::Guid{}));
	zzz::script::ScriptRegistry::Register<GlobalGameScript_002>("GlobalGameScript_002", zzz::common::Guid::Parse("f4710b65-c9e8-4632-8419-3d027f918e2c").value_or(zzz::common::Guid{}));
	zzz::script::ScriptRegistry::Register<SampleSceneScript>("SampleSceneScript");
	zzz::script::ScriptRegistry::Register<SampleObjectScript>("SampleObjectScript");
	zzz::script::ScriptRegistry::Register<SampleViewScript>("SampleViewScript", zzz::common::Guid::Parse("14366409-92e5-4842-837e-c3ae45c93cf7").value_or(zzz::common::Guid{}));
}
