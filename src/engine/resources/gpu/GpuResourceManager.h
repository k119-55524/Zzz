#pragma once

#include <tuple>
#include <memory>
#include <string>
#include <format>
#include <expected>
#include <functional>

#include "core/utils/Guid.h"
#include "engine/gapi/GAPI.h"
#include "engine/tasks/TaskDispatcher.h"
#include "core/templates/CallbackQueue.h"
#include "engine/resources/ResourceTable.h"
#include "engine/resources/ResourceTypes.h"
#include "engine/resources/cpu/CpuResourceManager.h"

using namespace zzz::core;

namespace zzz::engine
{
	template <typename T>
	using GpuResourceResult = std::expected<std::shared_ptr<T>, std::string>;

	template <typename T>
	using GpuResourceCallback = std::function<void(GpuResourceResult<T>)>;

	// Фабрикация GPU-ресурсов из соответствующих CPU-ресурсов
	[[nodiscard]] std::shared_ptr<GpuMesh>      CreateGpuResource(std::shared_ptr<CpuMesh> cpuMesh, GAPI* gapi);
	[[nodiscard]] std::shared_ptr<GpuMaterial>  CreateGpuResource(std::shared_ptr<CpuMaterial> cpuMaterial, GAPI* gapi);
	[[nodiscard]] std::shared_ptr<GpuTexture2D> CreateGpuResource(std::shared_ptr<CpuTexture2D> cpuTexture, GAPI* gapi);
	[[nodiscard]] std::shared_ptr<GpuShader>    CreateGpuResource(std::shared_ptr<CpuShader> cpuShader, GAPI* gapi);

	/**
	 * @class GpuResourceManager
	 * @brief Шаблонный менеджер видеопамяти и GPU-ресурсов.
	 * @details Автоматически разворачивает типизированные таблицы в std::tuple для SupportedResources...
	 *          При запросе ресурса типа TGpu автоматически запрашивает парный CPU-ресурс (typename TGpu::CpuType).
	 */
	template<typename... SupportedResources>
	class GpuResourceManager
	{
		Z_NO_COPY_MOVE(GpuResourceManager);

	public:
		GpuResourceManager() = delete;

		explicit GpuResourceManager(
			TaskDispatcher& taskDispatcher,
			std::shared_ptr<GAPI> gapi,
			std::shared_ptr<CoreCpuResourceManager> cpuResourceManager)
			: m_TaskDispatcher(taskDispatcher)
			, m_GAPI(std::move(gapi))
			, m_CpuManager(std::move(cpuResourceManager))
		{
		}

		virtual ~GpuResourceManager()
		{
			EmergencyStop();
		}

		// --- Обновление и выполнение колбэков на главном потоке ---
		void Update()
		{
			m_MainThreadQueue.ExecuteAll();
		}

		/// @brief Прямой доступ к типизированной таблице ресурса
		template<typename T>
		[[nodiscard]] ResourceTable<T>& GetTable() noexcept
		{
			return std::get<ResourceTable<T>>(m_Tables);
		}

		template<typename T>
		[[nodiscard]] const ResourceTable<T>& GetTable() const noexcept
		{
			return std::get<ResourceTable<T>>(m_Tables);
		}

		// --- Единая шаблонная функция асинхронного доступа GetAsync<T> ---
		template<typename T, typename ContextType>
		void GetAsync(
			const Guid& guid,
			std::weak_ptr<ContextType> context,
			GpuResourceCallback<T> onLoaded)
		{
			static_assert((std::is_same_v<T, SupportedResources> || ...),
				"Запрашиваемый тип ресурса не поддерживается данным GpuResourceManager!");

			GetTable<T>().GetOrRequest(
				guid,
				std::move(context),
				std::move(onLoaded),
				GetDispatcher(),
				[this](const Guid& g)
				{
					RequestFromCpu<T>(g);
				});
		}

		template<typename T>
		void GetAsync(
			const Guid& guid,
			GpuResourceCallback<T> onLoaded)
		{
			static_assert((std::is_same_v<T, SupportedResources> || ...),
				"Запрашиваемый тип ресурса не поддерживается данным GpuResourceManager!");

			GetTable<T>().GetOrRequest(
				guid,
				std::move(onLoaded),
				GetDispatcher(),
				[this](const Guid& g)
				{
					RequestFromCpu<T>(g);
				});
		}

	protected:
		void EmergencyStop()
		{
			(GetTable<SupportedResources>().Clear(), ...);
		}

		[[nodiscard]] auto GetDispatcher()
		{
			return [this](auto task) { m_MainThreadQueue.Push(std::move(task)); };
		}

		template<typename TGpu>
		void RequestFromCpu(const Guid& guid)
		{
			using TCpu = typename TGpu::CpuType;

			m_CpuManager->GetAsync<TCpu>(guid, [this, guid](auto cpuRes)
			{
				if (!cpuRes)
				{
					GetTable<TGpu>().Resolve(guid, std::unexpected(cpuRes.error()), GetDispatcher());
					return;
				}

				m_TaskDispatcher.Submit(eTaskPriority::Normal, [this, guid, cpuRes = std::move(*cpuRes)]()
				{
					try
					{
						auto gpuObj = CreateGpuResource(cpuRes, m_GAPI.get());
						GetTable<TGpu>().Resolve(guid, std::move(gpuObj), GetDispatcher());
					}
					catch (const std::exception& ex)
					{
						GetTable<TGpu>().Resolve(guid, std::unexpected(std::format("Ошибка создания GPU-ресурса: {}", ex.what())), GetDispatcher());
					}
					catch (...)
					{
						GetTable<TGpu>().Resolve(guid, std::unexpected("Неизвестная ошибка создания GPU-ресурса"), GetDispatcher());
					}
				});
			});
		}

		TaskDispatcher& m_TaskDispatcher;
		std::shared_ptr<GAPI> m_GAPI;
		std::shared_ptr<CoreCpuResourceManager> m_CpuManager;

		templates::CallbackQueue<> m_MainThreadQueue;
		std::tuple<ResourceTable<SupportedResources>...> m_Tables;
	};

	// Автоматическое разворачивание CoreGpuResourceManager из eEngineResourceType
	template<size_t... Is>
	auto BuildGpuManager(std::index_sequence<Is...>)
		-> GpuResourceManager<typename ResourceBinding<static_cast<eEngineResourceType>(Is)>::Gpu...>;

	using CoreGpuResourceManager = decltype(BuildGpuManager(std::make_index_sequence<static_cast<size_t>(eEngineResourceType::_Count)>{}));
}
