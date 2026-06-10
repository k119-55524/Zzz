#include <foundation.h>

#include "Platform.h"
#include "headers/Android.h"
#include "window/WinAndroid.h"
#include "input/InputAndroid.h"
#include "config/ConfigAndroid.h"
#include "main_loop/MainLoop_Android.h"

using namespace zzz::engine;

namespace
{
	void OnAppCmd(struct android_app* app, int32_t cmd)
	{
		WinAndroid::AndroidActinityCtx* ctx = reinterpret_cast<WinAndroid::AndroidActinityCtx*>(app->userData);
		if (ctx && ctx->window)
		{
			ctx->window->ProcessAppCmd(cmd);
		}
	}
}

namespace zzz::engine
{
	void ProcessAndroidInput(struct android_app* app)
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
}

void Platform::InitializePlatformSpecific()
{
	ensure(m_NativeData != nullptr, "Platform data for Android must not be null.");
	ensure(m_NativeData->activity != nullptr, "android_app->activity is null. Invalid platform data.");
	ensure(m_NativeData->looper != nullptr, "android_app->looper is null. Invalid platform data.");

	android_app* app = m_NativeData.get();
	if (app)
	{
		app->onAppCmd = OnAppCmd;
	}
}

void Platform::ShutdownPlatformSpecific()
{
}
