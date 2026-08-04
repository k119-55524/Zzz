#include "GameScript.h"
#include "../ScriptRegistry.h"

namespace zzz::script
{
	GameScript::GameScript()
	{
#if Z_EDITOR
		ScriptRegistry::RegisterInstance(this);
#endif
	}

	GameScript::~GameScript()
	{
#if Z_EDITOR
		ScriptRegistry::UnregisterInstance(this);
#endif
	}
}
