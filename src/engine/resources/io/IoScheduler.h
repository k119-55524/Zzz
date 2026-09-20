#pragma once

#include <vector>
#include <memory>
#include <string>
#include <span>
#include <mutex>
#include <atomic>
#include <thread>
#include <expected>
#include <functional>
#include <condition_variable>

#include "core/utils/Guid.h"
#include "core/io/FileSystem.h"
#include "core/enums/eFileLocation.h"
#include "core/io/package/PackageEntry.h"
#include "core/templates/DoubleBufferedVector.h"
#include "engine/tasks/TaskPriority.h"

namespace zzz::engine
{
	class IoScheduler;

	/**
	 * @class InFlightPermit
	 * @brief RAII-токен учёта байтов и запросов ввода-вывода в полёте (Backpressure).
	 * @details Удерживается до тех пор, пока воркер TaskDispatcher не закончит десериализацию ресурса.
	 *          В деструкторе автоматически возвращает квоту в IoScheduler.
	 */
	class InFlightPermit final
	{
	public:
		InFlightPermit() noexcept = default;
		InFlightPermit(IoScheduler* scheduler, size_t bytes) noexcept;
		~InFlightPermit();

		InFlightPermit(const InFlightPermit&) = delete;
		InFlightPermit& operator=(const InFlightPermit&) = delete;

		InFlightPermit(InFlightPermit&& other) noexcept;
		InFlightPermit& operator=(InFlightPermit&& other) noexcept;

		void Release() noexcept;

	private:
		IoScheduler* m_Scheduler = nullptr;
		size_t m_Bytes = 0;
	};

	/**
	 * @struct IoReadRequest
	 * @brief Запрос на асинхронное чтение сырого блока байт из файла/пакета.
	 */
	struct IoReadRequest
	{
		core::Guid guid;
		core::PackageEntry entry;
		core::eFileLocation location = core::eFileLocation::App;
		std::string archivePath;
		eTaskPriority priority = eTaskPriority::Normal;
		std::function<void(std::expected<std::vector<std::byte>, std::string>, InFlightPermit)> onCompleted;
	};

	/**
	 * @class IoScheduler
	 * @brief Выделенный планировщик дискового ввода-вывода с контролем Backpressure.
	 * @details Запускает выделенный поток m_IoThread (std::jthread), последовательно читает
	 *          сырые байты без конкуренции за диск и передаёт их воркерам с токеном InFlightPermit.
	 */
	class IoScheduler final
	{
		Z_NO_COPY_MOVE(IoScheduler);

	public:
		explicit IoScheduler(
			std::shared_ptr<core::FileSystem> fileSystem,
			size_t maxInFlightBytes = 64 * 1024 * 1024,
			size_t maxInFlightRequests = 128);

		~IoScheduler();

		[[nodiscard]] bool QueueRead(
			const core::Guid& guid,
			const core::PackageEntry& entry,
			core::eFileLocation location,
			std::string archivePath,
			eTaskPriority priority,
			std::function<void(std::expected<std::vector<std::byte>, std::string>, InFlightPermit)> onCompleted);

		void Stop();

		void ReleasePermit(size_t bytes) noexcept;

		[[nodiscard]] size_t GetInFlightBytes() const noexcept { return m_CurrentInFlightBytes.load(std::memory_order_relaxed); }
		[[nodiscard]] size_t GetInFlightRequests() const noexcept { return m_CurrentInFlightRequests.load(std::memory_order_relaxed); }

	private:
		void IoWorker(std::stop_token stopToken);

		std::shared_ptr<core::FileSystem> m_FileSystem;
		size_t m_MaxInFlightBytes;
		size_t m_MaxInFlightRequests;

		std::atomic<bool> m_IsRunning{ true };
		std::atomic<size_t> m_CurrentInFlightBytes{ 0 };
		std::atomic<size_t> m_CurrentInFlightRequests{ 0 };

		std::mutex m_CvMutex;
		std::condition_variable m_QueueCv;
		std::condition_variable m_BackpressureCv;

		core::DoubleBufferedVector<IoReadRequest> m_Requests;
		std::jthread m_IoThread;
	};
}
