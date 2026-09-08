#pragma once

#include "engine/scene/domain/IMVVMDomain.h"

namespace zzz::engine
{
	/**
	 * @class MVVMDomain
	 * @brief Базовая заглушка реализации IMVVMDomain (будет наполнена в ZzzGUI MVP).
	 */
	class MVVMDomain final : public IMVVMDomain
	{
	public:
		MVVMDomain() = default;
		~MVVMDomain() override = default;

		Z_NO_COPY_MOVE(MVVMDomain);

		void Update(float /*dt*/) override {}
		void Clear() override {}
	};
}
