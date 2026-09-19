#pragma once

#include <vector>
#include <memory>

#include "engine/scene/layer/ILayer.h"
#include "engine/gapi/clear_config/ClearConfig.h"

namespace zzz::core
{
	class ScriptFactory;
	class SceneData;
}

#include "engine/resources/ResourceTypes.h"

namespace zzz::engine
{
	using namespace zzz::core;
	class TaskDispatcher;

	class Scene final : public std::enable_shared_from_this<Scene>
	{
		Z_NO_COPY_MOVE(Scene);

	public:
		Scene(
			Guid guid,
			std::string name,
			std::shared_ptr<CoreCpuResourceManager> cpuResourceManager,
			std::shared_ptr<CoreGpuResourceManager> gpuResourceManager,
			SceneTransitionParams defaultTransition = {});

		~Scene();

		void Initialize(
			const ScriptFactory& scriptFactory,
			TaskDispatcher& taskDispatcher,
			std::function<void(std::expected<void, std::string>)> onLayersCreated);

		[[nodiscard]] const Guid& GetGuid() const noexcept { return m_Guid; }
		[[nodiscard]] const std::string& GetName() const noexcept { return m_Name; }

		[[nodiscard]] const ClearConfig& GetClearConfig() const noexcept { return m_ClearConfig; }
		void SetClearConfig(const ClearConfig& config) noexcept { m_ClearConfig = config; }

		[[nodiscard]] const SceneTransitionParams& GetTransitionParams() const noexcept { return m_TransitionParams; }
		void SetTransitionParams(const SceneTransitionParams& params) noexcept { m_TransitionParams = params; }

		[[nodiscard]] std::shared_ptr<CoreCpuResourceManager> GetCpuResourceManager() const noexcept { return m_CpuResourceManager; }
		[[nodiscard]] std::shared_ptr<CoreGpuResourceManager> GetGpuResourceManager() const noexcept { return m_GpuResourceManager; }

		// --- Управление слоями сцены ---
		[[nodiscard]] const std::vector<std::unique_ptr<ILayer>>& GetLayers() const noexcept { return m_Layers; }

		void BeginFrame();
		void Update(const Time& time);
		void InvokeStart();
		void InvokeDestroy();

	private:
		Guid m_Guid;
		std::string m_Name;
		std::shared_ptr<CoreCpuResourceManager> m_CpuResourceManager;
		std::shared_ptr<CoreGpuResourceManager> m_GpuResourceManager;
		ClearConfig m_ClearConfig;
		SceneTransitionParams m_TransitionParams;

		SceneEventBus m_EventBus;
		std::vector<std::shared_ptr<SceneScript>> m_Scripts;
		std::vector<std::unique_ptr<ILayer>> m_Layers;
	};
}
