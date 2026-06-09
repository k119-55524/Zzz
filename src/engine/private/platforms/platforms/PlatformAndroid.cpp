#if defined(Z_ANDROID)

#include "PlatformAndroid.h"
#include "../../platforms/main_loop/MainLoop_Android.h"
#include "../native_view/window/WinAndroid.h"
#include "../../inputs/platforms/InputAndroid.h"

using namespace zzz::engine;

PlatformAndroid::PlatformAndroid(std::string_view appName, std::shared_ptr<PlatformNativeData> platformData) :
	IPlatform(appName, platformData)
{
	ensure(m_PlatformData != nullptr, "Platform data for Android must not be null.");

	ensure(m_PlatformData->activity != nullptr, "android_app->activity is null. Invalid platform data.");
	ensure(m_PlatformData->looper != nullptr, "android_app->looper is null. Invalid platform data.");
}

PlatformAndroid::~PlatformAndroid()
{
}

void PlatformAndroid::InitializeImpl()
{
	android_app* app = m_PlatformData.get();
	if (app)
	{
		app->onAppCmd = PlatformAndroid::OnAppCmd;
		app->onInputEvent = PlatformAndroid::OnInputEvent;
	}
}

void PlatformAndroid::OnAppCmd(struct android_app* app, int32_t cmd)
{
	WinAndroid::MSWinCtx* ctx = reinterpret_cast<WinAndroid::MSWinCtx*>(app->userData);
	if (ctx && ctx->window)
	{
		ctx->window->ProcessAppCmd(cmd);
	}
}

int32_t PlatformAndroid::OnInputEvent(struct android_app* app, AInputEvent* event)
{
	WinAndroid::MSWinCtx* ctx = reinterpret_cast<WinAndroid::MSWinCtx*>(app->userData);
	if (ctx && ctx->input)
	{
		return ctx->input->ProcessMessage({ event }) ? 1 : 0;
	}

	return 0;
}

#endif // defined(Z_ANDROID)
