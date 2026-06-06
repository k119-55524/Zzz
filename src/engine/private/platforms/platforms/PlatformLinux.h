#pragma once

#if defined(Z_LINUX)

#include "IPlatform.h"

namespace zzz::engine
{
	class PlatformLinux final : public IPlatform
	{
	public:
		PlatformLinux(std::string_view appName, std::shared_ptr<void> platformData = nullptr);
		~PlatformLinux() override;

	private:
		void InitializeImpl() override;
	};
}
#endif // defined(Z_LINUX)