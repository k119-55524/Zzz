#pragma once

#if defined(Z_MACOS)

#include "IPlatform.h"

namespace zzz::engine
{
	class PlatformMacOS final : public IPlatform
	{
	public:
		PlatformMacOS(std::string_view appName, std::shared_ptr<PlatformNativeData> platformData = nullptr);
		~PlatformMacOS() override;

	private:
		void InitializeImpl() override;
	};
}
#endif // defined(Z_MACOS)
