#pragma once

#if defined(Z_ANDROID)

#include "IPlatform.h"
#include "headers/Android.h"

namespace zzz::engine
{
	class PlatformAndroid final : public IPlatform
	{
	public:
		PlatformAndroid(std::string_view appName, std::shared_ptr<PlatformNativeData> platformData = nullptr);
		~PlatformAndroid() override;
	public:
		static void ProcessInput(struct android_app* app);

	private:
		void InitializeImpl() override;
		static void OnAppCmd(struct android_app* app, int32_t cmd);
	};
}
#endif // defined(Z_ANDROID)
