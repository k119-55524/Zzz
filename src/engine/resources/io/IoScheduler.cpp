#include <logger.h>

#include "IoScheduler.h"
#include "core/utils/Ensure.h"
#include "core/utils/ThreadUtils.h"

Z_SET_LOG_CATEGORY(::zzz::core::LogEngine);

namespace zzz::engine
{
	IoScheduler::IoScheduler(
		std::shared_ptr<core::FileSystem> fileSystem,
		std::string defaultArchivePath)
		: m_FileSystem(std::move(fileSystem))
		, m_DefaultArchivePath(std::move(defaultArchivePath))
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
		}

		if (m_IoThread.joinable())
		{
			m_IoThread.join();
		}

		m_Requests.Clear();
	}

	void IoScheduler::QueueRead(
		const Guid& guid,
		const PackageEntry& entry,
		eTaskPriority priority,
		std::function<void(std::expected<std::vector<std::byte>, std::string>)> onCompleted,
		std::string archivePath,
		eFileLocation location)
	{
		ensure(onCompleted != nullptr, "IoScheduler::QueueRead: onCompleted не должен быть null");

		IoReadRequest req
		{
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
	}

	void IoScheduler::IoWorker(std::stop_token stopToken)
	{
		SetCurrentThreadName("IoSchedulerThread");
		SetCurrentThreadPriority(eThreadPriority::Normal);

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

				const size_t readSize = req.entry.GetSize();
				const std::string_view path = req.archivePath.empty() ? std::string_view(m_DefaultArchivePath) : std::string_view(req.archivePath);
				auto readRes = m_FileSystem->ReadBytes(req.location, path, req.entry.GetOffset(), readSize);

				req.onCompleted(std::move(readRes));
			}
		}
	}
}
