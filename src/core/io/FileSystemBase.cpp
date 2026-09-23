
#include <fstream>
#include <limits>
#include <system_error>

#include "FileSystemBase.h"
#include "core/utils/SafeRange.h"

namespace zzz::core
{
	FileSystemBase::FileSystemBase(std::shared_ptr<NativeAppData> nativeData) :
		m_NativeData{ std::move(nativeData) },
		m_Path{ std::make_shared<Path>(m_NativeData) }
	{
	}

	[[nodiscard]] bool FileSystemBase::FileExists(eFileLocation location, const std::filesystem::path& relativePath) const noexcept
	{
		auto pathRes = ResolvePhysicalPath(location, relativePath);
		if (!pathRes)
			return false;

		std::error_code ec;
		return std::filesystem::is_regular_file(*pathRes, ec) && !ec;
	}

	[[nodiscard]] std::expected<std::uintmax_t, std::string> FileSystemBase::GetFileSize(
		eFileLocation location, const std::filesystem::path& relativePath) const noexcept
	{
		auto pathRes = ResolvePhysicalPath(location, relativePath);
		if (!pathRes)
			return UNEXPECTED("{}", pathRes.error());

		std::error_code ec;
		const auto fileSize = std::filesystem::file_size(*pathRes, ec);
		if (ec)
			return UNEXPECTED("Не удалось получить размер файла '{}': {}", pathRes->string(), ec.message());

		return fileSize;
	}

	[[nodiscard]] std::expected<std::vector<std::byte>, std::string> FileSystemBase::ReadBytes(
		eFileLocation location, const std::filesystem::path& relativePath, std::size_t offset, std::size_t size) const noexcept
	{
		auto pathRes = ResolvePhysicalPath(location, relativePath);
		if (!pathRes)
			return UNEXPECTED("{}", pathRes.error());

		const auto& physicalPath = *pathRes;

		std::error_code ec;
		const auto fileSize = std::filesystem::file_size(physicalPath, ec);
		if (ec)
			return UNEXPECTED("Не удалось получить размер файла '{}': {}", physicalPath.string(), ec.message());

		if (!IsRangeInside<std::uintmax_t>(offset, size, fileSize))
		{
			return UNEXPECTED("Диапазон чтения (offset={}, size={}) выходит за границы файла '{}' (размер: {})",
				offset, size, physicalPath.string(), fileSize);
		}

		const auto streamOffset = NarrowTo<std::streamoff>(offset);
		if (!streamOffset)
			return UNEXPECTED("Смещение {} не представимо типом std::streamoff", offset);

		const auto streamSize = NarrowTo<std::streamsize>(size);
		if (!streamSize)
			return UNEXPECTED("Размер чтения {} не представим типом std::streamsize", size);

		std::ifstream file(physicalPath, std::ios::binary);
		if (!file.is_open())
			return UNEXPECTED("Не удалось открыть файл для чтения: '{}'", physicalPath.string());

		if (size == 0)
			return std::vector<std::byte>{};

		if (!file.seekg(*streamOffset, std::ios::beg))
			return UNEXPECTED("Ошибка смещения (seekg) на позицию {} в файле '{}'", offset, physicalPath.string());

		std::vector<std::byte> buffer(size);
		file.read(reinterpret_cast<char*>(buffer.data()), *streamSize);
		if (file.gcount() != *streamSize)
			return UNEXPECTED("Прочитано {} из {} байт файла '{}'", file.gcount(), size, physicalPath.string());

		return buffer;
	}

	[[nodiscard]] std::expected<std::vector<std::byte>, std::string> FileSystemBase::ReadAllBytes(
		eFileLocation location, const std::filesystem::path& relativePath) const noexcept
	{
		auto pathRes = ResolvePhysicalPath(location, relativePath);
		if (!pathRes)
			return UNEXPECTED("{}", pathRes.error());

		const auto& physicalPath = *pathRes;

		std::error_code ec;
		const auto fileSize = std::filesystem::file_size(physicalPath, ec);
		if (ec)
			return UNEXPECTED("Не удалось определить размер файла '{}': {}", physicalPath.string(), ec.message());

		if (fileSize == 0)
			return std::vector<std::byte>{};

		const auto bufferSize = NarrowTo<std::size_t>(fileSize);
		if (!bufferSize)
			return UNEXPECTED("Размер файла '{}' не представим типом std::size_t: {}", physicalPath.string(), fileSize);

		const auto streamSize = NarrowTo<std::streamsize>(fileSize);
		if (!streamSize)
			return UNEXPECTED("Размер файла '{}' не представим типом std::streamsize: {}", physicalPath.string(), fileSize);

		std::ifstream file(physicalPath, std::ios::binary);
		if (!file.is_open())
			return UNEXPECTED("Не удалось открыть файл для чтения: '{}'", physicalPath.string());

		std::vector<std::byte> buffer(*bufferSize);
		file.read(reinterpret_cast<char*>(buffer.data()), *streamSize);
		if (file.gcount() != *streamSize)
			return UNEXPECTED("Прочитано {} из {} байт файла '{}'", file.gcount(), fileSize, physicalPath.string());

		return buffer;
	}

	std::expected<void, std::string> FileSystemBase::WriteAllBytes(
		eFileLocation location, const std::filesystem::path& relativePath, std::span<const std::byte> bytes) noexcept
	{
		if (!IsLocationWritable(location))
			return UNEXPECTED("Попытка записи в защищённую область: {}", ToString(location));

		if (relativePath.is_absolute() || relativePath.generic_string().find("..") != std::string::npos)
			return UNEXPECTED("Недопустимый путь к файлу: выход за пределы директории запрещён");

		auto pathRes = ResolvePhysicalPath(location, relativePath);
		if (!pathRes)
			return UNEXPECTED("{}", pathRes.error());

		const auto& physicalPath = *pathRes;

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
