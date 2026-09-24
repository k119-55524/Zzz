#pragma once

#include "core/utils/Defines.h"

#if defined(Z_IOS)

#include "core/io/storage/FileSystemBase.h"

namespace zzz::core
{
	/**
	 * @class FileSystemiOS
	 * @brief Специализация файловой системы для платформы iOS.
	 *
	 * @details eFileLocation::App читается из бандла приложения (.app).
	 *          Пользовательские данные пишутся в песочницу приложения (NSApplicationSupportDirectory) через FileSystemBase.
	 */
	class FileSystemiOS final : public FileSystemBase
	{
	public:
		using FileSystemBase::FileSystemBase;
		~FileSystemiOS() = default;
	};
}

#endif // defined(Z_IOS)
