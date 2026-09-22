#pragma once

#include <span>
#include <atomic>
#include <string>
#include <format>
#include <memory>
#include <vector>
#include <expected>
#include <functional>
#include <mutex>
#include <condition_variable>

#include "core/utils/Guid.h"
#include "core/io/FileSystem.h"
#include "core/enums/eResourceType.h"
#include "core/enums/eFileLocation.h"
#include "core/constants/PackageConstants.h"
#include "engine/tasks/TaskDispatcher.h"
#include "engine/resources/cpu/CpuMesh.h"
#include "core/io/package/PackageEntry.h"
#include "engine/resources/ResourceRef.h"
#include "engine/resources/cpu/CpuShader.h"
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
	 * @details Направляет задачи чтения и парсинга в TaskDispatcher с запрошенным приоритетом (High, Normal, Background).
	 *          CPU-ресурсы не кэшируются на CPU и освобождаются сразу после создания GPU-ресурсов.
	 */
	class CpuResourceManager final
	{
		Z_NO_COPY_MOVE(CpuResourceManager);

		struct TaskGuard
		{
			Z_NO_COPY_MOVE(TaskGuard);

			explicit TaskGuard(CpuResourceManager& manager) noexcept : m_Manager(manager) {}

			~TaskGuard()
			{
				std::lock_guard lock(m_Manager.m_ShutdownMutex);
				if (--m_Manager.m_ActiveIoTasks == 0)
				{
					m_Manager.m_ShutdownCv.notify_all();
				}
			}

		private:
			CpuResourceManager& m_Manager;
		};

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
			ensure(priority != eTaskPriority::Critical,
				"CpuResourceManager::GetAsync: Приоритет Critical зарезервирован строго для кадровых задач движка!");

			bool isStopping = false;
			{
				std::lock_guard lock(m_ShutdownMutex);
				if (m_IsStopping.load(std::memory_order_relaxed))
				{
					isStopping = true;
				}
				else
				{
					++m_ActiveIoTasks;
				}
			}

			if (isStopping)
			{
				if (auto ctx = context.lock())
				{
					onLoaded(std::unexpected("CpuResourceManager is stopping"));
				}
				return;
			}

			try
			{
				m_TaskDispatcher.Submit(priority, [this, guid, type = GetCpuResourceType<T>(), context = std::move(context), onLoaded = std::move(onLoaded)]() mutable
				{
					TaskGuard taskGuard(*this);

					if (m_IsStopping.load(std::memory_order_acquire) || context.expired())
					{
						if (auto ctx = context.lock())
							onLoaded(std::unexpected("CpuResourceManager is stopping or context expired"));
						return;
					}

					auto loc = m_DataAssetsManager->GetAssetLocation(type, guid);
					if (!loc)
					{
						if (auto ctx = context.lock())
							onLoaded(std::unexpected(loc.error()));
						return;
					}

					std::expected<std::vector<std::byte>, std::string> readRes;
					if (loc->size == 0)
					{
						readRes = std::vector<std::byte>{};
					}
					else
					{
						readRes = m_FileSystem->ReadBytes(loc->location, loc->relativePath, loc->offset, loc->size);
					}

					if (!readRes)
					{
						if (auto ctx = context.lock())
							onLoaded(std::unexpected(std::move(readRes.error())));
						return;
					}

					if (m_IsStopping.load(std::memory_order_acquire) || context.expired())
					{
						if (auto ctx = context.lock())
							onLoaded(std::unexpected("CpuResourceManager is stopping or context expired"));
						return;
					}

					try
					{
						auto parseRes = T::CreateCpuResourceFromPackageBytes(
							loc->entry ? *loc->entry : PackageEntry{},
							*readRes
						);

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
			}
			catch (...)
			{
				{
					std::lock_guard lock(m_ShutdownMutex);
					if (--m_ActiveIoTasks == 0)
					{
						m_ShutdownCv.notify_all();
					}
				}
				throw;
			}
		}

	private:
		TaskDispatcher& m_TaskDispatcher;
		std::shared_ptr<DataAssetsManager> m_DataAssetsManager;
		std::shared_ptr<FileSystem> m_FileSystem;
		std::atomic<bool> m_IsStopping{false};
		size_t m_ActiveIoTasks{0};
		std::mutex m_ShutdownMutex;
		std::condition_variable m_ShutdownCv;
	};
}
