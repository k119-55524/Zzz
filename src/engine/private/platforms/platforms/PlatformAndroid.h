#pragma once

#if defined(Z_ANDROID)

#include "IPlatform.h"
#include <android_native_app_glue.h>

namespace zzz::engine
{
	class PlatformAndroid final : public IPlatform
	{
	public:
		PlatformAndroid(std::string_view appName, std::shared_ptr<PlatformNativeData> platformData = nullptr);
		~PlatformAndroid() override;

	private:
		void InitializeImpl() override;
	};
}
#endif // defined(Z_ANDROID)
