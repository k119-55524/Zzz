#pragma once

#include <string>
#include <string_view>
#include "core/utils/Guid.h"
#include "engine/scene/domain/ILayerDomain.h"

namespace zzz::engine
{
	/**
	 * @class IMVVMDomain
	 * @brief Контракт домена управления декларативными элементами интерфейса (MVVM).
	 */
	class IMVVMDomain : public ILayerDomain
	{
	public:
		~IMVVMDomain() override = default;
	};
}
