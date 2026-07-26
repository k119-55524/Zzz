#include "engine/public/core/utils/engine_export.h"
#include "Assets/Scripts/SampleGameScript.h"
#include "Assets/Scripts/SampleSceneScript.h"
#include "Assets/Scripts/SampleViewScript.h"
#include "Assets/Scripts/SampleObjectScript.h"

extern "C" ZZZ_ENGINE_API void RegisterAllScripts() {
    zzz::engine::ScriptFactory::Register<zzz::scripts::SampleGameScript>("SampleGameScript");
    zzz::engine::ScriptFactory::Register<zzz::scripts::SampleSceneScript>("SampleSceneScript");
    zzz::engine::ScriptFactory::Register<zzz::scripts::SampleViewScript>("SampleViewScript");
    zzz::engine::ScriptFactory::Register<zzz::scripts::SampleObjectScript>("SampleObjectScript");
}
