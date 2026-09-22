
#include <fstream>
#include <system_error>

#include "FileSystemBase.h"

namespace zzz::core
{
	FileSystemBase::FileSystemBase(std::shared_ptr<NativeAppData> nativeData) :
		m_NativeData{ std::move(nativeData) },
		m_Path{ std::make_shared<Path>(m_NativeData) }
	{
	}

	[[nodiscard]] bool FileSystemBase::FileExists(eFileLocation location, const std::filesystem::path& relativePath) const noexcept
	{
		std::error_code ec;
		return std::filesystem::is_regular_file(ResolvePhysicalPath(location, relativePath), ec) && !ec;
	}

	[[nodiscard]] std::expected<std::vector<std::byte>, std::string> FileSystemBase::ReadBytes(
		eFileLocation location, const std::filesystem::path& relativePath, std::size_t offset, std::size_t size) const noexcept
	{
		if (size == 0)
			return std::vector<std::byte>{};

		const auto physicalPath = ResolvePhysicalPath(location, relativePath);

		std::error_code ec;
		const auto fileSize = std::filesystem::file_size(physicalPath, ec);
		if (ec)
			return UNEXPECTED("Не удалось получить размер файла '{}': {}", physicalPath.string(), ec.message());

		if (offset + size > fileSize)
		{
			return UNEXPECTED("Диапазон чтения [{}, {}) выходит за границы файла '{}' (размер: {})",
				offset, offset + size, physicalPath.string(), fileSize);
		}

		std::ifstream file(physicalPath, std::ios::binary);
		if (!file.is_open())
			return UNEXPECTED("Не удалось открыть файл для чтения: '{}'", physicalPath.string());

		if (!file.seekg(static_cast<std::streamoff>(offset), std::ios::beg))
			return UNEXPECTED("Ошибка смещения (seekg) на позицию {} в файле '{}'", offset, physicalPath.string());

		std::vector<std::byte> buffer(size);
		file.read(reinterpret_cast<char*>(buffer.data()), static_cast<std::streamsize>(size));
		if (!file && !file.eof())
			return UNEXPECTED("Ошибка чтения {} байт из файла '{}'", size, physicalPath.string());

		return buffer;
	}

	[[nodiscard]] std::expected<std::vector<std::byte>, std::string> FileSystemBase::ReadAllBytes(
		eFileLocation location, const std::filesystem::path& relativePath) const noexcept
	{
		const auto physicalPath = ResolvePhysicalPath(location, relativePath);

		std::error_code ec;
		const auto fileSize = std::filesystem::file_size(physicalPath, ec);
		if (ec)
			return UNEXPECTED("Не удалось определить размер файла '{}': {}", physicalPath.string(), ec.message());

		if (fileSize == 0)
			return std::vector<std::byte>{};

		std::ifstream file(physicalPath, std::ios::binary);
		if (!file.is_open())
			return UNEXPECTED("Не удалось открыть файл для чтения: '{}'", physicalPath.string());

		std::vector<std::byte> buffer(fileSize);
		file.read(reinterpret_cast<char*>(buffer.data()), static_cast<std::streamsize>(fileSize));
		if (!file && !file.eof())
			return UNEXPECTED("Ошибка чтения данных из файла '{}'", physicalPath.string());

		return buffer;
	}

	std::expected<void, std::string> FileSystemBase::WriteAllBytes(
		eFileLocation location, const std::filesystem::path& relativePath, std::span<const std::byte> bytes) noexcept
	{
		if (!IsLocationWritable(location))
			return UNEXPECTED("Попытка записи в защищённую область: {}", ToString(location));

		if (relativePath.is_absolute() || relativePath.generic_string().find("..") != std::string::npos)
			return UNEXPECTED("Недопустимый путь к файлу: выход за пределы директории запрещён");

		const auto physicalPath = ResolvePhysicalPath(location, relativePath);

		std::error_code ec;
		const auto parentDir = physicalPath.parent_path();
		if (!parentDir.empty())
		{
			std::filesystem::create_directories(parentDir, ec);
			if (ec)
				return UNEXPECTED("Не удалось создать каталог '{}': {}", parentDir.string(), ec.message());
		}

		std::ofstream file(physicalPath, std::ios::binary | std::ios::trunc);
		if (!file.is_open())
			return UNEXPECTED("Не удалось открыть файл для записи: '{}'", physicalPath.string());

		if (!bytes.empty())
		{
			file.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
			if (!file)
				return UNEXPECTED("Ошибка записи в файл '{}'", physicalPath.string());
		}

		return {};
	}
}
