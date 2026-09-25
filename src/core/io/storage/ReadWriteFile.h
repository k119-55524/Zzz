#pragma once

#include <span>
#include <mutex>
#include <string>
#include <vector>
#include <cstddef>
#include <expected>
#include <filesystem>

#include "core/utils/Macroses.h"

namespace zzz::core
{
	/**
	 * @brief Изменяемый файл с потокобезопасными операциями чтения и записи.
	 * @details Путь подготавливает и проверяет FileSystem; конструктор проверок не выполняет.
	 *          Хэндл между вызовами не удерживается: файл открывается и закрывается внутри Read/Write.
	 *          Read и Write напрямую выполняют файловые операции. Write с пустыми данными ничего не делает
	 *          (только предупреждение в лог), иначе пишет напрямую в подготовленный файл.
	 *          Ошибки Read/Write возвращаются через std::expected с логированием через UNEXPECTED.
	 */
	class ReadWriteFile final
	{
	public:
		Z_NO_COPY_MOVE(ReadWriteFile);

		explicit ReadWriteFile(const std::filesystem::path& physicalPath);

		[[nodiscard]] std::expected<std::vector<std::byte>, std::string> Read();
		std::expected<void, std::string> Write(std::span<const std::byte> bytes);

	private:
		std::mutex				m_Mutex;
		std::filesystem::path	m_Path;
	};
}
