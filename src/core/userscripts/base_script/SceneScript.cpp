#include "SceneScript.h"
#include "../ScriptRegistry.h"

namespace zzz::core
{
	SceneScript::SceneScript()
	{
#if Z_EDITOR
		ScriptRegistry::RegisterInstance(this);
#endif
	}

	SceneScript::~SceneScript()
	{
#if Z_EDITOR
		ScriptRegistry::UnregisterInstance(this);
#endif
	}
}
