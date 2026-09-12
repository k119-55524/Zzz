#pragma once

#include "core/utils/macros/MiscMacros.h"

namespace zzz::engine
{
	/**
	 * @class MVVMDomain
	 * @brief Базовая заглушка реализации домена MVVM (будет наполнена в ZzzGUI MVP).
	 */
	class MVVMDomain final
	{
		Z_NO_COPY_MOVE(MVVMDomain);

	public:
		MVVMDomain() = default;
		~MVVMDomain() = default;
	};
}
