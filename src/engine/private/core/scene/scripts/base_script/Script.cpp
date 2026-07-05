
#include "Script.h"
#include "../ScriptRegistry.h"

using namespace zzz::script;

Script::Script(GameObject* owner) : m_Owner(owner)
{
#if Z_EDITOR
	ScriptRegistry::RegisterInstance(this);
#endif
}

Script::~Script()
{
#if Z_EDITOR
	ScriptRegistry::UnregisterInstance(this);
#endif
}
