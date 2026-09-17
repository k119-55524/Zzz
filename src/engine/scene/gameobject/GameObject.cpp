
#include <mutex>

#include "core/utils/Ensure.h"
#include "core/io/package/GameObjectData.h"
#include "core/userscripts/ScriptFactory.h"
#include "core/templates/CountdownTrigger.h"
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
		const size_t pairCount = std::max(meshGuids.size(), matGuids.size());

		for (size_t i = 0; i < pairCount; ++i)
		{
			Guid mg = (i < meshGuids.size()) ? meshGuids[i] : Guid{};
			Guid matg = (i < matGuids.size()) ? matGuids[i] : (data.GetMaterialGuid().IsValid() ? data.GetMaterialGuid() : Guid{});
			if (mg.IsValid() || matg.IsValid())
			{
				m_RenderPairs.push_back(RenderPair{ mg, matg });
			}
		}

		if (m_RenderPairs.empty() && data.GetMaterialGuid().IsValid())
		{
			m_RenderPairs.push_back(RenderPair{ Guid{}, data.GetMaterialGuid() });
		}

		// 4. Подсчёт количества ресурсов для асинхронной загрузки
		size_t resourceCount = 0;
		for (const auto& pair : m_RenderPairs)
		{
			if (pair.meshGuid.IsValid()) ++resourceCount;
			if (pair.materialGuid.IsValid()) ++resourceCount;
		}

		if (resourceCount == 0)
		{
			onReady({});
			return;
		}

		// 5. Асинхронная параллельная загрузка всех ресурсов ГО с неблокирующим CountdownTrigger
		auto firstError = std::make_shared<std::string>();
		auto errorMutex = std::make_shared<std::mutex>();

		auto trigger = std::make_shared<templates::CountdownTrigger>(resourceCount, [firstError, onReady = std::move(onReady)]() mutable {
			if (!firstError->empty())
			{
				onReady(std::unexpected(*firstError));
			}
			else
			{
				onReady({});
			}
		});

		for (size_t i = 0; i < m_RenderPairs.size(); ++i)
		{
			const auto& pair = m_RenderPairs[i];
			if (pair.meshGuid.IsValid())
			{
				resourceManager.LoadMeshAsync(pair.meshGuid, [this, i, trigger, firstError, errorMutex](std::expected<Guid, std::string> res) {
					if (!res)
					{
						std::lock_guard lock(*errorMutex);
						if (firstError->empty())
						{
							*firstError = res.error();
						}
					}
					else
					{
						m_RenderPairs[i].meshGuid = *res;
					}
					trigger->CountDown();
				}, ownerToken);
			}

			if (pair.materialGuid.IsValid())
			{
				resourceManager.LoadMaterialAsync(pair.materialGuid, [this, i, trigger, firstError, errorMutex](std::expected<Guid, std::string> res) {
					if (!res)
					{
						std::lock_guard lock(*errorMutex);
						if (firstError->empty())
						{
							*firstError = res.error();
						}
					}
					else
					{
						m_RenderPairs[i].materialGuid = *res;
					}
					trigger->CountDown();
				}, ownerToken);
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