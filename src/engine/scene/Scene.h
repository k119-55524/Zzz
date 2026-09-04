#pragma once

#include "engine/EngineIncludes.h"
#include "engine/gapi/clear_config/ClearConfig.h"
#include "engine/scene/layer/ILayer.h"
#include "engine/scene/layer/Layer3D.h"
#include <vector>
#include <memory>
#include <span>

namespace zzz::engine
{
	using namespace zzz::core;

	class Scene final : public std::enable_shared_from_this<Scene>
	{
		Z_NO_COPY_MOVE(Scene);

	public:
		Scene() = delete;
		Scene(Guid guid, std::string name, const std::vector<Guid>& sceneScriptGuids, const ScriptFactory& scriptFactory, ClearConfig clearConfig = {});
		~Scene();

		[[nodiscard]] const Guid& GetGuid() const noexcept { return m_Guid; }
		[[nodiscard]] const std::string& GetName() const noexcept { return m_Name; }

		[[nodiscard]] const ClearConfig& GetClearConfig() const noexcept { return m_ClearConfig; }
		void SetClearConfig(const ClearConfig& config) noexcept { m_ClearConfig = config; }

		// --- Управление слоями сцены ---
		void AddLayer(std::unique_ptr<ILayer> layer);
		[[nodiscard]] Layer3D* GetLayer3D() const noexcept;
		[[nodiscard]] const std::vector<std::unique_ptr<ILayer>>& GetLayers() const noexcept { return m_Layers; }

		void Update(const Time& time);
		void InvokeStart();
		void InvokeDestroy();

	private:
		Guid m_Guid;
		std::string m_Name;
		ClearConfig m_ClearConfig;
		SceneEventBus m_EventBus;
		std::vector<std::shared_ptr<SceneScript>> m_Scripts;

		// Слои сцены
		std::vector<std::unique_ptr<ILayer>> m_Layers;
		Layer3D* m_Layer3D{ nullptr };
	};
}
