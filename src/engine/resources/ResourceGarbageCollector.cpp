#include "ResourceGarbageCollector.h"
#include "ResourceManager.h"
#include <logger/logger.h>

Z_SET_LOG_CATEGORY(::zzz::core::LogEngine);

namespace zzz::engine
{
	ResourceGarbageCollector::ResourceGarbageCollector(
		ResourceManager& resourceManager,
		std::chrono::milliseconds interval)
		: m_ResourceManager(resourceManager)
		, m_Interval(interval)
	{
	}

	ResourceGarbageCollector::~ResourceGarbageCollector()
	{
		Stop();
	}

	void ResourceGarbageCollector::Start()
	{
		if (m_IsRunning.exchange(true, std::memory_order_acq_rel))
			return;

		m_Thread = std::jthread([this](std::stop_token st) {
			WorkerLoop(std::move(st));
		});
	}

	void ResourceGarbageCollector::Stop()
	{
		if (!m_IsRunning.exchange(false, std::memory_order_acq_rel))
			return;

		if (m_Thread.joinable())
		{
			m_Thread.request_stop();
			m_Cv.notify_all();
			m_Thread.join();
		}
	}

	void ResourceGarbageCollector::Pause()
	{
		m_PauseCount.fetch_add(1, std::memory_order_release);
	}

	void ResourceGarbageCollector::Resume()
	{
		const auto prev = m_PauseCount.fetch_sub(1, std::memory_order_acq_rel);
		ensure(prev > 0, "Непарный вызов ResourceGarbageCollector::Resume().");
	}

	void ResourceGarbageCollector::WorkerLoop(std::stop_token stopToken)
	{
		while (!stopToken.stop_requested())
		{
			{
				std::unique_lock lock(m_Mutex);
				m_Cv.wait_for(lock, stopToken, m_Interval, [this, &stopToken]() {
					return stopToken.stop_requested();
				});
			}

			if (stopToken.stop_requested())
				break;

			if (!IsPaused())
			{
				m_ResourceManager.UnloadUnused([this, &stopToken]() {
					return stopToken.stop_requested() || IsPaused();
				});
			}
		}
	}
}
