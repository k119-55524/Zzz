
// RegisterAllScripts.cpp — автогенерация регистрации C++ скриптов
#include "SampleGameScript.h"
#include "SampleSceneScript.h"
#include "SampleObjectScript.h"
#include "SampleViewScript.h"
#include <core/userscripts/ScriptRegistry.h>

extern "C" void RegisterAllScripts()
{
	zzz::script::ScriptRegistry::Register<SampleGameScript>("SampleGameScript");
	zzz::script::ScriptRegistry::Register<SampleSceneScript>("SampleSceneScript");
	zzz::script::ScriptRegistry::Register<SampleObjectScript>("SampleObjectScript");
	zzz::script::ScriptRegistry::Register<SampleViewScript>("SampleViewScript");
}
