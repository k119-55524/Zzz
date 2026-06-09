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
	}
}

void PlatformAndroid::OnAppCmd(struct android_app* app, int32_t cmd)
{
	WinAndroid::AndroidActinityCtx* ctx = reinterpret_cast<WinAndroid::AndroidActinityCtx*>(app->userData);
	if (ctx && ctx->window)
	{
		ctx->window->ProcessAppCmd(cmd);
	}
}

void PlatformAndroid::ProcessInput(struct android_app* app)
{
	if (!app) return;

	auto* inputBuffer = android_app_swap_input_buffers(app);
	if (!inputBuffer) return;

	WinAndroid::AndroidActinityCtx* ctx = reinterpret_cast<WinAndroid::AndroidActinityCtx*>(app->userData);
	if (ctx && ctx->input)
	{
		for (uint64_t i = 0; i < inputBuffer->motionEventsCount; ++i)
		{
			AndroidMsg msg = { &inputBuffer->motionEvents[i], nullptr };
			ctx->input->ProcessMessage(msg);
		}
		for (uint64_t i = 0; i < inputBuffer->keyEventsCount; ++i)
		{
			AndroidMsg msg = { nullptr, &inputBuffer->keyEvents[i] };
			ctx->input->ProcessMessage(msg);
		}
	}

	android_app_clear_motion_events(inputBuffer);
	android_app_clear_key_events(inputBuffer);
}

#endif // defined(Z_ANDROID)
