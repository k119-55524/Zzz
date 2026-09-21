#pragma once

#include <span>
#include <string>
#include <format>
#include <memory>
#include <vector>
#include <expected>

#include "core/utils/Guid.h"
#include "core/io/FileSystem.h"
#include "core/enums/eResourceType.h"
#include "engine/tasks/TaskDispatcher.h"
#include "engine/resources/cpu/CpuMesh.h"
#include "core/io/package/PackageEntry.h"
#include "engine/resources/ResourceRef.h"
#include "engine/resources/cpu/CpuShader.h"
#include "engine/resources/io/IoScheduler.h"
#include "engine/resources/cpu/CpuMaterial.h"
#include "engine/resources/cpu/CpuTexture2D.h"
#include "core/io/package/DataAssetsManager.h"

using namespace zzz::core;

namespace zzz::engine
{
	template<typename T>
	concept ParsableCpuResource = requires(const PackageEntry& entry, std::span<const std::byte> bytes)
	{
		{ T::CreateCpuResourceFromPackageBytes(entry, bytes) } -> std::same_as<std::expected<std::shared_ptr<T>, std::string>>;
	};

	template<typename T>
	constexpr eResourceType GetCpuResourceType() noexcept
	{
		if constexpr (std::is_same_v<T, CpuMesh>)           return eResourceType::Mesh;
		else if constexpr (std::is_same_v<T, CpuMaterial>)  return eResourceType::Material;
		else if constexpr (std::is_same_v<T, CpuTexture2D>) return eResourceType::Texture2D;
		else if constexpr (std::is_same_v<T, CpuShader>)    return eResourceType::Shader;
		else static_assert(sizeof(T) == 0, "Неизвестный тип CPU-ресурса!");
	}

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
			std::shared_ptr<DataAssetsManager> dataAssetsManager,
			std::shared_ptr<FileSystem> fileSystem);

		~CpuResourceManager();

		template<ParsableCpuResource T, typename ContextType>
		void GetAsync(
			const Guid& guid,
			std::weak_ptr<ContextType> context,
			std::function<void(std::expected<ResourceRef<T>, std::string>)> onLoaded,
			eTaskPriority priority)
		{
			constexpr auto type = GetCpuResourceType<T>();
			auto entryOpt = m_DataAssetsManager->GetEntry(type, guid);
			if (!entryOpt)
			{
				if (auto ctx = context.lock())
					onLoaded(std::unexpected(std::format("Ресурс типа {} с GUID '{}' не найден в data.dat", ToString(type), guid.ToString())));

				return;
			}

			auto onIoComplete = [priority, entry = *entryOpt, context = std::move(context), onLoaded = std::move(onLoaded), taskDispatcher = &m_TaskDispatcher](
				std::expected<std::vector<std::byte>, std::string> readResult) mutable
			{
				if (!readResult)
				{
					if (auto ctx = context.lock())
						onLoaded(std::unexpected(std::move(readResult.error())));

					return;
				}

				auto sharedBytes = std::make_shared<std::vector<std::byte>>(std::move(*readResult));
				taskDispatcher->Submit(priority, [entry = std::move(entry), sharedBytes, context = std::move(context), onLoaded = std::move(onLoaded)]() mutable
				{
					if (context.expired())
						return;

					try
					{
						auto parseRes = T::CreateCpuResourceFromPackageBytes(entry, *sharedBytes);
						sharedBytes.reset();

						if (auto ctx = context.lock())
						{
							if (!parseRes)
								onLoaded(std::unexpected(std::move(parseRes.error())));
							else
								onLoaded(ResourceRef<T>(std::move(*parseRes)));
						}
					}
					catch (const std::exception& ex)
					{
						if (auto ctx = context.lock())
							onLoaded(std::unexpected(std::format("Исключение при десериализации ресурса: {}", ex.what())));
					}
					catch (...)
					{
						if (auto ctx = context.lock())
							onLoaded(std::unexpected(std::string("Неизвестное исключение при десериализации ресурса")));
					}
				});
			};

			m_IoScheduler->QueueRead(guid, *entryOpt, priority, std::move(onIoComplete));
		}

	private:
		TaskDispatcher& m_TaskDispatcher;
		std::shared_ptr<DataAssetsManager> m_DataAssetsManager;
		std::shared_ptr<FileSystem> m_FileSystem;
		std::unique_ptr<IoScheduler> m_IoScheduler;
	};
}
