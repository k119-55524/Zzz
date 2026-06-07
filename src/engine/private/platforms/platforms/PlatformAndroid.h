#pragma once

#if defined(Z_ANDROID)

#include "IPlatform.h"

namespace zzz::engine
{
	class PlatformAndroid final : public IPlatform
	{
	public:
		PlatformAndroid(std::string_view appName, std::shared_ptr<void> platformData = nullptr);
		~PlatformAndroid() override;

	private:
		void InitializeImpl() override;
	};
}
#endif // defined(Z_ANDROID)
