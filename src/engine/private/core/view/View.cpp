#include "View.h"
#include "../../platforms/input/Input.h"
#include "../../platforms/window/Window.h"
#include <common/common.h>

using namespace zzz::common;

using namespace zzz::engine;

View::View(const Platform& platform, std::function<void(View&)> onWindowClose) :
	m_Platform{ platform },
	OnWindowClose(std::move(onWindowClose))
{
	ensure(OnWindowClose != nullptr, "OnWindowClose must not be null.");

	Initialize(nullptr);
}

#if Z_EDITOR
View::View(const Platform& platform, void* data) :
	m_Platform{ platform }
{
	Initialize(data);
}
#endif

void View::Initialize(void* data)
{
	m_Input = safe_make_shared<Input>();
	auto inputRes = m_Input->Initialize();
	if (!inputRes)
		THROW_RUNTIME("Failed to initialize input system: {}.", inputRes.error());

	WindowCallbacks callbacks;
	callbacks.OnClose            = [this]()                                 { HandleWindowClose(); };
	callbacks.OnResize           = [this](Size2D<>& size, eWinResize type)  { OnWindowResize(size, type); };
	callbacks.OnResizeStart      = [this]()                                 { OnWindowResizeStart(); };
	callbacks.OnSizing           = [this]()                                 { OnWindowSizing(); };
	callbacks.OnResizeEnd        = [this]()                                 { OnWindowResizeEnd(); };
	callbacks.OnDpiChanged       = [this]()                                 { OnWindowDpiChanged(); };
	callbacks.OnFocus            = [this](bool focus)                       { OnWindowFocus(focus); };
	callbacks.OnActivate         = [this](bool active)                      { OnWindowActivate(active); };
	callbacks.OnSurfaceCreated   = [this](void* handle)                     { OnWindowSurfaceCreated(handle); };
	callbacks.OnSurfaceDestroyed = [this]()                                 { OnWindowSurfaceDestroyed(); };
	callbacks.OnSuspend          = [this]()                                 { OnWindowSuspend(); };
	callbacks.OnResume           = [this]()                                 { OnWindowResume(); };
	callbacks.OnLowMemory        = [this]()                                 { OnWindowLowMemory(); };
	callbacks.OnSafeAreaChanged  = [this](int t, int b, int l, int r)       { OnWindowSafeAreaChanged(t, b, l, r); };

	m_Window = safe_make_shared<Window>(m_Platform, m_Input, std::move(callbacks));
	auto res = m_Window->Initialize(m_Platform.GetAppName(), data);
	if (!res)
		THROW_RUNTIME("Failed to initialize window: {}.", res.error());
}

#pragma region Window Events
void View::HandleWindowClose()
{
	DOut("Window Event: OnClose");

	// В редакторе управление происходит из вне поэтому колбэк может быть не инициализирован
	if (OnWindowClose != nullptr)
		OnWindowClose(*this);
}

void View::OnWindowResize(Size2D<>& size, eWinResize type)
{
	DOut("Window Event: OnResize ({}x{}) Type: {}", size.width, size.height, EnumToString::ToString(type));
}

void View::OnWindowResizeStart()
{
	DOut("Window Event: OnResizeStart");
}

void View::OnWindowSizing()
{
	DOut("Window Event: OnSizing");
}

void View::OnWindowResizeEnd()
{
	DOut("Window Event: OnResizeEnd");
}

void View::OnWindowDpiChanged()
{
	DOut("Window Event: OnDpiChanged");
}

void View::OnWindowFocus(bool focus)
{
	DOut("Window Event: OnFocus ({})", focus ? "true" : "false");
	if (!focus)
	{
		m_Input->ResetState();
	}
}

void View::OnWindowActivate(bool active)
{
	DOut("Window Event: OnActivate ({})", active ? "true" : "false");
}

#pragma endregion

#pragma region App Lifecycle & GPU Surface
void View::OnWindowSurfaceCreated(void* handle)
{
	DOut("Window Event: OnSurfaceCreated (handle: {})", handle);
}

void View::OnWindowSurfaceDestroyed()
{
	DOut("Window Event: OnSurfaceDestroyed");
}

void View::OnWindowSuspend()
{
	DOut("Window Event: OnSuspend");
}

void View::OnWindowResume()
{
	DOut("Window Event: OnResume");
}

void View::OnWindowLowMemory()
{
	DOut("Window Event: OnLowMemory");
}

void View::OnWindowSafeAreaChanged(int top, int bottom, int left, int right)
{
	DOut("Window Event: OnSafeAreaChanged (t:{}, b:{}, l:{}, r:{})", top, bottom, left, right);
}
#pragma endregion

