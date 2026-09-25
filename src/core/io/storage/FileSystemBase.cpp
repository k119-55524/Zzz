#include <system_error>

#include "logger.h"
#include "core/utils/ThrowWrappers.h"
#include "core/utils/macros/MiscMacros.h"
#include "core/constants/PackagesConstants.h"
#include "platforms/file_access/FileAccess.h"

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
		auto pathRes = ResolvePhysicalPath(location, relativePath);
		if (!pathRes)
			return false;

		std::error_code ec;
		return std::filesystem::is_regular_file(*pathRes, ec) && !ec;
	}

	[[nodiscard]] std::expected<std::filesystem::path, std::string> FileSystemBase::ResolvePhysicalPath(
		eFileLocation location, const std::filesystem::path& relativePath) const noexcept
	{
		if (!m_Path)
			return UNEXPECTED("Подсистема Path не инициализирована в FileSystemBase.");

		auto dirRes = m_Path->GetDirectory(location);
		if (!dirRes)
			return std::unexpected(std::move(dirRes.error()));

		return *dirRes / relativePath;
	}

	[[nodiscard]] std::expected<void, std::string> FileSystemBase::ValidateFilePath(const std::filesystem::path& physicalPath) const noexcept
	{
		std::error_code ec;
		const auto status = std::filesystem::status(physicalPath, ec);
		if (ec && ec != std::errc::no_such_file_or_directory)
			return UNEXPECTED("Не удалось получить статус пути '{}': {}", physicalPath.string(), ec.message());

		if (std::filesystem::exists(status))
		{
			if (!std::filesystem::is_regular_file(status))
				return UNEXPECTED("Путь '{}' не является обычным файлом.", physicalPath.string());

			if (!FileAccess::CanReadWrite(physicalPath))
				return UNEXPECTED("Нет прав на чтение и запись файла '{}'.", physicalPath.string());

			return {};
		}

		const auto directory = physicalPath.parent_path();
		const auto directoryStatus = std::filesystem::status(directory, ec);
		if (ec)
			return UNEXPECTED("Не удалось получить статус каталога '{}': {}", directory.string(), ec.message());

		if (!std::filesystem::is_directory(directoryStatus))
			return UNEXPECTED("Путь '{}' не является каталогом.", directory.string());

		if (!FileAccess::CanCreateIn(directory))
			return UNEXPECTED("Нет прав на создание файла '{}' в каталоге '{}'.", physicalPath.string(), directory.string());

		return {};
	}

	[[nodiscard]] std::expected<std::filesystem::path, std::string> FileSystemBase::GetGamePackagePath() const noexcept
	{
		auto pathRes = ResolvePhysicalPath(eFileLocation::App, c_GamePackageRelativePath);
		if (!pathRes)
			return std::unexpected(std::move(pathRes.error()));

		if (!FileExists(eFileLocation::App, c_GamePackageRelativePath))
			return UNEXPECTED("Файл главного пакета '{}' не найден.", pathRes->string());

		return pathRes;
	}

	[[nodiscard]] std::expected<std::filesystem::path, std::string> FileSystemBase::GetDataPackagePath() const noexcept
	{
		auto pathRes = ResolvePhysicalPath(eFileLocation::App, c_DataPackageRelativePath);
		if (!pathRes)
			return std::unexpected(std::move(pathRes.error()));

		if (!FileExists(eFileLocation::App, c_DataPackageRelativePath))
			return UNEXPECTED("Файл пакета данных '{}' не найден.", pathRes->string());

		return pathRes;
	}

	[[nodiscard]] std::expected<std::filesystem::path, std::string> FileSystemBase::GetUserConfigPath(
		std::string_view companyName, std::string_view appName)
	{
		Z_CHECK_ONCE_CALL();

		if (!m_Path)
			return UNEXPECTED("Подсистема Path не инициализирована в FileSystemBase.");

		if (auto res = m_Path->InitializeUserData(companyName, appName); !res)
			return std::unexpected(std::move(res.error()));

		auto pathRes = ResolvePhysicalPath(eFileLocation::User, c_UserConfigFileName);
		if (!pathRes)
			return std::unexpected(std::move(pathRes.error()));

		std::error_code ec;
		const auto directory = pathRes->parent_path();
		std::filesystem::create_directories(directory, ec);
		if (ec)
			return UNEXPECTED("Не удалось создать каталог пользовательских данных '{}': {}", directory.string(), ec.message());

		if (auto res = ValidateFilePath(*pathRes); !res)
			return std::unexpected(std::move(res.error()));

		return pathRes;
	}
}
