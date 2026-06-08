#pragma once

#if defined(Z_IOS)

#include "IPlatform.h"

namespace zzz::engine
{
	class PlatformiOS final : public IPlatform
	{
	public:
		PlatformiOS(std::string_view appName, std::shared_ptr<PlatformNativeData> platformData = nullptr);
		~PlatformiOS() override;

	private:
		void InitializeImpl() override;
	};
}
#endif // defined(Z_IOS)
