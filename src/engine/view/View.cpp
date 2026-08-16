
#include "View.h"
#include "scene/Scene.h"
#include "../platforms/input/Input.h"
#include "../platforms/window/NativeWindow.h"

using namespace zzz::core;
using namespace zzz::engine;

View::View(const StartViewPlatformData& settings, const std::vector<Guid>& scripts, const Platform& platform, std::function<void(View&)> onWindowClose) :
	m_Platform{ platform },
	m_IsActive{ true },
	OnWindowClose(std::move(onWindowClose))
{
	ensure(OnWindowClose != nullptr, "OnWindowClose не должен быть null.");

	std::vector<std::shared_ptr<ViewScript>> viewScripts;
	for (const auto& viewScriptGuid : scripts)
	{
		auto viewScript = ScriptFactory::CreateViewScript(viewScriptGuid);
		if (!viewScript)
			THROW_RUNTIME("Не удалось создать ViewScript по GUID {}.", viewScriptGuid.ToString());

		viewScripts.push_back(viewScript);
	}

	Initialize(settings, viewScripts);
}

#if Z_EDITOR
View::View(const Platform& platform, void* data) :
	m_IsActive{ true },
	m_Platform{ platform }
{
	Initialize(data);
}
#endif

View::~View()
{
	m_EventBus.InvokeDestroy();
	m_Scripts.clear();
}

void View::Initialize(const StartViewPlatformData& settings, const std::vector<std::shared_ptr<ViewScript>>& scripts)
{
	m_Input = safe_make_shared<Input>();
	auto inputRes = m_Input->Initialize();
	if (!inputRes)
		THROW_RUNTIME("Не удалось инициализировать систему ввода: {}.", inputRes.error());

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

	m_NativeWindow = safe_make_shared<NativeWindow>(m_Platform, m_Input, std::move(callbacks));
	auto res = m_NativeWindow->Initialize(settings);
	if (!res)
		THROW_RUNTIME("Не удалось инициализировать окно: {}.", res.error());

	for (const auto& script : scripts)
	{
		if (!script)
			continue;

		script->Init(&m_EventBus);
		m_Scripts.push_back(script);
	}
}

#if Z_EDITOR
void View::Initialize(void* data)
{
	m_Input = safe_make_shared<Input>();
	auto inputRes = m_Input->Initialize();
	if (!inputRes)
		THROW_RUNTIME("Не удалось инициализировать систему ввода: {}.", inputRes.error());

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

	m_NativeWindow = safe_make_shared<NativeWindow>(m_Platform, m_Input, std::move(callbacks));
	auto res = m_NativeWindow->Initialize(StartViewPlatformData{}, data);
	if (!res)
		THROW_RUNTIME("Не удалось инициализировать окно: {}.", res.error());
}
#endif

#pragma region Window Events
void View::HandleWindowClose()
{
	DOut("Событие окна: OnClose");

	// В редакторе управление происходит из вне поэтому колбэк может быть не инициализирован
	if (OnWindowClose != nullptr)
		OnWindowClose(*this);
}

void View::OnWindowResize(Size2D<>& size, eWinResize type)
{
	DOut("Событие окна: OnResize ({}x{}) Тип: {}", size.width, size.height, EnumToString::ToString(type));
}

void View::OnWindowResizeStart()
{
	DOut("Событие окна: OnResizeStart");
}

void View::OnWindowSizing()
{
	DOut("Событие окна: OnSizing");
}

void View::OnWindowResizeEnd()
{
	DOut("Событие окна: OnResizeEnd");
}

void View::OnWindowDpiChanged()
{
	DOut("Событие окна: OnDpiChanged");
}

void View::OnWindowFocus(bool focus)
{
	DOut("Событие окна: OnFocus ({})", focus ? "true" : "false");
	if (!focus)
	{
		m_Input->ResetState();
	}
}

void View::OnWindowActivate(bool active)
{
	DOut("Событие окна: OnActivate ({})", active ? "true" : "false");
}

#pragma endregion

#pragma region App Lifecycle & GPU Surface
void View::OnWindowSurfaceCreated(void* handle)
{
	DOut("Событие окна: OnSurfaceCreated (handle: {})", handle);
}

void View::OnWindowSurfaceDestroyed()
{
	DOut("Событие окна: OnSurfaceDestroyed");
}

void View::OnWindowSuspend()
{
	DOut("Событие окна: OnSuspend");
}

void View::OnWindowResume()
{
	DOut("Событие окна: OnResume");
}

void View::OnWindowLowMemory()
{
	DOut("Событие окна: OnLowMemory");
}

void View::OnWindowSafeAreaChanged(int top, int bottom, int left, int right)
{
	DOut("Событие окна: OnSafeAreaChanged (t:{}, b:{}, l:{}, r:{})", top, bottom, left, right);
}
#pragma endregion

void View::Update(const Time& time)
{
	if (!m_IsActive)
		return;

	m_EventBus.InvokeUpdate(time);

	if (m_ActiveScene)
		m_ActiveScene->Update(time);
}
