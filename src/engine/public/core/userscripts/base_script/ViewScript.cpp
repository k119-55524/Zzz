#include "ViewScript.h"
#include "../../events/EventBus.h"
#include "../ScriptRegistry.h"

namespace zzz::script
{

	ViewScript::ViewScript() : BaseScript()
	{
#if Z_EDITOR
		ScriptRegistry::RegisterInstance(this);
#endif
	}

	ViewScript::~ViewScript()
	{
#if Z_EDITOR
		ScriptRegistry::UnregisterInstance(this);
#endif
	}

}
