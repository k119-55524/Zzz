
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

		// Инстанцирование и наполнение скриптами
		for (const auto& sGuid : data.GetScriptGuids())
		{
			AddScript(scriptFactory.CreateScript(sGuid, this));
		}

		// Если мешей нет
		if (!data.HasMesh())
		{
			onReady({});
			return;
		}

		const auto renderPairs = data.GetRenderPairs();
		m_RenderPairs.clear();
		m_RenderPairs.resize(renderPairs.size());

		// Асинхронная параллельная загрузка всех ресурсов ГО с неблокирующим AsyncInitTracker
		auto tracker = std::make_shared<AsyncInitTracker>(renderPairs.size() * 2, std::move(onReady));
		for (size_t i = 0; i < renderPairs.size(); ++i)
		{
			const auto& pair = renderPairs[i];
			ensure(pair.meshGuid.IsValid() && pair.materialGuid.IsValid(),
				"GameObject '{}' ({}): пара рендера #{} содержит невалидный GUID (mesh: '{}', material: '{}').",
				m_Name, m_Guid.ToString(), i, pair.meshGuid.ToString(), pair.materialGuid.ToString());

			gpuResourceManager.GetAsync<GpuMesh>(pair.meshGuid, weak_from_this(), [this, i, tracker](auto res)
			{
				if (!res)
					tracker->NotifyError(std::move(res.error()));
				else
				{
					m_RenderPairs[i].gpuMesh = std::move(*res);
					tracker->NotifySuccess();
				}
			});

			gpuResourceManager.GetAsync<GpuMaterial>(pair.materialGuid, weak_from_this(), [this, i, tracker](auto res)
			{
				if (!res)
					tracker->NotifyError(std::move(res.error()));
				else
				{
					m_RenderPairs[i].gpuMaterial = std::move(*res);
					tracker->NotifySuccess();
				}
			});
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