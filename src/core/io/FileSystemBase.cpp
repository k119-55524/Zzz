
#include <fstream>
#include <system_error>

#include "FileSystemBase.h"
#include "core/utils/Ensure.h"

namespace zzz::core
{
	FileSystemBase::FileSystemBase(std::shared_ptr<NativeAppData> nativeData) :
		m_NativeData{ std::move(nativeData) },
		m_Path{ std::make_shared<Path>(m_NativeData) }
	{
	}

	[[nodiscard]] std::expected<std::filesystem::path, std::string> FileSystemBase::ResolvePhysicalPath(
		eFileLocation location, std::string_view relativePath) const noexcept
	{
		try
		{
			if (location == eFileLocation::App)
			{
				return m_Path->GetExecutableDirectory() / relativePath;
			}

			ensure(!m_Path->GetUserDataDirectory().empty(),
				"Каталог пользовательских данных не инициализирован. Вызовите InitializeUserData() перед обращением к пользовательским директориям.");

			switch (location)
			{
			case eFileLocation::User:
				return m_Path->GetUserDataDirectory() / relativePath;

			case eFileLocation::Saves:
				return m_Path->GetDirectory(eUserDirectoryKind::Saves) / relativePath;

			case eFileLocation::Cache:
				return m_Path->GetDirectory(eUserDirectoryKind::Cache) / relativePath;

			case eFileLocation::Logs:
				return m_Path->GetDirectory(eUserDirectoryKind::Logs) / relativePath;

			default:
				return UNEXPECTED("Неизвестная логическая область размещения файла: {}", static_cast<int>(location));
			}
		}
		catch (const std::exception& e)
		{
			return UNEXPECTED("Ошибка формирования пути: {}", e.what());
		}
	}

	[[nodiscard]] bool FileSystemBase::FileExists(eFileLocation location, std::string_view relativePath) const noexcept
	{
		auto resolved = ResolvePhysicalPath(location, relativePath);
		if (!resolved)
			return false;

		std::error_code ec;
		return std::filesystem::is_regular_file(*resolved, ec) && !ec;
	}

	[[nodiscard]] std::expected<std::vector<std::byte>, std::string> FileSystemBase::ReadBytes(
		eFileLocation location, std::string_view relativePath, std::size_t offset, std::size_t size) const noexcept
	{
		if (size == 0)
			return std::vector<std::byte>{};

		auto resolved = ResolvePhysicalPath(location, relativePath);
		if (!resolved)
			return UNEXPECTED("{}", resolved.error());

		std::error_code ec;
		const auto fileSize = std::filesystem::file_size(*resolved, ec);
		if (ec)
			return UNEXPECTED("Не удалось получить размер файла '{}': {}", resolved->string(), ec.message());

		if (offset + size > fileSize)
		{
			return UNEXPECTED("Диапазон чтения [{}, {}) выходит за границы файла '{}' (размер: {})",
				offset, offset + size, resolved->string(), fileSize);
		}

		std::ifstream file(*resolved, std::ios::binary);
		if (!file.is_open())
			return UNEXPECTED("Не удалось открыть файл для чтения: '{}'", resolved->string());

		if (!file.seekg(static_cast<std::streamoff>(offset), std::ios::beg))
			return UNEXPECTED("Ошибка смещения (seekg) на позицию {} в файле '{}'", offset, resolved->string());

		std::vector<std::byte> buffer(size);
		file.read(reinterpret_cast<char*>(buffer.data()), static_cast<std::streamsize>(size));
		if (!file && !file.eof())
			return UNEXPECTED("Ошибка чтения {} байт из файла '{}'", size, resolved->string());

		return buffer;
	}

	[[nodiscard]] std::expected<std::vector<std::byte>, std::string> FileSystemBase::ReadAllBytes(
		eFileLocation location, std::string_view relativePath) const noexcept
	{
		auto resolved = ResolvePhysicalPath(location, relativePath);
		if (!resolved)
			return UNEXPECTED("{}", resolved.error());

		std::error_code ec;
		const auto fileSize = std::filesystem::file_size(*resolved, ec);
		if (ec)
			return UNEXPECTED("Не удалось определить размер файла '{}': {}", resolved->string(), ec.message());

		if (fileSize == 0)
			return std::vector<std::byte>{};

		std::ifstream file(*resolved, std::ios::binary);
		if (!file.is_open())
			return UNEXPECTED("Не удалось открыть файл для чтения: '{}'", resolved->string());

		std::vector<std::byte> buffer(fileSize);
		file.read(reinterpret_cast<char*>(buffer.data()), static_cast<std::streamsize>(fileSize));
		if (!file && !file.eof())
			return UNEXPECTED("Ошибка чтения данных из файла '{}'", resolved->string());

		return buffer;
	}

	std::expected<void, std::string> FileSystemBase::WriteAllBytes(
		eFileLocation location, std::string_view relativePath, std::span<const std::byte> bytes) noexcept
	{
		if (!IsLocationWritable(location))
			return UNEXPECTED("Попытка записи в защищённую область: {}", ToString(location));

		const std::filesystem::path p{ relativePath };
		if (p.is_absolute() || relativePath.find("..") != std::string_view::npos)
			return UNEXPECTED("Недопустимый путь к файлу: выход за пределы директории запрещён");

		auto resolved = ResolvePhysicalPath(location, relativePath);
		if (!resolved)
			return UNEXPECTED("{}", resolved.error());

		std::error_code ec;
		const auto parentDir = resolved->parent_path();
		if (!parentDir.empty())
		{
			std::filesystem::create_directories(parentDir, ec);
			if (ec)
				return UNEXPECTED("Не удалось создать каталог '{}': {}", parentDir.string(), ec.message());
		}

		std::ofstream file(*resolved, std::ios::binary | std::ios::trunc);
		if (!file.is_open())
			return UNEXPECTED("Не удалось открыть файл для записи: '{}'", resolved->string());

		if (!bytes.empty())
		{
			file.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
			if (!file)
				return UNEXPECTED("Ошибка записи в файл '{}'", resolved->string());
		}

		return {};
	}
}
