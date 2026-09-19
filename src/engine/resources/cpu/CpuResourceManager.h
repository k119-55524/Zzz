#pragma once

#include <tuple>
#include <memory>
#include <string>
#include <format>
#include <expected>
#include <functional>
#include <string_view>

#include "core/utils/Guid.h"
#include "core/io/FileSystem.h"
#include "core/enums/eResourceType.h"
#include "core/io/package/SceneData.h"
#include "core/io/package/PackageEntry.h"
#include "core/templates/CallbackQueue.h"
#include "engine/tasks/TaskDispatcher.h"
#include "engine/resources/ResourceTable.h"
#include "engine/resources/ResourceTypes.h"
#include "core/io/package/DataAssetsManager.h"

using namespace zzz::core;

namespace zzz::engine
{
	class PackageManager;

	template <typename T>
	using CpuResourceResult = std::expected<std::shared_ptr<T>, std::string>;

	template <typename T>
	using CpuResourceCallback = std::function<void(CpuResourceResult<T>)>;

	template<typename T, typename Mgr>
	std::expected<std::shared_ptr<T>, std::string> LoadCpuResourceSync(
		const Guid& guid,
		Mgr& manager);

	/**
	 * @class CpuResourceManager
	 * @brief Шаблонный потокобезопасный центральный сервис загрузки и кэширования CPU-ресурсов.
	 * @details Автоматически разворачивает типизированные таблицы в std::tuple для всех типов из SupportedResources...
	 *          Задачи I/O и десериализации отправляются в пул потоков TaskDispatcher.
	 */
	template<typename... SupportedResources>
	class CpuResourceManager
	{
		Z_NO_COPY_MOVE(CpuResourceManager);

	public:
		CpuResourceManager() = delete;

		explicit CpuResourceManager(
			TaskDispatcher& taskDispatcher,
			std::shared_ptr<PackageManager> packageManager,
			std::shared_ptr<DataAssetsManager> dataAssetsManager = nullptr,
			std::shared_ptr<FileSystem> fileSystem = nullptr)
			: m_TaskDispatcher(taskDispatcher)
			, m_PackageManager(std::move(packageManager))
			, m_DataAssetsManager(std::move(dataAssetsManager))
			, m_FileSystem(std::move(fileSystem))
		{
		}

		virtual ~CpuResourceManager()
		{
			EmergencyStop();
		}

		// --- Обновление и доставка колбэков на главном потоке ---
		void Update()
		{
			m_MainThreadQueue.ExecuteAll();
		}

		/// @brief Прямой доступ к типизированной таблице ресурса T
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

		// --- Загрузка структуры сцены (SceneData) ---
		[[nodiscard]] std::expected<SceneData, std::string> LoadSceneData(const Guid& sceneGuid);
		[[nodiscard]] std::expected<SceneData, std::string> LoadSceneData(std::string_view sceneName);

		[[nodiscard]] std::expected<PackageEntry, std::string> FindEntry(const Guid& guid, eResourceType type) const;

		// --- Единая шаблонная функция асинхронного доступа GetAsync<T> ---
		template<typename T, typename ContextType>
		void GetAsync(
			const Guid& guid,
			std::weak_ptr<ContextType> context,
			CpuResourceCallback<T> onLoaded)
		{
			static_assert((std::is_same_v<T, SupportedResources> || ...),
				"Запрашиваемый тип ресурса не поддерживается данным CpuResourceManager!");

			GetTable<T>().GetOrRequest(
				guid,
				std::move(context),
				std::move(onLoaded),
				GetDispatcher(),
				[this](const Guid& g)
				{
					DispatchLoad<T>(g);
				});
		}

		template<typename T>
		void GetAsync(
			const Guid& guid,
			CpuResourceCallback<T> onLoaded)
		{
			static_assert((std::is_same_v<T, SupportedResources> || ...),
				"Запрашиваемый тип ресурса не поддерживается данным CpuResourceManager!");

			GetTable<T>().GetOrRequest(
				guid,
				std::move(onLoaded),
				GetDispatcher(),
				[this](const Guid& g)
				{
					DispatchLoad<T>(g);
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

		template<typename T>
		void DispatchLoad(const Guid& guid)
		{
			m_TaskDispatcher.Submit(eTaskPriority::Normal, [this, guid]()
			{
				try
				{
					auto result = LoadCpuResourceSync<T>(guid, *this);
					GetTable<T>().Resolve(guid, std::move(result), GetDispatcher());
				}
				catch (const std::exception& ex)
				{
					GetTable<T>().Resolve(guid, std::unexpected(std::format("Исключение при загрузке: {}", ex.what())), GetDispatcher());
				}
				catch (...)
				{
					GetTable<T>().Resolve(guid, std::unexpected("Неизвестное исключение при загрузке"), GetDispatcher());
				}
			});
		}

		TaskDispatcher& m_TaskDispatcher;
		std::shared_ptr<PackageManager> m_PackageManager;
		std::shared_ptr<DataAssetsManager> m_DataAssetsManager;
		std::shared_ptr<FileSystem> m_FileSystem;

		templates::CallbackQueue<> m_MainThreadQueue;
		std::tuple<ResourceTable<SupportedResources>...> m_Tables;

		template<typename T, typename Mgr>
		friend std::expected<std::shared_ptr<T>, std::string> LoadCpuResourceSync(
			const Guid& guid,
			Mgr& manager);
	};

	// Автоматическое разворачивание CoreCpuResourceManager из eEngineResourceType
	template<size_t... Is>
	auto BuildCpuManager(std::index_sequence<Is...>)
		-> CpuResourceManager<typename ResourceBinding<static_cast<eEngineResourceType>(Is)>::Cpu...>;

	using CoreCpuResourceManager = decltype(BuildCpuManager(std::make_index_sequence<static_cast<size_t>(eEngineResourceType::_Count)>{}));
}
