#pragma once

#include <span>
#include <vector>
#include <memory>
#include <string>
#include <format>
#include <expected>

#include "core/utils/Guid.h"
#include "core/io/FileSystem.h"
#include "core/enums/eResourceType.h"
#include "engine/tasks/TaskDispatcher.h"
#include "core/io/package/PackageEntry.h"
#include "engine/resources/ResourceRef.h"
#include "engine/resources/cpu/CpuMesh.h"
#include "core/io/ResourceStorageTraits.h"
#include "engine/resources/cpu/CpuShader.h"
#include "core/constants/PackageConstants.h"
#include "engine/resources/io/IoScheduler.h"
#include "engine/resources/cpu/CpuMaterial.h"
#include "engine/resources/cpu/CpuTexture2D.h"
#include "core/io/package/DataAssetsManager.h"

using namespace zzz::core;

namespace zzz::engine
{
	class PackageManager;

	template<typename T>
	concept ParsableCpuResource = requires(const core::PackageEntry& entry, std::span<const std::byte> bytes)
	{
		{ T::CreateFromMemory(entry, bytes) } -> std::same_as<std::expected<std::shared_ptr<T>, std::string>>;
	};

	/**
	 * @class CpuResourceManager
	 * @brief Транзитный сервис асинхронного дискового ввода-вывода и десериализации CPU-ресурсов.
	 * @details Загружает сырые данные из архивов через IoScheduler, десериализует их в структуры
	 *          (CpuMesh, CpuMaterial и т.д.) на воркерах TaskDispatcher и передаёт заказчику.
	 *          CPU-ресурсы не кэшируются на CPU и освобождаются сразу после создания GPU-ресурсов.
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

		template<typename T, typename ContextType>
		void GetAsync(
			const Guid& guid,
			std::weak_ptr<ContextType> context,
			std::function<void(std::expected<ResourceRef<T>, std::string>)> onLoaded,
			eTaskPriority priority = eTaskPriority::Normal)
		{
			static_assert(ParsableCpuResource<T>, "Тип ресурса обязан поддерживать статический метод CreateFromMemory");
			constexpr auto type = GetCpuResourceType<T>();
			auto entryRes = FindEntry(guid, type);
			if (!entryRes)
			{
				if (auto ctx = context.lock())
				{
					onLoaded(std::unexpected(entryRes.error()));
				}
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

			auto onIoComplete = [this, guid, priority, entry = *entryRes, context = std::move(context), onLoaded = std::move(onLoaded)](
				std::expected<std::vector<std::byte>, std::string> bytesRes,
				InFlightPermit permit) mutable
			{
				if (!bytesRes)
				{
					if (auto ctx = context.lock())
					{
						onLoaded(std::unexpected(std::move(bytesRes.error())));
					}
					return;
				}

				auto sharedPermit = std::make_shared<InFlightPermit>(std::move(permit));
				auto sharedBytes = std::make_shared<std::vector<std::byte>>(std::move(*bytesRes));

				m_TaskDispatcher.Submit(priority,
					[this, guid, entry = std::move(entry), sharedBytes, sharedPermit, context = std::move(context), onLoaded = std::move(onLoaded)]() mutable
				{
					if (context.expired())
					{
						return;
					}

					try
					{
						auto parseRes = T::CreateFromMemory(entry, *sharedBytes);
						sharedBytes.reset();
						sharedPermit.reset();

						if (auto ctx = context.lock())
						{
							if (!parseRes)
							{
								onLoaded(std::unexpected(std::move(parseRes.error())));
							}
							else
							{
								onLoaded(ResourceRef<T>(std::move(*parseRes)));
							}
						}
					}
					catch (const std::exception& ex)
					{
						if (auto ctx = context.lock())
						{
							onLoaded(std::unexpected(std::format("Исключение при десериализации ресурса: {}", ex.what())));
						}
					}
					catch (...)
					{
						if (auto ctx = context.lock())
						{
							onLoaded(std::unexpected(std::string("Неизвестное исключение при десериализации ресурса")));
						}
					}
				});
			};

			if (m_IoScheduler)
			{
				const bool queued = m_IoScheduler->QueueRead(guid, *entryRes, eFileLocation::App, std::move(archivePath), priority, std::move(onIoComplete));
				if (!queued)
				{
					if (auto ctx = context.lock())
					{
						onLoaded(std::unexpected(std::string("Не удалось поставить задачу в IoScheduler (планировщик закрыт)")));
					}
				}
			}
			else
			{
				if (auto ctx = context.lock())
				{
					onLoaded(std::unexpected(std::string("IoScheduler не инициализирован")));
				}
			}
		}

	private:
		[[nodiscard]] std::expected<PackageEntry, std::string> FindEntry(const Guid& guid, eResourceType type) const;

		TaskDispatcher& m_TaskDispatcher;
		std::shared_ptr<PackageManager> m_PackageManager;
		std::shared_ptr<DataAssetsManager> m_DataAssetsManager;
		std::shared_ptr<FileSystem> m_FileSystem;
		std::unique_ptr<IoScheduler> m_IoScheduler;
	};

	template<typename T>
	constexpr eResourceType GetCpuResourceType() noexcept
	{
		if constexpr (std::is_same_v<T, CpuMesh>)           return eResourceType::Mesh;
		else if constexpr (std::is_same_v<T, CpuMaterial>)  return eResourceType::Material;
		else if constexpr (std::is_same_v<T, CpuTexture2D>) return eResourceType::Texture2D;
		else if constexpr (std::is_same_v<T, CpuShader>)    return eResourceType::Shader;
	}
}
