
#include <format>
#include <cstdint>
#include <fstream>
#include <system_error>

#include "core/utils/SafeMath.h"

#include "ReadWriteFile.h"

namespace zzz::core
{
	ReadWriteFile::ReadWriteFile(const std::filesystem::path& physicalPath)
		: m_Mutex{},
		m_Path{ physicalPath }
	{
	}

	std::expected<std::vector<std::byte>, std::string> ReadWriteFile::Read()
	{
		std::lock_guard<std::mutex> lock(m_Mutex);

		const auto status = GetStatus(m_Path);
		if (!status)
			return std::unexpected(status.error());

		if (!std::filesystem::exists(*status))
			return UNEXPECTED("Файл '{}' не существует.", m_Path.string());

		if (!std::filesystem::is_regular_file(*status))
			return UNEXPECTED("Путь '{}' не является обычным файлом.", m_Path.string());

		// Открытие на чтение и запись одновременно проверяет оба права
		std::fstream stream(m_Path, std::ios::in | std::ios::out | std::ios::binary | std::ios::ate);
		if (!stream.is_open())
			return UNEXPECTED("Нет прав на чтение и запись файла '{}'.", m_Path.string());

		const auto streamEnd = stream.tellg();
		if (streamEnd < 0)
			return UNEXPECTED("Не удалось определить размер файла '{}'", m_Path.string());

		const auto size = NarrowTo<std::size_t>(static_cast<std::uint64_t>(streamEnd));
		if (!size)
			return UNEXPECTED("Размер файла '{}' не представим типом std::size_t", m_Path.string());

		if (*size == 0)
			return std::vector<std::byte>{};

		stream.seekg(0, std::ios::beg);
		std::vector<std::byte> buffer(*size);
		stream.read(reinterpret_cast<char*>(buffer.data()), static_cast<std::streamsize>(*size));

		const auto readCount = static_cast<std::size_t>(stream.gcount());
		if (readCount != *size)
			return UNEXPECTED("Ошибка чтения: прочитано {} из {} байт файла '{}'", readCount, *size, m_Path.string());

		return buffer;
	}

	std::expected<void, std::string> ReadWriteFile::Write(std::span<const std::byte> bytes)
	{
		if (bytes.empty())
		{
			DOutWarning("Запись пустого массива данных в файл '{}' пропущена.", m_Path.string());
			return {};
		}

		std::lock_guard<std::mutex> lock(m_Mutex);

		const auto status = GetStatus(m_Path);
		if (!status)
			return std::unexpected(status.error());

		const bool existed = std::filesystem::exists(*status);
		std::vector<std::filesystem::path> createdDirs;

		if (existed)
		{
			if (auto res = CheckExistingFile(m_Path, *status); !res)
				return std::unexpected(res.error());
		}
		else
		{
			auto missingDirs = CollectMissingDirs(m_Path);
			if (!missingDirs)
				return std::unexpected(missingDirs.error());

			createdDirs = std::move(*missingDirs);
		}

		// Откат изменений на диске при любом выходе до успешной записи, включая исключения
		struct Rollback
		{
			const std::filesystem::path&				createdFile;
			const std::vector<std::filesystem::path>&	createdDirs;
			bool										armed = true;

			~Rollback()
			{
				if (!armed)
					return;

				std::error_code ec;
				if (!createdFile.empty())
					std::filesystem::remove(createdFile, ec);

				for (const auto& dir : createdDirs)
					std::filesystem::remove(dir, ec);
			}
		};

		const std::filesystem::path createdFile = existed ? std::filesystem::path{} : m_Path;
		Rollback rollback{ createdFile, createdDirs };

		if (!createdDirs.empty())
		{
			std::error_code ec;
			std::filesystem::create_directories(createdDirs.front(), ec);
			if (ec)
				return UNEXPECTED("Не удалось создать каталог '{}': {}", createdDirs.front().string(), ec.message());
		}

		{
			std::ofstream stream(m_Path, std::ios::out | std::ios::binary | std::ios::trunc);
			if (!stream.is_open())
				return UNEXPECTED("Не удалось открыть файл '{}' для записи.", m_Path.string());

			stream.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));

			stream.close();
			if (stream.fail())
				return UNEXPECTED("Ошибка записи данных в файл '{}'", m_Path.string());
		}

		rollback.armed = false;
		return {};
	}

#pragma region Validation helpers
	std::expected<std::filesystem::file_status, std::string> ReadWriteFile::GetStatus(const std::filesystem::path& path)
	{
		std::error_code ec;
		const auto status = std::filesystem::status(path, ec);
		if (ec && ec != std::errc::no_such_file_or_directory)
			return UNEXPECTED("Не удалось получить статус пути '{}': {}", path.string(), ec.message());

		return status;
	}

	std::expected<void, std::string> ReadWriteFile::CheckExistingFile(
		const std::filesystem::path& path, const std::filesystem::file_status& status)
	{
		if (!std::filesystem::is_regular_file(status))
			return UNEXPECTED("Путь '{}' не является обычным файлом.", path.string());

		std::fstream stream(path, std::ios::in | std::ios::out | std::ios::binary);
		if (!stream.is_open())
			return UNEXPECTED("Нет прав на чтение и запись файла '{}'.", path.string());

		return {};
	}

	std::expected<std::vector<std::filesystem::path>, std::string> ReadWriteFile::CollectMissingDirs(const std::filesystem::path& path)
	{
		std::vector<std::filesystem::path> missing;
		auto dir = path.parent_path();
		while (!dir.empty())
		{
			const auto status = GetStatus(dir);
			if (!status)
				return std::unexpected(status.error());

			if (std::filesystem::exists(*status))
			{
				if (!std::filesystem::is_directory(*status))
					return UNEXPECTED("Путь '{}' не является каталогом.", dir.string());

				break;
			}

			missing.push_back(dir);

			const auto parent = dir.parent_path();
			if (parent == dir)
				break;

			dir = parent;
		}

		return missing;
	}
#pragma endregion // Validation helpers
}
