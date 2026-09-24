#include "core/headers/platforms/MSWin.h"

#include "FileAccess.h"

namespace zzz::core
{
	namespace
	{
		/// @brief Открывает хэндл с запрошенными правами и сразу закрывает его. Содержимое не изменяется.
		[[nodiscard]] bool TryOpen(const std::filesystem::path& path, DWORD access, DWORD flags) noexcept
		{
			const HANDLE handle = CreateFileW(
				path.c_str(),
				access,
				FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
				nullptr,
				OPEN_EXISTING,
				flags,
				nullptr);

			if (handle == INVALID_HANDLE_VALUE)
				return false;

			CloseHandle(handle);
			return true;
		}
	}

	bool FileAccess::CanReadWrite(const std::filesystem::path& filePath) noexcept
	{
		return TryOpen(filePath, GENERIC_READ | GENERIC_WRITE, FILE_ATTRIBUTE_NORMAL);
	}

	bool FileAccess::CanCreateIn(const std::filesystem::path& dirPath) noexcept
	{
		// FILE_FLAG_BACKUP_SEMANTICS обязателен для открытия каталога
		return TryOpen(dirPath, FILE_ADD_FILE, FILE_FLAG_BACKUP_SEMANTICS);
	}
}
