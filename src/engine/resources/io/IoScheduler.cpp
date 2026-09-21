
#include <logger.h>

#include "core/utils/Ensure.h"
#include "core/utils/ThreadUtils.h"

#include "IoScheduler.h"

Z_SET_LOG_CATEGORY(::zzz::core::LogEngine);

namespace zzz::engine
{
	InFlightPermit::InFlightPermit(IoScheduler* scheduler, size_t bytes) noexcept :
			m_Scheduler(scheduler), 
			m_Bytes(bytes)
	{
	}

	InFlightPermit::~InFlightPermit()
	{
		Release();
	}

	InFlightPermit::InFlightPermit(InFlightPermit&& other) noexcept :
			m_Scheduler(other.m_Scheduler),
			m_Bytes(other.m_Bytes)
	{
		other.m_Scheduler = nullptr;
		other.m_Bytes = 0;
	}

	InFlightPermit& InFlightPermit::operator=(InFlightPermit&& other) noexcept
	{
		if (this != &other)
		{
			Release();
			m_Scheduler = other.m_Scheduler;
			m_Bytes = other.m_Bytes;
			other.m_Scheduler = nullptr;
			other.m_Bytes = 0;
		}
		return *this;
	}

	void InFlightPermit::Release() noexcept
	{
		if (m_Scheduler)
		{
			m_Scheduler->ReleasePermit(m_Bytes);
			m_Scheduler = nullptr;
			m_Bytes = 0;
		}
	}

	IoScheduler::IoScheduler(
		std::shared_ptr<core::FileSystem> fileSystem,
		size_t maxInFlightBytes,
		size_t maxInFlightRequests)
		: m_FileSystem(std::move(fileSystem))
		, m_MaxInFlightBytes(maxInFlightBytes)
		, m_MaxInFlightRequests(maxInFlightRequests)
	{
		ensure(m_FileSystem != nullptr, "IoScheduler: FileSystem не должен быть null");
		m_IoThread = std::jthread([this](std::stop_token stopToken)
		{
			IoWorker(stopToken);
		});
	}

	IoScheduler::~IoScheduler()
	{
		Stop();
	}

	void IoScheduler::Stop()
	{
		if (m_IoThread.joinable())
		{
			m_IoThread.request_stop();
		}

		{
			std::lock_guard lock(m_CvMutex);
			m_QueueCv.notify_all();
			m_BackpressureCv.notify_all();
		}

		if (m_IoThread.joinable())
		{
			m_IoThread.join();
		}

		m_Requests.Clear();
	}

	void IoScheduler::ReleasePermit(size_t bytes) noexcept
	{
		m_CurrentInFlightBytes.fetch_sub(bytes, std::memory_order_relaxed);
		m_CurrentInFlightRequests.fetch_sub(1, std::memory_order_relaxed);

		std::lock_guard lock(m_CvMutex);
		m_BackpressureCv.notify_one();
	}

	bool IoScheduler::QueueRead(
		const core::Guid& guid,
		const core::PackageEntry& entry,
		core::eFileLocation location,
		std::string archivePath,
		eTaskPriority priority,
		std::function<void(std::expected<std::vector<std::byte>, std::string>, InFlightPermit)> onCompleted)
	{
		ensure(onCompleted != nullptr, "IoScheduler::QueueRead: onCompleted не должен быть null");

		IoReadRequest req{
			.guid = guid,
			.entry = entry,
			.location = location,
			.archivePath = std::move(archivePath),
			.priority = priority,
			.onCompleted = std::move(onCompleted)
		};

		m_Requests.Push(std::move(req));

		{
			std::lock_guard lock(m_CvMutex);
			m_QueueCv.notify_one();
		}

		return true;
	}

	void IoScheduler::IoWorker(std::stop_token stopToken)
	{
		core::SetCurrentThreadName("IoSchedulerThread");
		core::SetCurrentThreadPriority(core::eThreadPriority::Normal);

		while (!stopToken.stop_requested())
		{
			{
				std::unique_lock lock(m_CvMutex);
				m_QueueCv.wait(lock, [this, &stopToken]()
				{
					return m_Requests.HasPendingWrites() || stopToken.stop_requested();
				});
			}

			if (stopToken.stop_requested())
				break;

			auto& batch = m_Requests.SwapAndGetReadBuffer();
			for (auto& req : batch)
			{
				if (stopToken.stop_requested())
					break;

				// Контроль Backpressure перед чтением с диска
				{
					std::unique_lock lock(m_CvMutex);
					m_BackpressureCv.wait(lock, [this, &stopToken]()
					{
						const bool withinBytes = m_CurrentInFlightBytes.load(std::memory_order_relaxed) < m_MaxInFlightBytes;
						const bool withinRequests = m_CurrentInFlightRequests.load(std::memory_order_relaxed) < m_MaxInFlightRequests;
						return (withinBytes && withinRequests) || stopToken.stop_requested();
					});
				}

				if (stopToken.stop_requested())
					break;

				const size_t readSize = req.entry.GetSize();
				auto readRes = m_FileSystem->ReadBytes(req.location, req.archivePath, req.entry.GetOffset(), readSize);

				// Учитываем токен в полёте
				m_CurrentInFlightBytes.fetch_add(readSize, std::memory_order_relaxed);
				m_CurrentInFlightRequests.fetch_add(1, std::memory_order_relaxed);
				InFlightPermit permit(this, readSize);

				// Передаём прочитанные байты и токен подписчику
				req.onCompleted(std::move(readRes), std::move(permit));
			}
		}
	}
}
