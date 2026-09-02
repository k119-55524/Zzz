
#include "scene/Scene.h"
#include "scene/SceneManager.h"
#include "../platforms/input/Input.h"
#include "engine/utils/EngineLogFlags.h"
#include "core/userscripts/ScriptFactory.h"
#include "../platforms/window/NativeWindow.h"
#include "core/enums/eWinResize.h"

#include "View.h"

Z_SET_LOG_CATEGORY(::zzz::core::Window);

using namespace zzz::core;
using namespace zzz::engine;

View::View(
	const ViewConfigData& viewData,
	ViewPlatformData* platformData,
	std::shared_ptr<ScriptFactory> scriptFactory,
	std::shared_ptr<SceneManager> sceneManager,
	const Platform& platform,
	std::shared_ptr<GAPI> gapi,
	std::function<void(View&)> onWindowClose,
	const View* parentView) :
	m_Platform{ platform },
	m_Guid{ viewData.GetViewGuid() },
	m_Input{ nullptr },
	m_NativeWindow{ nullptr },
	OnWindowClose{ std::move(onWindowClose) }
{
	ensure(platformData != nullptr, "ViewPlatformData не должен быть null.");
	ensure(gapi != nullptr, "GAPI не должен быть null.");
	ensure(OnWindowClose != nullptr, "OnWindowClose не должен быть null.");
	ensure(scriptFactory != nullptr, "ScriptFactory не должен быть null.");
	ensure(sceneManager != nullptr, "SceneManager не должен быть null.");

	Initialize(viewData, platformData, std::move(scriptFactory), std::move(sceneManager), std::move(gapi), parentView);
}

#if Z_EDITOR
View::View(const Platform& platform, std::shared_ptr<GAPI> gapi, void* data) :
	m_Platform{ platform },
	m_Input{ nullptr },
	m_NativeWindow{ nullptr }
{
	ensure(gapi != nullptr, "GAPI не должен быть null.");

	Initialize(data);
}
#endif // Z_EDITOR

View::~View()
{
	m_EventBus.InvokeDestroy();
	m_Scripts.clear();
	m_ActiveScene.reset();
}

void View::Initialize(const ViewConfigData& viewData, ViewPlatformData* platformData, std::shared_ptr<ScriptFactory> scriptFactory, std::shared_ptr<SceneManager> sceneManager, std::shared_ptr<GAPI> gapi, const View* parentView)
{
	m_UserPlatformData = platformData;

	m_Input = safe_make_shared<Input>();
	auto inputRes = m_Input->Initialize();
	if (!inputRes)
		THROW_RUNTIME("Не удалось инициализировать систему ввода: {}", inputRes.error());

	WindowCallbacks callbacks;
	callbacks.OnClose            = [this]()                                 { HandleWindowClose(); };
	callbacks.OnResize           = [this](Size2D<>& size, eWinResize type)  { OnWindowResize(size, type); };
	callbacks.OnResizeStart      = [this]()                                 { OnWindowResizeStart(); };
	callbacks.OnSizing           = [this]()                                 { OnWindowSizing(); };
	callbacks.OnResizeEnd        = [this]()                                 { OnWindowResizeEnd(); };
	callbacks.OnMoveStart        = [this]()                                 { OnWindowMoveStart(); };
	callbacks.OnMoving           = [this]()                                 { OnWindowMoving(); };
	callbacks.OnMoveEnd          = [this]()                                 { OnWindowMoveEnd(); };
	callbacks.OnDpiChanged       = [this]()                                 { OnWindowDpiChanged(); };
	callbacks.OnFocus            = [this](bool focus)                       { OnWindowFocus(focus); };
	callbacks.OnActivate         = [this](bool active)                      { OnWindowActivate(active); };
	callbacks.OnDisplayChanged   = [this]()                                 { OnWindowDisplayChanged(); };
	callbacks.OnSurfaceCreated   = [this](void* handle)                     { OnWindowSurfaceCreated(handle); };
	callbacks.OnSurfaceDestroyed = [this]()                                 { OnWindowSurfaceDestroyed(); };
	callbacks.OnSuspend          = [this]()                                 { OnWindowSuspend(); };
	callbacks.OnResume           = [this]()                                 { OnWindowResume(); };
	callbacks.OnLowMemory        = [this]()                                 { OnWindowLowMemory(); };
	callbacks.OnSafeAreaChanged  = [this](int t, int b, int l, int r)       { OnWindowSafeAreaChanged(t, b, l, r); };

	m_NativeWindow = safe_make_shared<NativeWindow>(m_Platform, m_Input, std::move(callbacks));
	m_SurfView = safe_make_shared<SurfView>(m_NativeWindow, std::move(gapi));
	m_RenderManager = safe_make_unique<RenderManager>(m_SurfView);

	auto res = m_NativeWindow->Initialize(*m_UserPlatformData, parentView);
	if (!res)
		THROW_RUNTIME("Не удалось инициализировать окно: {}", res.error());

	for (const auto& scriptGuid : viewData.GetUiScriptGuids())
	{
		auto script = scriptFactory->CreateViewScript(scriptGuid);
		ensure(script != nullptr, "Не удалось создать экземпляр ViewScript с GUID: " + scriptGuid.ToString());
		script->Init(&m_EventBus);
		m_Scripts.push_back(std::move(script));
	}

	auto sceneRes = sceneManager->LoadScene(viewData.GetSceneGuid());
	if (!sceneRes)
		THROW_RUNTIME("Не удалось загрузить стартовую сцену View (guid {}): {}", viewData.GetSceneGuid().ToString(), sceneRes.error());

	m_ActiveScene = *sceneRes;
}

#if Z_EDITOR
void View::Initialize(void* data)
{
	(void)data;
	m_Input = safe_make_shared<Input>();
	auto inputRes = m_Input->Initialize();
	if (!inputRes)
		THROW_RUNTIME("Не удалось инициализировать систему ввода: {}", inputRes.error());

	WindowCallbacks callbacks;
	callbacks.OnClose            = [this]()                                 { HandleWindowClose(); };
	callbacks.OnResize           = [this](Size2D<>& size, eWinResize type)  { OnWindowResize(size, type); };
	callbacks.OnResizeStart      = [this]()                                 { OnWindowResizeStart(); };
	callbacks.OnSizing           = [this]()                                 { OnWindowSizing(); };
	callbacks.OnResizeEnd        = [this]()                                 { OnWindowResizeEnd(); };
	callbacks.OnMoveStart        = [this]()                                 { OnWindowMoveStart(); };
	callbacks.OnMoving           = [this]()                                 { OnWindowMoving(); };
	callbacks.OnMoveEnd          = [this]()                                 { OnWindowMoveEnd(); };
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
		THROW_RUNTIME("Не удалось инициализировать окно: {}", res.error());
}
#endif

#pragma region Window Events
void View::HandleWindowClose()
{
	DOut("[View::HandleWindowClose] - OnClose");

	// Закрытие окна - это отдельный, самодостаточный факт: независимо от того, в каком состоянии
	// (Normal/Maximized/Minimized) окно было секунду назад, теперь оно Closed. Это единственное
	// место, где eWindowState::Closed вообще пишется - используется ViewManager/Engine на старте,
	// чтобы не пересоздавать окна, которые пользователь сам закрыл (см. rendering_pipeline_review.md).
	const auto& navState = m_NativeWindow->GetState();
	m_UserPlatformData->SetWindowState(eWindowState::Closed);
	if (navState.GetState() == eWindowState::Normal)
		m_UserPlatformData->SetWindowRect(navState.GetWindowRect());

	// В редакторе управление происходит из вне поэтому колбэк может быть не инициализирован
#if !Z_EDITOR
	OnWindowClose(*this);
#endif // !Z_EDITOR
}

void View::OnWindowResize(Size2D<>& size, eWinResize type)
{
	m_EventBus.InvokeResize(size, type);

	if (m_SurfView)
		m_SurfView->OnResize(size);

	const auto& state = m_NativeWindow->GetState();
	switch (state.GetState())
	{
	case eWindowState::Maximized:
		m_UserPlatformData->SetWindowState(eWindowState::Maximized);
		break;
	case eWindowState::Normal:
		m_UserPlatformData->SetWindowState(eWindowState::Normal);
		m_UserPlatformData->SetWindowRect(state.GetWindowRect());
		break;
	case eWindowState::Minimized:
		m_UserPlatformData->SetWindowState(eWindowState::Minimized);
		break;
	default:
		break;
	}

	DOut(!Z_LOG_GET(g_IsResizing), "[View::OnWindowResize] - {}x{} (Type: {})", size.width, size.height, ToString(type));
}

void View::OnWindowResizeStart()
{
	Z_LOG_SET(g_IsResizing, true);
	DOut("[View::OnWindowResizeStart]");
}

void View::OnWindowSizing()
{
	DOut(0.5, "[View::OnWindowSizing]");
}

void View::OnWindowResizeEnd()
{
	Z_LOG_SET(g_IsResizing, false);
	const auto& navState = m_NativeWindow->GetState();
	if (navState.GetState() == eWindowState::Normal)
		m_UserPlatformData->SetWindowRect(navState.GetWindowRect());

	auto finalSize = m_NativeWindow->GetPhysicalClientSize();
	OnWindowResize(finalSize, eWinResize::Resize);

	DOut("[View::OnWindowResizeEnd] - Resize completed at {}x{}", finalSize.width, finalSize.height);
}

void View::OnWindowMoveStart()
{
	DOut("[View::OnWindowMoveStart]");
}

void View::OnWindowMoving()
{
	DOut(0.5, "[View::OnWindowMoving]");
}

void View::OnWindowMoveEnd()
{
	const auto& navState = m_NativeWindow->GetState();
	if (navState.GetState() == eWindowState::Normal)
		m_UserPlatformData->SetWindowRect(navState.GetWindowRect());

	DOut("[View::OnWindowMoveEnd]");
}

void View::OnWindowDpiChanged()
{
	DOut("[View::OnWindowDpiChanged]");
}

void View::OnWindowFocus(bool focus)
{
	DOut("[View::OnWindowFocus] - Focus: {}", focus ? "true" : "false");
	if (!focus)
		m_Input->ResetState();
}

void View::OnWindowActivate(bool active)
{
	DOut("[View::OnWindowActivate] - Active: {}", active ? "true" : "false");
}

void View::OnWindowDisplayChanged()
{
	DOut("[View::OnWindowDisplayChanged]");
}

#pragma endregion

#pragma region App Lifecycle & GPU Surface
/**
 * @brief Вызывается при выделении нативной графической поверхности операционной системой.
 * @param handle Нативный хэндл окна/поверхности (HWND в DirectX 12, ANativeWindow* в Vulkan, CAMetalLayer* в Metal).
 * Передает хэндл в SurfView для привязки и создания цепочки кадров Swapchain.
 */
void View::OnWindowSurfaceCreated(void* handle)
{
	DOut("[View::OnWindowSurfaceCreated] - Handle: {}", handle);
	m_SurfView->OnSurfaceCreated(handle);
}

/**
 * @brief Вызывается при уничтожении графической поверхности операционной системой.
 * Сообщает SurfView о необходимости остановки GPU (WaitForGpu) и безопасного освобождения Swapchain и буфера глубины.
 */
void View::OnWindowSurfaceDestroyed()
{
	DOut("[View::OnWindowSurfaceDestroyed]");
	m_SurfView->OnSurfaceDestroyed();
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
	m_EventBus.InvokeUpdate(time);
}

void View::PreRender()
{
	m_RenderManager->PreRender();
}

void View::PrepareFrame()
{
	m_RenderManager->PrepareFrame(m_ActiveScene.lock());
}

void View::RenderFrame()
{
	m_RenderManager->RenderFrame();
}

void View::PostRender()
{
	m_RenderManager->PostRender();
}
