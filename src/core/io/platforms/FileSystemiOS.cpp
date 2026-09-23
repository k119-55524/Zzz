#include "core/utils/Defines.h"

#if defined(Z_IOS)

#include "FileSystemiOS.h"

namespace zzz::core
{
	[[nodiscard]] bool FileSystemiOS::FileExists(eFileLocation location, const std::filesystem::path& relativePath) const noexcept
	{
		if (location == eFileLocation::App)
			return false; // Stub до КП-5

		return FileSystemBase::FileExists(location, relativePath);
	}

	[[nodiscard]] std::expected<std::uintmax_t, std::string> FileSystemiOS::GetFileSize(
		eFileLocation location, const std::filesystem::path& relativePath) const noexcept
	{
		if (location == eFileLocation::App)
			return UNEXPECTED("iOS NSBundle чтение будет реализовано на шаге КП-5.");

		return FileSystemBase::GetFileSize(location, relativePath);
	}

	[[nodiscard]] std::expected<std::vector<std::byte>, std::string> FileSystemiOS::ReadBytes(
		eFileLocation location, const std::filesystem::path& relativePath, std::size_t offset, std::size_t size) const noexcept
	{
		if (location == eFileLocation::App)
			return UNEXPECTED("iOS NSBundle чтение будет реализовано на шаге КП-5.");

		return FileSystemBase::ReadBytes(location, relativePath, offset, size);
	}

	[[nodiscard]] std::expected<std::vector<std::byte>, std::string> FileSystemiOS::ReadAllBytes(
		eFileLocation location, const std::filesystem::path& relativePath) const noexcept
	{
		if (location == eFileLocation::App)
			return UNEXPECTED("iOS NSBundle чтение будет реализовано на шаге КП-5.");

		return FileSystemBase::ReadAllBytes(location, relativePath);
	}

	std::expected<void, std::string> FileSystemiOS::WriteAllBytes(
		eFileLocation location, const std::filesystem::path& relativePath, std::span<const std::byte> bytes) noexcept
	{
		if (location == eFileLocation::App)
			return UNEXPECTED("Запись в eFileLocation::App на iOS запрещена (read-only бандл приложения).");

		return FileSystemBase::WriteAllBytes(location, relativePath, bytes);
	}
}

#endif // defined(Z_IOS)
