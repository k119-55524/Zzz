
#include <format>

#include "core/utils/Ensure.h"
#include "engine/resources/gpu/GpuMesh.h"
#include "core/io/package/GameObjectData.h"
#include "core/userscripts/ScriptFactory.h"
#include "core/templates/AsyncInitTracker.h"
#include "engine/resources/gpu/GpuMaterial.h"
#include "engine/scene/storage/NodeStorage.h"
#include "core/userscripts/base_script/Script.h"
#include "engine/resources/gpu/GpuResourceManager.h"

#include "GameObject.h"

using namespace zzz::core;
using namespace zzz::math;
using namespace zzz::templates;

Z_SET_LOG_CATEGORY(zzz::core::Scene);

namespace zzz::engine
{
	GameObject::GameObject(const Guid& guid, std::string name, NodeStorage& storage, NodeHandle nodeHandle)
		: m_Guid(guid)
		, m_Name(std::move(name))
		, m_NodeStorage(&storage)
		, m_NodeHandle(nodeHandle)
	{
		ensure(m_Guid.IsValid(), "GameObject: передан невалидный Guid");
		ensure(!m_Name.empty(), "GameObject: передано пустое имя объекта");
		ensure(storage.IsValid(nodeHandle), "GameObject: передан невалидный NodeHandle ({})", nodeHandle);
	}

	void GameObject::Initialize(
		const GameObjectData& data,
		const ScriptFactory& scriptFactory,
		GpuResourceManager& gpuResourceManager,
		std::function<void(std::expected<void, std::string>)> onReady)
	{
		ensure(onReady != nullptr, "GameObject::Initialize: onReady коллбэк не должен быть null.");

		// 1. Инстанцирование и наполнение скриптами
		for (const auto& sGuid : data.GetScriptGuids())
		{
			AddScript(scriptFactory.CreateScript(sGuid, this));
		}

		// 2. Проверка наличия геометрии: если мешей нет, ГО готов мгновенно
		if (!data.HasMesh())
		{
			onReady({});
			return;
		}

		// 3. Формирование пар рендера (меш + материал)
		m_RenderPairs.clear();
		const auto meshGuids = data.GetMeshGuids();
		const auto& matGuids = data.GetMaterialGuids();
		if (meshGuids.size() != matGuids.size())
		{
			onReady(std::unexpected(std::format(
				"GameObject '{}' ({}): количество мешей ({}) не совпадает с количеством материалов ({}).",
				m_Name, m_Guid.ToString(), meshGuids.size(), matGuids.size())));
			return;
		}

		m_RenderPairs.reserve(meshGuids.size());
		for (size_t i = 0; i < meshGuids.size(); ++i)
		{
			if (!meshGuids[i].IsValid() || !matGuids[i].IsValid())
			{
				onReady(std::unexpected(std::format(
					"GameObject '{}' ({}): пара рендера #{} содержит невалидный GUID (mesh: '{}', material: '{}').",
					m_Name, m_Guid.ToString(), i, meshGuids[i].ToString(), matGuids[i].ToString())));
				return;
			}
			m_RenderPairs.push_back(RenderPair{ meshGuids[i], matGuids[i], nullptr, nullptr });
		}

		// 4. Подсчёт количества ресурсов для асинхронной загрузки
		const size_t resourceCount = m_RenderPairs.size() * 2;
		if (resourceCount == 0)
		{
			onReady({});
			return;
		}

		// 5. Асинхронная параллельная загрузка всех ресурсов ГО с неблокирующим AsyncInitTracker
		auto tracker = std::make_shared<AsyncInitTracker>(resourceCount, std::move(onReady));

		for (size_t i = 0; i < m_RenderPairs.size(); ++i)
		{
			const auto& pair = m_RenderPairs[i];
			if (pair.meshGuid.IsValid())
			{
				gpuResourceManager.LoadAsync<GpuMesh>(pair.meshGuid, weak_from_this(), [this, i, tracker](std::expected<std::shared_ptr<GpuMesh>, std::string> res)
				{
					if (!res)
					{
						tracker->NotifyError(std::move(res.error()));
					}
					else
					{
						m_RenderPairs[i].gpuMesh = *res;
						tracker->NotifySuccess();
					}
				});
			}

			if (pair.materialGuid.IsValid())
			{
				gpuResourceManager.LoadAsync<GpuMaterial>(pair.materialGuid, weak_from_this(), [this, i, tracker](std::expected<std::shared_ptr<GpuMaterial>, std::string> res)
				{
					if (!res)
					{
						tracker->NotifyError(std::move(res.error()));
					}
					else
					{
						m_RenderPairs[i].gpuMaterial = *res;
						tracker->NotifySuccess();
					}
				});
			}
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