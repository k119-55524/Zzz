
#include "core/utils/Ensure.h"
#include "engine/scene/storage/NodeStorage.h"
#include "core/userscripts/base_script/Script.h"

#include "GameObject.h"

using namespace zzz::core;
using namespace zzz::math;

namespace zzz::engine
{
	GameObject::GameObject(const Guid& guid, std::string name, VisualPayload visual) :
		m_Guid(guid),
		m_Name(std::move(name)),
		m_NodeStorage(nullptr),
		m_NodeIndex(kInvalidNodeIndex),
		m_Visual(std::move(visual)),
		m_Scripts()
	{
	}

	void GameObject::BindNodeStorage(NodeStorage* storage, zU32 nodeIndex)
	{
		ensure(storage != nullptr, "GameObject::BindNodeStorage: указатель на NodeStorage не должен быть null");
		ensure(nodeIndex != kInvalidNodeIndex, "GameObject::BindNodeStorage: индекс ноды не должен быть kInvalidNodeIndex");
		ensure(m_NodeStorage == nullptr, "GameObject::BindNodeStorage: объект '{}' уже привязан к NodeStorage", m_Name);

		m_NodeStorage = storage;
		m_NodeIndex = nodeIndex;
	}

	void GameObject::AddScript(std::shared_ptr<Script> script)
	{
		if (script != nullptr)
		{
			m_Scripts.push_back(std::move(script));
		}
	}
}
