#pragma once

#include "engine/gapi/IGAPI.h"

#if defined(Z_METAL)

namespace zzz::engine
{
	class MetalAPI final : public IGAPI
	{
	public:
		explicit MetalAPI() = default;
		~MetalAPI() override;

		void WaitForGpu() override;
		void Initialize(std::shared_ptr<UserSettingsManager> userSettings) override;

		template<typename T = void>
		void RegisterPendingTransition([[maybe_unused]] T* resource = nullptr) noexcept {}

		template<typename T = void>
		void FlushPendingTransitions([[maybe_unused]] T* cmdList = nullptr) noexcept {}
	};
}

#endif // Z_METAL
