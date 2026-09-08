#include "engine/scene/layer/SceneTreeLayerBase.h"
#include "engine/scene/gameobject/GameObject.h"
#include "engine/resources/ResourceManager.h"
#include "core/io/package/LayerData.h"
#include "core/io/package/MeshData.h"
#include "core/userscripts/ScriptFactory.h"
#include "core/utils/MemoryUtils.h"
#include "core/utils/Ensure.h"
#include <unordered_map>

Z_SET_LOG_CATEGORY(::zzz::core::Scene);

namespace zzz::engine
{
	SceneTreeLayerBase::SceneTreeLayerBase(
		std::string name,
		std::shared_ptr<ResourceManager> resourceManager,
		std::unique_ptr<IDomainFactory> domainFactory,
		std::unique_ptr<ISpatialStorage> spatialStorage)
		: m_Name(std::move(name))
		, m_IsVisible{ true }
		, m_ResourceManager(std::move(resourceManager))
		, m_DomainFactory(std::move(domainFactory))
		, m_SpatialStorage(std::move(spatialStorage))
	{
		ensure(m_DomainFactory != nullptr, "DomainFactory не должен быть null в SceneTreeLayerBase.");
		ensure(m_SpatialStorage != nullptr, "SpatialStorage не должен быть null в SceneTreeLayerBase.");

		m_ObjectDomain = m_DomainFactory->CreateObjectDomain();
		m_EntityDomain = m_DomainFactory->CreateEntityDomain();
	}

	void SceneTreeLayerBase::BeginFrame()
	{
		if (!m_IsVisible)
		{
			return;
		}

		m_TreeContainer.BeginFrame();
	}

	void SceneTreeLayerBase::Update(float dt)
	{
		if (!m_IsVisible)
		{
			return;
		}

		OnUpdateDomains(dt);
		OnUpdateSpatial();
	}

	void SceneTreeLayerBase::ApplyHandoverBarrier()
	{
		if (!m_IsVisible)
		{
			return;
		}

		m_TreeContainer.ApplyHandoverBarrier();
	}

	void SceneTreeLayerBase::OnUpdateDomains(float dt)
	{
		if (m_ObjectDomain)
		{
			m_ObjectDomain->Update(dt);
		}
		if (m_EntityDomain)
		{
			m_EntityDomain->Update(dt);
		}
	}

	void SceneTreeLayerBase::OnUpdateSpatial()
	{
		m_TreeContainer.ResolveTransforms();
	}

	void SceneTreeLayerBase::Populate(const ::zzz::core::LayerData& layerData, const ::zzz::core::ScriptFactory& scriptFactory)
	{
		std::unordered_map<::zzz::core::Guid, NodeHandle> guidToHandle;

		// --- Проход 1: Создание объектов и регистрация в структурах слоя ---
		for (const auto& objData : layerData.GetObjects())
		{
			if (objData.IsEntity())
			{
				if (m_EntityDomain)
				{
					m_EntityDomain->CreateEntity(objData.GetGuid(), objData.GetName());
				}
				continue;
			}

			::zzz::GameObject* go = m_ObjectDomain ? m_ObjectDomain->CreateObject(objData.GetGuid(), objData.GetName()) : nullptr;
			if (go == nullptr)
			{
				THROW_RUNTIME("Не удалось создать GameObject '{}' в слое '{}'", objData.GetName(), m_Name);
			}

			NodeHandle handle = m_TreeContainer.CreateNode(objData.GetName(), go);
			go->BindSceneTree(&m_TreeContainer, handle);

			go->SetActive(objData.IsActive());
			go->SetMeshGuid(objData.GetMeshGuid());
			go->SetMaterialGuid(objData.GetMaterialGuid());

			go->SetLocalPosition(objData.GetPosition());
			go->SetLocalRotation(objData.GetRotation());
			go->SetLocalScale(objData.GetScale());

			for (const auto& sGuid : objData.GetScriptGuids())
			{
				auto script = scriptFactory.CreateScript(sGuid, go);
				if (script != nullptr)
				{
					go->AddScript(std::move(script));
				}
			}

			if (m_ResourceManager != nullptr && objData.GetMeshGuid() != ::zzz::core::Guid{})
			{
				auto res = m_ResourceManager->LoadDataAsset<::zzz::core::MeshData>(objData.GetMeshGuid());
				if (res)
				{
					DOut("[SceneTreeLayerBase::Populate] Меш '{}' успешно загружен: вершин {}, треугольников {}",
						objData.GetMeshGuid().ToString(), res->GetVertexCount(), res->GetIndexCount() / 3);
				}
				else
				{
					DOutWarning("[SceneTreeLayerBase::Populate] Не удалось загрузить меш '{}': {}",
						objData.GetMeshGuid().ToString(), res.error());
				}
			}

			const uint32_t spHandle = m_SpatialStorage ? m_SpatialStorage->Insert(static_cast<uint64_t>(handle.index)) : 0xFFFFFFFF;
			m_TreeContainer.SetSpatialHandle(handle, spHandle);

			guidToHandle[objData.GetGuid()] = handle;
		}

		// --- Проход 2: Связывание иерархии ---
		for (const auto& objData : layerData.GetObjects())
		{
			const auto& parentGuid = objData.GetParentGuid();
			if (parentGuid != ::zzz::core::Guid{})
			{
				auto childIt = guidToHandle.find(objData.GetGuid());
				if (childIt != guidToHandle.end())
				{
					auto parentIt = guidToHandle.find(parentGuid);
					if (parentIt != guidToHandle.end())
					{
						m_TreeContainer.SetParent(childIt->second, parentIt->second, false);
					}
					else
					{
						DOutWarning("[SceneTreeLayerBase::Populate] Родитель с GUID '{}' не найден для объекта '{}' в слое '{}'",
							parentGuid.ToString(), objData.GetName(), m_Name);
					}
				}
			}
		}
	}
}
