
#include "View.h"
#include "scene/Scene.h"
#include "../platforms/input/Input.h"
#include "../platforms/window/NativeWindow.h"

using namespace zzz::core;
using namespace zzz::engine;

View::View(const ViewPlatformData& settings, const std::vector<Guid>& scripts, const Platform& platform, const ScriptFactory& scriptFactory, std::function<void(View&)> onWindowClose, const View* parentView) :
	m_Platform{ platform },
	m_Input{ safe_make_shared<Input>() },
	m_NativeWindow{ safe_make_shared<NativeWindow>(platform, m_Input, WindowCallbacks{}) },
	OnWindowClose{ std::move(onWindowClose) },
	m_IsActive{ true }
{
	ensure(OnWindowClose != nullptr, "OnWindowClose не должен быть null.");

	std::vector<std::shared_ptr<ViewScript>> viewScripts;
	viewScripts.reserve(scripts.size());
	for (const auto& guid : scripts)
	{
		auto script = scriptFactory.CreateViewScript(guid);
		ensure(script != nullptr, "Не удалось создать экземпляр ViewScript с GUID: " + guid.ToString());
		viewScripts.push_back(std::move(script));
	}

	Initialize(settings, viewScripts, parentView);
}

#if Z_EDITOR
View::View(const Platform& platform, void* data) :
	m_Platform{ platform },
	m_Input{ safe_make_shared<Input>() },
	m_NativeWindow{ safe_make_shared<NativeWindow>(platform, m_Input, WindowCallbacks{}) },
	m_IsActive{ true }
{
	Initialize(data);
}
#endif // Z_EDITOR

View::~View()
{
	m_EventBus.InvokeDestroy();
	m_Scripts.clear();
	m_ActiveScene = nullptr;
}

void View::Initialize(const ViewPlatformData& settings, const std::vector<std::shared_ptr<ViewScript>>& scripts, const View* parentView)
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
	auto res = m_NativeWindow->Initialize(settings, parentView);
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
	(void)data;
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
	auto res = m_NativeWindow->Initialize(ViewPlatformData{});
	if (!res)
		THROW_RUNTIME("Не удалось инициализировать окно: {}.", res.error());
}
#endif

#pragma region Window Events
void View::HandleWindowClose()
{
	DOut("[View::HandleWindowClose] - OnClose");

	// В редакторе управление происходит из вне поэтому колбэк может быть не инициализирован
	if (OnWindowClose != nullptr)
		OnWindowClose(*this);
}

void View::OnWindowResize(Size2D<>& size, eWinResize type)
{
	DOut("[View::OnWindowResize] - {}x{} (Type: {})", size.GetWidth(), size.GetHeight(), EnumToString::ToString(type));
}

void View::OnWindowResizeStart()
{
	DOut("[View::OnWindowResizeStart]");
}

void View::OnWindowSizing()
{
	DOut("[View::OnWindowSizing]");
}

void View::OnWindowResizeEnd()
{
	DOut("[View::OnWindowResizeEnd]");
}

void View::OnWindowDpiChanged()
{
	DOut("[View::OnWindowDpiChanged]");
}

void View::OnWindowFocus(bool focus)
{
	DOut("[View::OnWindowFocus] - Focus: {}", focus ? "true" : "false");
	if (!focus)
	{
		m_Input->ResetState();
	}
}

void View::OnWindowActivate(bool active)
{
	DOut("[View::OnWindowActivate] - Active: {}", active ? "true" : "false");
}

#pragma endregion

#pragma region App Lifecycle & GPU Surface
void View::OnWindowSurfaceCreated(void* handle)
{
	DOut("[View::OnWindowSurfaceCreated] - Handle: {}", handle);
}

void View::OnWindowSurfaceDestroyed()
{
	DOut("[View::OnWindowSurfaceDestroyed]");
}

void View::OnWindowSuspend()
{
	DOut("[View::OnWindowSuspend]");
}

void View::OnWindowResume()
{
	DOut("[View::OnWindowResume]");
}

void View::OnWindowLowMemory()
{
	DOut("[View::OnWindowLowMemory]");
}

void View::OnWindowSafeAreaChanged(int top, int bottom, int left, int right)
{
	DOut("[View::OnWindowSafeAreaChanged] - Top: {}, Bottom: {}, Left: {}, Right: {}", top, bottom, left, right);
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
