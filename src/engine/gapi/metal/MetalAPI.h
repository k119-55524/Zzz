#pragma once

#include "engine/gapi/IGAPI.h"

#if defined(Z_METAL)

namespace zzz::engine
{
	class MetalAPI final : public IGAPI
	{
	public:
		explicit MetalAPI(std::shared_ptr<UserSettingsManager> userSettings);
		~MetalAPI() override;

		void SubmitCommandLists() override;
		void BeginRender() override;
		void EndRender() override;

	protected:
		[[nodiscard]] std::expected<void, std::string> Init() override;
		void WaitForGpu() override;
	};
}

#endif // Z_METAL
