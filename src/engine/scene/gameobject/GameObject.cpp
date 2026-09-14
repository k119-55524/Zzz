
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
		m_Guid(guid) ,
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
		ResourceManager& resourceManager)
	{
		// 1. Инстанцирование и наполнение скриптами
		for (const auto& sGuid : data.GetScriptGuids())
		{
			auto script = scriptFactory.CreateScript(sGuid, this);
			if (script != nullptr)
			{
				AddScript(std::move(script));
			}
		}

		// 2. Наполнение и предзагрузка визуальных ресурсов (меши)
		switch (data.GetMeshType())
		{
		case GameObjectData::eMeshType::Multi:
		{
			for (const auto& smGuid : data.GetSubmeshGuids())
			{
				if (smGuid.IsValid())
				{
					auto res = resourceManager.LoadDataAsset<MeshData>(smGuid);
					if (res)
					{
						DOut("[GameObject::Initialize] Сабмеш '{}' для '{}' успешно загружен: вершин {}, треугольников {}",
							smGuid.ToString(), m_Name, res->GetVertexCount(), res->GetIndexCount() / 3);
					}
					else
					{
						DOutWarning("[GameObject::Initialize] Не удалось загрузить сабмеш '{}' для '{}': {}",
							smGuid.ToString(), m_Name, res.error());
					}
				}
			}
			break;
		}
		case GameObjectData::eMeshType::Simple:
		{
			if (data.GetMeshGuid().IsValid())
			{
				auto res = resourceManager.LoadDataAsset<MeshData>(data.GetMeshGuid());
				if (res)
				{
					DOut("[GameObject::Initialize] Меш '{}' для '{}' успешно загружен: вершин {}, треугольников {}",
						data.GetMeshGuid().ToString(), m_Name, res->GetVertexCount(), res->GetIndexCount() / 3);
				}
				else
				{
					DOutWarning("[GameObject::Initialize] Не удалось загрузить меш '{}' для '{}': {}",
						data.GetMeshGuid().ToString(), m_Name, res.error());
				}
			}
			break;
		}
		case GameObjectData::eMeshType::None:
		default:
			break;
		}
	}

	void GameObject::AddScript(std::shared_ptr<Script> script)
	{
		if (script != nullptr)
		{
			m_Scripts.push_back(std::move(script));
		}
	}
}
