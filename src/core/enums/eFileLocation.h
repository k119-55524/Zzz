#pragma once

#include "math/utils/Types.h"

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
}
