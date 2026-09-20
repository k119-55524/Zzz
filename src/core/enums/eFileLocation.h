#pragma once

#include <string_view>
#include "math/utils/Types.h"
#include "core/utils/ThrowWrappers.h"

namespace zzz::core
{
	/**
	 * @enum eFileLocation
	 * @brief Логическая область размещения файлов в приложении (1 байт).
	 *
	 * @details Служит строго типизированным ключом адресации без накладных расходов на парсинг строк.
	 */
	enum class eFileLocation : zU8
	{
		App = 0,    ///< Ресурсы приложения / папка установки игры (Read-Only на Android/iOS, Read/Write в папке .exe на ПК)
		User,       ///< Пользовательские данные / конфиги (Read-Write: LOCALAPPDATA / internalDataPath)
		Saves,      ///< Папка сохранений игры (Read-Write)
		Cache,      ///< Папка временного кэша (Read-Write)
		Logs        ///< Папка лог-файлов (Read-Write)
	};

	constexpr std::string_view ToString(eFileLocation location)
	{
		switch (location)
		{
		case eFileLocation::App:   return "App";
		case eFileLocation::User:  return "User";
		case eFileLocation::Saves: return "Saves";
		case eFileLocation::Cache: return "Cache";
		case eFileLocation::Logs:  return "Logs";
		}
		THROW_RUNTIME("Необработанный eFileLocation");
	}

	[[nodiscard]] constexpr bool IsLocationWritable(eFileLocation location) noexcept
	{
		return location != eFileLocation::App;
	}
}
