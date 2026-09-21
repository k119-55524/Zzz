#pragma once

#include <span>
#include <mutex>
#include <memory>
#include <string>
#include <thread>
#include <vector>
#include <expected>
#include <functional>
#include <condition_variable>

#include "core/utils/Guid.h"
#include "core/io/FileSystem.h"
#include "core/enums/eFileLocation.h"
#include "engine/tasks/TaskPriority.h"
#include "core/io/package/PackageEntry.h"
#include "core/constants/PackageConstants.h"
#include "core/templates/DoubleBufferedVector.h"

using namespace zzz::core;

namespace zzz::engine
{
	/**
	 * @struct IoReadRequest
	 * @brief Запрос на асинхронное чтение сырого блока байт из файла/пакета.
	 */
	struct IoReadRequest
	{
		Guid guid;
		PackageEntry entry;
		eFileLocation location = eFileLocation::App;
		std::string archivePath;
		eTaskPriority priority = eTaskPriority::Normal;
		std::function<void(std::expected<std::vector<std::byte>, std::string>)> onCompleted;
	};

	/**
	 * @class IoScheduler
	 * @brief Выделенный планировщик дискового ввода-вывода.
	 * @details Запускает выделенный поток m_IoThread (std::jthread) и последовательно
	 *          читает сырые байты без конкуренции за диск.
	 */
	class IoScheduler final
	{
		Z_NO_COPY_MOVE(IoScheduler);

	public:
		explicit IoScheduler(
			std::shared_ptr<core::FileSystem> fileSystem,
			std::string defaultArchivePath = std::string(core::c_DataPackageRelativePath));

		~IoScheduler();

		void QueueRead(
			const Guid& guid,
			const PackageEntry& entry,
			eTaskPriority priority,
			std::function<void(std::expected<std::vector<std::byte>, std::string>)> onCompleted,
			std::string archivePath = {},
			eFileLocation location = eFileLocation::App);

	private:
		void Stop();
		void IoWorker(std::stop_token stopToken);

		std::shared_ptr<FileSystem> m_FileSystem;
		std::string m_DefaultArchivePath;

		std::mutex m_CvMutex;
		std::condition_variable m_QueueCv;

		DoubleBufferedVector<IoReadRequest> m_Requests;
		std::jthread m_IoThread;
	};
}
