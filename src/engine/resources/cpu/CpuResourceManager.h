#pragma once

#include <memory>
#include <string>
#include <format>
#include <expected>
#include <atomic>
#include <span>
#include <vector>

#include "core/utils/Guid.h"
#include "core/io/FileSystem.h"
#include "core/enums/eResourceType.h"
#include "engine/tasks/TaskDispatcher.h"
#include "core/io/package/PackageEntry.h"
#include "core/templates/CallbackQueue.h"
#include "core/constants/PackageConstants.h"
#include "engine/resources/ResourceRef.h"
#include "engine/resources/cpu/CpuMesh.h"
#include "engine/resources/ResourceTable.h"
#include "engine/resources/cpu/CpuShader.h"
#include "engine/resources/cpu/CpuMaterial.h"
#include "engine/resources/cpu/CpuTexture2D.h"
#include "engine/resources/io/IoScheduler.h"
#include "core/io/ResourceStorageTraits.h"
#include "core/io/package/DataAssetsManager.h"

using namespace zzz::core;

namespace zzz::engine
{
	class PackageManager;

	/**
	 * @class CpuResourceManager
	 * @brief Централизованный сервис асинхронной загрузки и кэширования CPU-ресурсов.
	 * @details Хранит типизированные таблицы ресурсов m_Meshes, m_Materials, m_Textures, m_Shaders.
	 *          Владеет приватным планировщиком дискового ввода-вывода IoScheduler.
	 */
	class CpuResourceManager final
	{
		Z_NO_COPY_MOVE(CpuResourceManager);

	public:
		CpuResourceManager() = delete;

		explicit CpuResourceManager(
			TaskDispatcher& taskDispatcher,
			std::shared_ptr<PackageManager> packageManager,
			std::shared_ptr<DataAssetsManager> dataAssetsManager = nullptr,
			std::shared_ptr<FileSystem> fileSystem = nullptr);

		~CpuResourceManager();

		inline void Update() noexcept {}

		template<typename T, typename ContextType>
		void GetAsync(
			const Guid& guid,
			std::weak_ptr<ContextType> context,
			std::function<void(std::expected<ResourceRef<T>, std::string>)> onLoaded,
			eTaskPriority priority = eTaskPriority::Normal)
		{
			GetTable<T>().GetOrRequest(guid,
				std::move(context),
				[cb = std::move(onLoaded)](typename ResourceTable<T>::ResultType res)
				{
					if (!res)
					{
						cb(std::unexpected(std::move(res.error())));
					}
					else
					{
						cb(ResourceRef<T>(std::move(*res)));
					}
				},
				[this, priority](const Guid& g)
				{
					DispatchLoad<T>(g, priority);
				});
		}

		template<typename T>
		void GetAsync(
			const Guid& guid,
			std::function<void(std::expected<ResourceRef<T>, std::string>)> onLoaded,
			eTaskPriority priority = eTaskPriority::Normal)
		{
			GetTable<T>().GetOrRequest(guid,
				[cb = std::move(onLoaded)](typename ResourceTable<T>::ResultType res)
				{
					if (!res)
					{
						cb(std::unexpected(std::move(res.error())));
					}
					else
					{
						cb(ResourceRef<T>(std::move(*res)));
					}
				},
				[this, priority](const Guid& g)
				{
					DispatchLoad<T>(g, priority);
				});
		}

		template<typename T>
		[[nodiscard]] ResourceRef<T> TryGet(const Guid& guid)
		{
			auto res = GetTable<T>().TryGet(guid);
			return res ? ResourceRef<T>(std::move(res)) : ResourceRef<T>{};
		}

		void Stop();
		void Clear();

	private:
		template<typename T>
		[[nodiscard]] auto& GetTable() noexcept
		{
			if constexpr (std::is_same_v<T, CpuMesh>)           return m_Meshes;
			else if constexpr (std::is_same_v<T, CpuMaterial>)  return m_Materials;
			else if constexpr (std::is_same_v<T, CpuTexture2D>) return m_Textures;
			else if constexpr (std::is_same_v<T, CpuShader>)    return m_Shaders;
			else static_assert(sizeof(T) == 0, "Запрашиваемый тип ресурса не поддерживается CpuResourceManager!");
		}

		void EmergencyStop();

		template<typename T>
		void DispatchLoad(const Guid& guid, eTaskPriority priority);

		template<typename T>
		std::expected<std::shared_ptr<T>, std::string> LoadResourceSync(const Guid& guid);

		template<typename T>
		std::expected<std::shared_ptr<T>, std::string> LoadFromBytes(const PackageEntry& entry, std::span<const std::byte> bytes);

		[[nodiscard]] std::expected<PackageEntry, std::string> FindEntry(const Guid& guid, eResourceType type) const;

		TaskDispatcher& m_TaskDispatcher;
		std::shared_ptr<PackageManager> m_PackageManager;
		std::shared_ptr<DataAssetsManager> m_DataAssetsManager;
		std::shared_ptr<FileSystem> m_FileSystem;
		std::unique_ptr<IoScheduler> m_IoScheduler;

		std::atomic<bool> m_IsRunning{ true };

		ResourceTable<CpuMesh>      m_Meshes;
		ResourceTable<CpuMaterial>  m_Materials;
		ResourceTable<CpuTexture2D> m_Textures;
		ResourceTable<CpuShader>    m_Shaders;
	};

	template<typename T>
	constexpr eResourceType GetCpuResourceType() noexcept
	{
		if constexpr (std::is_same_v<T, CpuMesh>)           return eResourceType::Mesh;
		else if constexpr (std::is_same_v<T, CpuMaterial>)  return eResourceType::Material;
		else if constexpr (std::is_same_v<T, CpuTexture2D>) return eResourceType::Texture2D;
		else if constexpr (std::is_same_v<T, CpuShader>)    return eResourceType::Shader;
	}

	template<typename T>
	void CpuResourceManager::DispatchLoad(const Guid& guid, eTaskPriority priority)
	{
		if (!m_IsRunning.load(std::memory_order_acquire))
		{
			GetTable<T>().Resolve(guid, std::unexpected(std::string("CpuResourceManager остановлен")));
			return;
		}

		constexpr auto type = GetCpuResourceType<T>();
		auto entryRes = FindEntry(guid, type);
		if (!entryRes)
		{
			GetTable<T>().Resolve(guid, std::unexpected(entryRes.error()));
			return;
		}

		const auto storageKind = GetResourceStorageTraits(type).storageKind;
		std::string archivePath;
		if (storageKind == eResourceStorageKind::DataArchive)
		{
			archivePath = c_DataPackageRelativePath;
		}
		else if (storageKind == eResourceStorageKind::PackageArchive)
		{
			archivePath = c_GamePackageRelativePath;
		}

		auto onIoComplete = [this, guid, priority, entry = *entryRes](
			std::expected<std::vector<std::byte>, std::string> bytesRes,
			InFlightPermit permit) mutable
		{
			if (!bytesRes)
			{
				GetTable<T>().Resolve(guid, std::unexpected(std::move(bytesRes.error())));
				return;
			}

			auto sharedPermit = std::make_shared<InFlightPermit>(std::move(permit));
			auto sharedBytes = std::make_shared<std::vector<std::byte>>(std::move(*bytesRes));

			const bool enqueued = m_TaskDispatcher.Submit(priority,
				[this, guid, entry = std::move(entry), sharedBytes, sharedPermit]() mutable
			{
				try
				{
					auto parseRes = LoadFromBytes<T>(entry, *sharedBytes);
					GetTable<T>().Resolve(guid, std::move(parseRes));
				}
				catch (const std::exception& ex)
				{
					GetTable<T>().Resolve(guid, std::unexpected(std::format("Исключение при десериализации ресурса: {}", ex.what())));
				}
				catch (...)
				{
					GetTable<T>().Resolve(guid, std::unexpected(std::string("Неизвестное исключение при десериализации ресурса")));
				}
			});

			if (!enqueued)
			{
				GetTable<T>().Resolve(guid, std::unexpected(std::string("Не удалось поставить задачу десериализации в TaskDispatcher (пул закрыт)")));
			}
		};

		if (m_IoScheduler)
		{
			const bool queued = m_IoScheduler->QueueRead(guid, *entryRes, eFileLocation::App, std::move(archivePath), priority, std::move(onIoComplete));
			if (!queued)
			{
				GetTable<T>().Resolve(guid, std::unexpected(std::string("Не удалось поставить задачу в IoScheduler (планировщик закрыт)")));
			}
		}
		else
		{
			GetTable<T>().Resolve(guid, std::unexpected(std::string("IoScheduler не инициализирован")));
		}
	}
}
