
#include "core/utils/Ensure.h"
#include "core/io/package/MeshData.h"
#include "core/io/package/GameObjectData.h"
#include "core/userscripts/ScriptFactory.h"
#include "engine/resources/ResourceManager.h"
#include "engine/scene/storage/NodeStorage.h"
#include "core/userscripts/base_script/Script.h"

#include "GameObject.h"

using namespace zzz::core;
using namespace zzz::math;

Z_SET_LOG_CATEGORY(zzz::core::Scene);

namespace zzz::engine
{
	GameObject::GameObject(const Guid& guid, std::string name, NodeStorage& storage, NodeHandle nodeHandle) :
		m_Guid(guid),
		m_Name(std::move(name)),
		m_NodeStorage(&storage),
		m_NodeHandle(nodeHandle)
	{
		ensure(m_Guid.IsValid(), "GameObject: передан невалидный Guid");
		ensure(!m_Name.empty(), "GameObject: передано пустое имя объекта");
		ensure(storage.IsValid(nodeHandle), "GameObject: передан невалидный NodeHandle ({})", nodeHandle);
	}

	void GameObject::Initialize(
		const GameObjectData& data,
		const ScriptFactory& scriptFactory,
		ResourceManager& resourceManager,
		std::function<void(std::expected<void, std::string>)> onReady,
		std::weak_ptr<const void> ownerToken)
	{
		ensure(onReady != nullptr, "GameObject::Initialize: onReady коллбэк не должен быть null.");

		// 1. Инстанцирование и наполнение скриптами
		for (const auto& sGuid : data.GetScriptGuids())
		{
			auto script = scriptFactory.CreateScript(sGuid, this);
			if (script != nullptr)
				AddScript(std::move(script));
		}

		// 2. Асинхронная загрузка меша
		if (!data.HasMesh())
		{
			onReady({});
			return;
		}

		resourceManager.LoadMeshAsync(data.GetMeshGuids(),
			[this, onReady = std::move(onReady)](std::expected<Guid, std::string> res)
			{
				if (!res)
				{
					onReady(std::unexpected(res.error()));
					return;
				}

				m_MeshGuid = *res;
				m_NodeStorage->SetVisible(m_NodeHandle, true);

				onReady({});
			},
			ownerToken);
	}

	void GameObject::AddScript(std::shared_ptr<Script> script)
	{
		if (script != nullptr)
		{
			m_Scripts.push_back(std::move(script));
		}
	}
}