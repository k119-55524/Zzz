
#include "WinAndroid.h"
#include "../Platform.h"

using namespace zzz::engine;

WinAndroid::WinAndroid(const std::shared_ptr<Platform> platform, const std::shared_ptr<Input> input) :
	WindowBase(platform, input)
{
}

WinAndroid::~WinAndroid()
{
}

std::expected<void, std::string> WinAndroid::Initialize(const std::string_view appName)
{
	m_Ctx = { this, m_Input.get() };
	android_app* app = m_Platform->GetNativeData().get();
	if (app)
	{
		app->userData = &m_Ctx;
	}

	return {};
}

void WinAndroid::ProcessAppCmd(int32_t cmd)
{
	switch (cmd)
	{
	case APP_CMD_INIT_WINDOW:
		DOut("APP_CMD_INIT_WINDOW.");
		break;
	case APP_CMD_TERM_WINDOW:
		DOut("APP_CMD_TERM_WINDOW.");
		onCloseRequested();
		break;
	case APP_CMD_WINDOW_RESIZED:
		DOut("APP_CMD_WINDOW_RESIZED.");
		break;
	}
}

