#include "SceneScript.h"
#include "../ScriptFactory.h"

namespace zzz::core
{
	SceneScript::SceneScript()
	{
#if Z_EDITOR
		ScriptFactory::RegisterInstance(this);
#endif
	}

	SceneScript::~SceneScript()
	{
#if Z_EDITOR
		ScriptFactory::UnregisterInstance(this);
#endif
	}
}
