#include "GameScript.h"
#include "../ScriptFactory.h"

namespace zzz::core
{
	GameScript::GameScript()
	{
#if Z_EDITOR
		ScriptFactory::RegisterInstance(this);
#endif
	}

	GameScript::~GameScript()
	{
#if Z_EDITOR
		ScriptFactory::UnregisterInstance(this);
#endif
	}
}
