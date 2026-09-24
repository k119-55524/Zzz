#pragma once

#include <span>
#include <atomic>
#include <string>
#include <format>
#include <memory>
#include <expected>
#include <functional>
#include <mutex>
#include <condition_variable>

#include "core/utils/Guid.h"
#include "core/enums/eResourceType.h"
#include "engine/tasks/TaskDispatcher.h"
#include "core/io/package/PackageEntry.h"
#include "engine/resources/ResourceRef.h"
#include "core/io/package/DataAssetsManager.h"

using namespace zzz::core;

namespace zzz::engine
{
	template<typename T>
	concept ParsableCpuResource = requires(const PackageEntry& entry, std::span<const std::byte> bytes)
	{
		{ T::c_ResourceType } -> std::convertible_to<eResourceType>;
		{ T::CreateCpuResourceFromPackageBytes(entry, bytes) } -> std::same_as<std::expected<std::shared_ptr<T>, std::string>>;
	};

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

			explicit TaskGuard(CpuResourceManager& manager) noexcept : m_Manager(manager), m_Released(false) {}

			void Release() noexcept
			{
				if (!m_Released)
				{
					m_Released = true;
					std::lock_guard lock(m_Manager.m_ShutdownMutex);
					if (--m_Manager.m_ActiveIoTasks == 0)
					{
						m_Manager.m_ShutdownCv.notify_all();
					}
				}
			}

			~TaskGuard()
			{
				Release();
			}

		private:
			CpuResourceManager& m_Manager;
			bool m_Released{false};
		};

	public:
		CpuResourceManager() = delete;

		explicit CpuResourceManager(
			TaskDispatcher& taskDispatcher,
			std::shared_ptr<DataAssetsManager> dataAssetsManager);

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

			bool submitted = false;
			try
			{
				submitted = m_TaskDispatcher.Submit(priority, [this, guid, type = T::c_ResourceType, context, onLoaded]() mutable
				{
					TaskGuard taskGuard(*this);

					if (m_IsStopping.load(std::memory_order_acquire) || context.expired())
					{
						taskGuard.Release();
						if (auto ctx = context.lock())
							onLoaded(std::unexpected("CpuResourceManager is stopping or context expired"));
						return;
					}

					const auto* entry = m_DataAssetsManager->GetEntry(type, guid);
					if (!entry)
					{
						taskGuard.Release();
						if (auto ctx = context.lock())
							onLoaded(std::unexpected(std::format("Package entry of type {} with GUID '{}' was not found.", ToString(type), guid.ToString())));
						return;
					}

					auto payloadRes = m_DataAssetsManager->ReadRawPayload(*entry);
					if (!payloadRes)
					{
						std::string err = std::move(payloadRes.error());
						taskGuard.Release();
						if (auto ctx = context.lock())
							onLoaded(std::unexpected(std::move(err)));
						return;
					}

					if (m_IsStopping.load(std::memory_order_acquire) || context.expired())
					{
						taskGuard.Release();
						if (auto ctx = context.lock())
							onLoaded(std::unexpected("CpuResourceManager is stopping or context expired"));
						return;
					}

					std::expected<ResourceRef<T>, std::string> finalResult;
					try
					{
						auto parseRes = T::CreateCpuResourceFromPackageBytes(
							*entry,
							payloadRes->GetSpan()
						);

						if (!parseRes)
							finalResult = std::unexpected(std::move(parseRes.error()));
						else
							finalResult = ResourceRef<T>(std::move(*parseRes));
					}
					catch (const std::exception& ex)
					{
						finalResult = std::unexpected(std::format("Исключение при десериализации ресурса: {}", ex.what()));
					}
					catch (...)
					{
						finalResult = std::unexpected(std::string("Неизвестное исключение при десериализации ресурса"));
					}

					taskGuard.Release();

					if (auto ctx = context.lock())
					{
						onLoaded(std::move(finalResult));
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

			if (!submitted)
			{
				{
					std::lock_guard lock(m_ShutdownMutex);
					if (--m_ActiveIoTasks == 0)
					{
						m_ShutdownCv.notify_all();
					}
				}
				if (auto ctx = context.lock())
				{
					onLoaded(std::unexpected("TaskDispatcher rejected task: pool is closed"));
				}
			}
		}

	private:
		TaskDispatcher& m_TaskDispatcher;
		std::shared_ptr<DataAssetsManager> m_DataAssetsManager;
		std::atomic<bool> m_IsStopping{false};
		size_t m_ActiveIoTasks{0};
		std::mutex m_ShutdownMutex;
		std::condition_variable m_ShutdownCv;
	};
}
