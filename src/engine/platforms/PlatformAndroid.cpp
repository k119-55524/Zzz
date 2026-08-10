#include "engine/EngineIncludes.h"
#include "Platform.h"
#include "window/WinAndroid.h"
#include "input/InputAndroid.h"
#include "mainloop/MainLoopAndroid.h"

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
	ensure(m_NativeData != nullptr, "Данные платформы для Android не должны быть null.");
	ensure(m_NativeData->activity != nullptr, "android_app->activity равен null. Некорректные данные платформы.");
	ensure(m_NativeData->looper != nullptr, "android_app->looper равен null. Некорректные данные платформы.");

	android_app* app = m_NativeData.get();
	if (app)
	{
		app->onAppCmd = OnAppCmd;
	}
}

void Platform::ShutdownPlatformSpecific()
{
}

PlatformHardwareState Platform::GatherHardwareState() const
{
	PlatformHardwareState state{};
	state.cpu.architecture = "ARM64";
	return state;
}
