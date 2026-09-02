#pragma once

#include "core/utils/Defines.h"

#if defined(Z_DESKTOP)

#include "core/io/FileSystemBase.h"

namespace zzz::core
{
	/**
	 * @class FileSystemDesktop
	 * @brief Реализация файловой системы для десктопных платформ (Windows, Linux, macOS).
	 *
	 * @details На десктопе все области, включая App, располагаются на физическом диске
	 *          и полностью реализуются базовым классом FileSystemBase.
	 */
	class FileSystemDesktop final : public FileSystemBase
	{
	public:
		using FileSystemBase::FileSystemBase;
	};
}

#endif // defined(Z_DESKTOP)
