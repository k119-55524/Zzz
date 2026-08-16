
#include "Script.h"
#include "../ScriptFactory.h"

using namespace zzz::core;

Script::Script(GameObject* owner) : m_Owner(owner)
{
#if Z_EDITOR
	ScriptFactory::RegisterInstance(this);
#endif
}

Script::~Script()
{
#if Z_EDITOR
	ScriptFactory::UnregisterInstance(this);
#endif
}
