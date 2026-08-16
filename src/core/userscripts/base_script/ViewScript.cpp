#include "ViewScript.h"
#include "core/events/EventBus.h"
#include "../ScriptFactory.h"

namespace zzz::core
{

	ViewScript::ViewScript() : BaseScript()
	{
#if Z_EDITOR
		ScriptFactory::RegisterInstance(this);
#endif
	}

	ViewScript::~ViewScript()
	{
#if Z_EDITOR
		ScriptFactory::UnregisterInstance(this);
#endif
	}

}
