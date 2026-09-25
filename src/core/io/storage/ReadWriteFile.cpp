
#include <cstdint>
#include <fstream>

#include "core/utils/SafeMath.h"

#include "ReadWriteFile.h"

namespace zzz::core
{
	ReadWriteFile::ReadWriteFile(const std::filesystem::path& physicalPath)
		: m_Path{ physicalPath }
	{
	}

	std::expected<std::vector<std::byte>, std::string> ReadWriteFile::Read()
	{
		std::lock_guard<std::mutex> lock(m_Mutex);

		std::ifstream stream(m_Path, std::ios::in | std::ios::binary | std::ios::ate);
		if (!stream.is_open())
			return UNEXPECTED("Не удалось открыть файл '{}' для чтения.", m_Path.string());

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

		std::ofstream stream(m_Path, std::ios::out | std::ios::binary | std::ios::trunc);
		if (!stream.is_open())
			return UNEXPECTED("Не удалось открыть файл '{}' для записи.", m_Path.string());

		stream.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));

		stream.close();
		if (stream.fail())
			return UNEXPECTED("Ошибка записи данных в файл '{}'.", m_Path.string());

		return {};
	}
}
