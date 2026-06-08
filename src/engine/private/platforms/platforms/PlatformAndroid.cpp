#if defined(Z_ANDROID)

#include "PlatformAndroid.h"
#include "../../platforms/main_loop/MainLoop_Android.h"

using namespace zzz::engine;

PlatformAndroid::PlatformAndroid(std::string_view appName, std::shared_ptr<PlatformNativeData> platformData) :
	IPlatform(appName, platformData)
{
	ensure(m_PlatformData != nullptr, "Platform data for Android must not be null.");
}

PlatformAndroid::~PlatformAndroid()
{
}

void PlatformAndroid::InitializeImpl()
{
	android_app* app = GetNativeApp();
	if (app)
	{
		// Мы можем установить обработчик команд здесь или в Engine
		// app->onAppCmd = ...
	}
}

#endif // defined(Z_ANDROID)
