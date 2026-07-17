#include "ViewScript.h"
#include "../../events/EventBus.h"

namespace zzz::script
{

	ViewScript::ViewScript() : BaseScript()
	{
	}

	ViewScript::~ViewScript()
	{
		if (m_Bus)
			m_Bus->UnsubscribeAll(shared_from_this());
	}

}
