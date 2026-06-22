#include "WinAndroid.h"
#include "../Platform.h"

using namespace zzz::engine;

WinAndroid::WinAndroid(const Platform& platform, const std::shared_ptr<Input> input, WindowCallbacks callbacks) :
	WindowBase(platform, input, std::move(callbacks))
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
		// [Android] Выделена графическая поверхность. Окно готово к отрисовке.
		// Передаем хэндл нативного окна для инициализации Swapchain.
		// TODO: VERIFY_AND_CALL(m_Callbacks.OnSurfaceCreated, app->window);
		break;
	case APP_CMD_TERM_WINDOW:
		DOut("APP_CMD_TERM_WINDOW.");
		// [Android] Окно (и его графическая поверхность) скрыто или уничтожается системой.
		// Вызываем OnSurfaceDestroyed ДО уничтожения, чтобы убить Swapchain.
		VERIFY_AND_CALL(m_Callbacks.OnSurfaceDestroyed);
		
		// Если приложение действительно завершается, можно вызвать OnClose.
		// В Android это также означает, что Activity завершается.
		VERIFY_AND_CALL(m_Callbacks.OnClose);
		break;
	case APP_CMD_WINDOW_RESIZED:
		DOut("APP_CMD_WINDOW_RESIZED.");
		// [Android] Изменился размер окна (например, из-за скрытия системной панели навигации или поворота).
		// TODO: Извлечь новые размеры и передать в OnResize.
		break;
	}
}

