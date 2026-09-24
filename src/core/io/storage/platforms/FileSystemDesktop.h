#pragma once

#include "core/utils/Defines.h"

#if defined(Z_DESKTOP)

#include "core/io/storage/FileSystemBase.h"

namespace zzz::core
{
	/**
	 * @class FileSystemDesktop
	 * @brief Реализация файловой системы для десктопных платформ (Windows, Linux, macOS).
	 *
	 * @details На десктопе все области располагаются на физическом диске
	 *          и полностью реализуются базовым классом FileSystemBase.
	 */
	class FileSystemDesktop final : public FileSystemBase
	{
	public:
		using FileSystemBase::FileSystemBase;
	};
}

#endif // defined(Z_DESKTOP)
