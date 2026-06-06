#pragma once

#if defined(Z_WINDOWS)

#include "IPlatform.h"

namespace zzz::engine
{
	class PlatformMSWindows final : public IPlatform
	{
	public:
		PlatformMSWindows(std::string_view appName, std::shared_ptr<void> platformData = nullptr);
		~PlatformMSWindows() override;

	private:
		void InitializeImpl() override;
	};
}
#endif // defined(Z_WINDOWS)