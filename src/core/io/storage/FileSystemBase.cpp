#include "FileSystemBase.h"
#include "logger.h"
#include "core/constants/PackagesConstants.h"
#include "core/utils/macros/MiscMacros.h"
#include "core/utils/ThrowWrappers.h"

#include <system_error>

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



	[[nodiscard]] std::expected<void, std::string> FileSystemBase::DeleteFile(
		eFileLocation location, const std::filesystem::path& relativePath) const noexcept
	{
		if (!IsLocationWritable(location))
			return UNEXPECTED("Попытка удаления файла в защищённой области: {}", ToString(location));

		auto pathRes = ResolvePhysicalPath(location, relativePath);
		if (!pathRes)
			return UNEXPECTED("{}", pathRes.error());

		std::error_code ec;
		if (!std::filesystem::remove(*pathRes, ec) || ec)
			return UNEXPECTED("Не удалось удалить файл '{}': {}", pathRes->string(), ec.message());

		return {};
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

	[[nodiscard]] std::expected<std::filesystem::path, std::string> FileSystemBase::GetGamePackagePath() const noexcept
	{
		return ResolvePhysicalPath(eFileLocation::App, c_GamePackageRelativePath);
	}

	[[nodiscard]] std::expected<std::filesystem::path, std::string> FileSystemBase::GetDataPackagePath() const noexcept
	{
		return ResolvePhysicalPath(eFileLocation::App, c_DataPackageRelativePath);
	}

	[[nodiscard]] std::expected<void, std::string> FileSystemBase::InitializeUserData(
		std::string_view companyName, std::string_view appName)
	{
		Z_CHECK_ONCE_CALL();

		if (!m_Path)
			return UNEXPECTED("Подсистема Path не инициализирована в FileSystemBase.");

		return m_Path->InitializeUserData(companyName, appName);
	}
}
