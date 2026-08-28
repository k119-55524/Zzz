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

	private:
		friend class Engine;
		void Initialize(std::shared_ptr<UserSettingsManager> userSettings) override;
	};
}

#endif // Z_METAL
