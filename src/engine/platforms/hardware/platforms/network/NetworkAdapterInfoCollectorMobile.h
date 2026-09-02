#pragma once

#include "core/utils/Defines.h"

#if defined(Z_MOBILE)

#include <vector>
#include "core/hardware/NetworkAdapterInfo.h"

namespace zzz::engine
{
	/**
	 * @brief Честный стаб для Android/iOS: перечисление сетевых адаптеров отложено до этапа роллаута
	 * мобильных платформ, пока честно пустой список.
	 */
	class NetworkAdapterInfoCollectorMobile final
	{
	public:
		[[nodiscard]] std::vector<::zzz::core::NetworkAdapterInfo> Collect() const
		{
			return {};
		}
	};
}

#endif // defined(Z_MOBILE)
