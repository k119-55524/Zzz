#pragma once

#include "core/utils/Defines.h"

#if defined(Z_MOBILE)

#include "core/hardware/MotherboardInfo.h"

namespace zzz::engine
{
	/**
	 * @brief Честный стаб для Android/iOS: понятие "материнская плата" неприменимо к мобильным
	 * устройствам как таковое (по прецеденту MonitorProviderMobile - единая реализация на Z_MOBILE).
	 */
	class MotherboardInfoCollectorMobile final
	{
	public:
		[[nodiscard]] ::zzz::core::MotherboardInfo Collect() const
		{
			using namespace zzz::core;
			return MotherboardInfo("Unknown", "Unknown", "");
		}
	};
}

#endif // defined(Z_MOBILE)
