
#include <unistd.h>

#include "FileAccess.h"

namespace zzz::core
{
	bool FileAccess::CanReadWrite(const std::filesystem::path& filePath) noexcept
	{
		return access(filePath.c_str(), R_OK | W_OK) == 0;
	}

	bool FileAccess::CanCreateIn(const std::filesystem::path& dirPath) noexcept
	{
		// Для создания файла в каталоге нужны права на запись и на вход в каталог
		return access(dirPath.c_str(), W_OK | X_OK) == 0;
	}
}
