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
		static void OnAppCmd(struct android_app* app, int32_t cmd);
		static int32_t OnInputEvent(struct android_app* app, AInputEvent* event);
	};
}
#endif // defined(Z_ANDROID)
