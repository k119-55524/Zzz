#pragma once

#include "core/utils/Defines.h"

#if defined(Z_MOBILE)

#include <vector>
#include "core/hardware/StorageInfo.h"

namespace zzz::engine
{
	/**
	 * @brief Честный стаб для Android/iOS: реальная статистика песочницы приложения (statfs по
	 * internalDataPath / NSHomeDirectory) требует доступа к NativeAppData, который сейчас не прокидывается
	 * в коллекторы (чтобы HardwareManager оставался полностью без #ifdef и без платформенных зависимостей
	 * в конструкторе). Отложено до этапа роллаута мобильных платформ, пока честно пустой список.
	 */
	class StorageInfoCollectorMobile final
	{
	public:
		[[nodiscard]] std::vector<::zzz::core::StorageInfo> Collect() const
		{
			return {};
		}
	};
}

#endif // defined(Z_MOBILE)
