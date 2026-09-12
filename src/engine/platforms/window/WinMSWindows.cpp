
#include "core/utils/Defines.h"

#if defined(Z_WINDOWS)

#include "WinMSWindows.h"
#include "../Platform.h"
#include "../monitor/IMonitorProvider.h"
#include "engine/view/View.h"

Z_SET_LOG_CATEGORY(::zzz::core::Window);

using namespace zzz::core;

using namespace zzz::engine;

WinMSWindows::WinMSWindows(const Platform& platform, const std::shared_ptr<Input> input, WindowCallbacks callbacks) :
	WindowBase(platform, input, std::move(callbacks)),
	m_hWnd{ nullptr }
{}

WinMSWindows::~WinMSWindows()
{
	if (m_hWnd)
	{
		HWND hWndTemp = m_hWnd;
		m_hWnd = nullptr;
		DestroyWindow(hWndTemp);
	}
}

LRESULT CALLBACK WinMSWindows::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept
{
	try
	{
		MSWinCtx* ctx = nullptr;

		if (uMsg == WM_NCCREATE)
		{
			const auto* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
			ctx = static_cast<MSWinCtx*>(pCreate->lpCreateParams);
			SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(ctx));
		}
		else
			ctx = reinterpret_cast<MSWinCtx*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

		if (ctx)
		{
			bool isHandleInput = true;
			if (uMsg == WM_CLOSE)
				isHandleInput = false;

			auto procRes = ctx->window->MsgProc(hWnd, uMsg, wParam, lParam);
			if (procRes.isContinue)
			{
				if (isHandleInput && ctx->input->ProcessMessage({ hWnd, uMsg, wParam, lParam }))
					return 0;

				return DefWindowProc(hWnd, uMsg, wParam, lParam);
			}

			return procRes.result;
		}
	}
	catch (const std::exception& e)
	{
		DOutCritical("Исключение в WindowProc: {}", e.what());
		PostMessage(hWnd, WM_CLOSE, 0, 0);
	}
	catch (...)
	{
		DOutCritical("Неизвестное исключение в WindowProc");
		PostMessage(hWnd, WM_CLOSE, 0, 0);
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

[[nodiscard]] bool WinMSWindows::IsMaximized() const
{
	ensure(m_hWnd != nullptr, "WinMSWindows::IsMaximized вызван для неинициализированного окна (m_hWnd == nullptr).");

	WINDOWPLACEMENT wp{};
	wp.length = sizeof(WINDOWPLACEMENT);
	if (GetWindowPlacement(m_hWnd, &wp))
		return (wp.showCmd == SW_SHOWMAXIMIZED);

	return false;
}

Rect2D<zI32> WinMSWindows::GetRestoredWindowRect() const
{
	ensure(m_hWnd != nullptr, "WinMSWindows::GetRestoredWindowRect вызван для неинициализированного окна (m_hWnd == nullptr).");

	WINDOWPLACEMENT wp{};
	wp.length = sizeof(WINDOWPLACEMENT);
	if (GetWindowPlacement(m_hWnd, &wp))
	{
		const RECT& r = wp.rcNormalPosition;
		return Rect2D<zI32>{ Point2D<zI32>{r.left, r.top}, Size2D<zI32>{r.right - r.left, r.bottom - r.top} };
	}

	RECT r{};
	if (!GetWindowRect(m_hWnd, &r))
		THROW_RUNTIME("GetWindowRect завершился ошибкой.");

	return Rect2D<zI32>{ Point2D<zI32>{r.left, r.top}, Size2D<zI32>{r.right - r.left, r.bottom - r.top} };
}

Rect2D<zI32> WinMSWindows::GetClientRect() const
{
	ensure(m_hWnd != nullptr, "WinMSWindows::GetClientRect вызван для неинициализированного окна (m_hWnd == nullptr).");

	RECT clientRect{};
	if (!::GetClientRect(m_hWnd, &clientRect))
		THROW_RUNTIME("GetClientRect завершился ошибкой.");

	POINT pt{ clientRect.left, clientRect.top };
	ClientToScreen(m_hWnd, &pt);

	return Rect2D<zI32>{ Point2D<zI32>{pt.x, pt.y}, Size2D<zI32>{clientRect.right - clientRect.left, clientRect.bottom - clientRect.top} };
}

std::expected<void, std::string> WinMSWindows::Initialize(const ViewPlatformData& platformData, const View* parentView)
{
	HWND parentHWnd = parentView ? parentView->GetNativeWindow().GetHWnd() : nullptr;
	bool isChild = (parentHWnd != nullptr);

	// Только первичное окно может быть в режиме Fullscreen/Borderless. Дочерние всегда Windowed.
	eMSWinWindowMode winMode = isChild ? eMSWinWindowMode::Windowed : platformData.GetWindowMode();

	// Closed -> Normal сбрасываем безусловно, для ЛЮБОГО View: раз дошли до Initialize(), значит
	// окно физически создаётся и показывается прямо сейчас - Closed как результирующее состояние
	// невалиден в принципе (решение "не создавать это окно вовсе" принимается снаружи, ДО вызова
	// Initialize() - см. ViewManager/Engine::Run(), проверка сохранённого состояния перед
	// CreateChildView()/CreateIndependentView()).
	// Minimized -> Normal сбрасываем только для Primary: нельзя стартовать приложение свёрнутым.
	// Для Child/Independent (platformData.IsPrimary() == false) пытаемся восстановить сохранённую
	// свёрнутость как есть.
	eWindowState targetState = platformData.GetWindowState();
	if (targetState == eWindowState::Closed)
		targetState = eWindowState::Normal;
	if (platformData.IsPrimary() && targetState == eWindowState::Minimized)
		targetState = eWindowState::Normal;

	const auto& windowRect = platformData.GetWindowRect();
	int xPos = windowRect.position.x;
	int yPos = windowRect.position.y;
	int width = static_cast<int>(windowRect.size.width);
	int height = static_cast<int>(windowRect.size.height);

	DWORD windowStyle = ConverterMSWinTypes::ToNative(winMode);
	// Флаг IsResizable строго берется из ProjectData (platformData)
	if (!platformData.IsResizable())
		windowStyle &= ~(WS_THICKFRAME | WS_MAXIMIZEBOX);

	m_Ctx = { this, m_Input.get() };
	CreateWindowEx(
		0,
		m_Platform.GetProjectPlatformData().GetWindowClassName().c_str(),
		platformData.GetTitle().c_str(),
		windowStyle,
		xPos, yPos, width, height,
		parentHWnd,
		nullptr,
		GetModuleHandle(NULL),
		&m_Ctx);

	if (!m_hWnd)
		THROW_RUNTIME("CreateWindowEx( ... ) завершился ошибкой. Код ошибки (Windows): {}", ::GetLastError());

	m_NativeState.SetWindowRect(GetRestoredWindowRect());
	m_NativeState.SetState(targetState);
	if (!platformData.GetMonitorId().empty())
		m_NativeState.SetMonitorId(platformData.GetMonitorId());

	// [Windows] Системное окно успешно создано.
	// Передаем m_hWnd наверх (во View/Engine), чтобы графическое API (Vulkan/DirectX)
	// могло привязаться к этому окну и создать Swapchain. Без этого рендеринг невозможен.
	VERIFY_AND_CALL(m_Callbacks.OnSurfaceCreated, m_hWnd);

	int showCmd = (targetState == eWindowState::Maximized) ? SW_MAXIMIZE : ((targetState == eWindowState::Minimized) ? SW_MINIMIZE : SW_SHOW);
	ShowWindow(m_hWnd, showCmd);
	UpdateWindow(m_hWnd);

	return std::expected<void, std::string>();
}

WinMSWindows::MsgProcResult WinMSWindows::MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_NCCREATE:
		m_hWnd = hWnd;
		return { true, TRUE };

	case WM_CLOSE:
		/**
		 * @brief [Windows] Пользователь нажал крестик или Alt+F4.
		 * Транслируем в OnClose, чтобы движок начал плавное завершение работы.
		 */
		VERIFY_AND_CALL(m_Callbacks.OnClose);
		return { false, 0 };

	case WM_DESTROY:
		/**
		 * @brief [Windows] Окно физически уничтожается операционной системой.
		 * Транслируем в OnSurfaceDestroyed, чтобы убить Vulkan/Metal Swapchain
		 * строго ДО того, как хэндл окна станет невалидным.
		 *
		 * ВАЖНО: PostQuitMessage(0) здесь намеренно не вызывается. Это окно - одно
		 * из потенциально нескольких (Primary/Child/Independent) на одной очереди
		 * сообщений потока (см. MainLoopMSWin::Run() - PeekMessage с hWnd=0 вычерпывает
		 * сообщения всех окон разом), и WM_DESTROY тут приходит для ЛЮБОГО из них, а не
		 * только для Primary. Решение "закрыть всё приложение" принимает ViewManager -
		 * View::HandleWindowClose() -> ViewManager::OnWindowClose() смотрит, Primary это
		 * или нет, и только для Primary зовёт OnAllViewsClosed() -> Engine::OnAppClosed()
		 * -> MainLoop::Stop() (isRunning.store(false), MainLoopCommon.h) - этого достаточно,
		 * чтобы MainLoopMSWin::Run() вышел из цикла, WM_QUIT не нужен. PostQuitMessage(0)
		 * тут раньше стоял безусловно и по факту закрывал ВСЕ окна при закрытии любого
		 * одного (см. rendering_pipeline_review.md).
		 */
		VERIFY_AND_CALL(m_Callbacks.OnSurfaceDestroyed);
		SetWindowLongPtr(hWnd, GWLP_USERDATA, 0);
		return { true, TRUE };

	case WM_SIZE:
		/**
		 * @brief [Windows] Размер клиентской области изменился.
		 * Транслируем в OnResize. Также отслеживаем состояния минимизации (Hide) и восстановления (Show).
		 */
		{
			Size2D<> winSize(static_cast<zU32>(LOWORD(lParam)), static_cast<zU32>(HIWORD(lParam)));
			if (wParam == SIZE_MINIMIZED)
			{
				m_NativeState.SetState(eWindowState::Minimized);
				VERIFY_AND_CALL(m_Callbacks.OnResize, winSize, eWinResize::Hide);
			}
			else if (wParam == SIZE_MAXIMIZED)
			{
				m_NativeState.SetState(eWindowState::Maximized);
				VERIFY_AND_CALL(m_Callbacks.OnResize, winSize, eWinResize::Show);
			}
			else if (wParam == SIZE_RESTORED)
			{
				m_NativeState.SetState(eWindowState::Normal);
				VERIFY_AND_CALL(m_Callbacks.OnResize, winSize, eWinResize::Show);
			}
			else
			{
				VERIFY_AND_CALL(m_Callbacks.OnResize, winSize, eWinResize::Resize);
			}
		}
		return { false, 0 };

	case WM_ENTERSIZEMOVE:
		m_SizeMoveMode = eSizeMoveMode::None;
		return { false, 0 };

	case WM_SIZING:
		if (m_SizeMoveMode != eSizeMoveMode::Resize)
		{
			m_SizeMoveMode = eSizeMoveMode::Resize;
			VERIFY_AND_CALL(m_Callbacks.OnResizeStart);
		}
		VERIFY_AND_CALL(m_Callbacks.OnSizing);
		return { false, 0 };

	case WM_MOVING:
		if (m_SizeMoveMode != eSizeMoveMode::Move)
		{
			m_SizeMoveMode = eSizeMoveMode::Move;
			VERIFY_AND_CALL(m_Callbacks.OnMoveStart);
		}
		VERIFY_AND_CALL(m_Callbacks.OnMoving);
		return { false, 0 };

	case WM_MOVE:
		m_NativeState.SetWindowRect(GetRestoredWindowRect());
		return { false, 0 };

	case WM_EXITSIZEMOVE:
		m_NativeState.SetWindowRect(GetRestoredWindowRect());
		if (m_SizeMoveMode == eSizeMoveMode::Resize)
		{
			VERIFY_AND_CALL(m_Callbacks.OnResizeEnd);
		}
		else if (m_SizeMoveMode == eSizeMoveMode::Move)
		{
			VERIFY_AND_CALL(m_Callbacks.OnMoveEnd);
		}
		m_SizeMoveMode = eSizeMoveMode::None;
		return { false, 0 };

	case WM_GETMINMAXINFO:
	{
		MINMAXINFO* pMinMaxInfo = reinterpret_cast<MINMAXINFO*>(lParam);
		DWORD dwStyle = static_cast<DWORD>(GetWindowLongPtr(hWnd, GWL_STYLE));
		DWORD dwExStyle = static_cast<DWORD>(GetWindowLongPtr(hWnd, GWL_EXSTYLE));
		BOOL bMenu = (GetMenu(hWnd) != NULL);

		RECT minRect = { 0, 0, static_cast<LONG>(c_MinWinSize), static_cast<LONG>(c_MinWinSize) };
		AdjustWindowRectEx(&minRect, dwStyle, bMenu, dwExStyle);
		pMinMaxInfo->ptMinTrackSize.x = minRect.right - minRect.left;
		pMinMaxInfo->ptMinTrackSize.y = minRect.bottom - minRect.top;

		RECT maxRect = { 0, 0, static_cast<LONG>(c_UHD_4K.width), static_cast<LONG>(c_UHD_4K.height) };
		AdjustWindowRect(&maxRect, WS_OVERLAPPEDWINDOW, FALSE);
		pMinMaxInfo->ptMaxTrackSize.x = maxRect.right - maxRect.left;
		pMinMaxInfo->ptMaxTrackSize.y = maxRect.bottom - maxRect.top;

		return { false, 0 };
	}

	// Обрабатываем изменение разрешения и конфигурации мониторов
	case WM_DISPLAYCHANGE:
	{
		OnMonitorResolutionChanged();
		VERIFY_AND_CALL(m_Callbacks.OnDisplayChanged);
		return { false, 0 };
	}

	// Обрабатываем изменение DPI в системе
	case WM_DPICHANGED:
	{
		/**
		 * @brief [Windows] Окно было перенесено на монитор с другим масштабом (DPI).
		 */
		const auto* suggestedRect = reinterpret_cast<const RECT*>(lParam);
		if (suggestedRect)
		{
			SetWindowPos(m_hWnd, nullptr,
				suggestedRect->left, suggestedRect->top,
				suggestedRect->right - suggestedRect->left,
				suggestedRect->bottom - suggestedRect->top,
				SWP_NOZORDER | SWP_NOACTIVATE);
			m_NativeState.SetWindowRect(GetRestoredWindowRect());
		}

		OnMonitorResolutionChanged();
		VERIFY_AND_CALL(m_Callbacks.OnDpiChanged);

		return { false, 0 };
	}

	case WM_SETFOCUS:
		/**
		 * @brief [Windows] Окно получило фокус клавиатуры (пользователь кликнул по нему).
		 */
		VERIFY_AND_CALL(m_Callbacks.OnFocus, true);
		break;

	case WM_KILLFOCUS:
		/**
		 * @brief [Windows] Окно потеряло фокус клавиатуры (пользователь переключился на другое приложение).
		 */
		VERIFY_AND_CALL(m_Callbacks.OnFocus, false);
		break;

	case WM_ACTIVATE:
		/**
		 * @brief [Windows] Изменение активности окна (например, окно ушло на задний план, но всё ещё видно).
		 */
		m_IsActivate = (LOWORD(wParam) != WA_INACTIVE);
		VERIFY_AND_CALL(m_Callbacks.OnActivate, m_IsActivate);
		break;

	case WM_POWERBROADCAST:
		/**
		 * @brief [Windows] События электропитания системы.
		 */
		if (wParam == PBT_APMSUSPEND)
			VERIFY_AND_CALL(m_Callbacks.OnSuspend); // ПК уходит в спящий/ждущий режим (Suspend).
		else if (wParam == PBT_APMRESUMESUSPEND)
			VERIFY_AND_CALL(m_Callbacks.OnResume);  // ПК проснулся и восстановил работу (Resume).
		return { true, TRUE };

	case WM_COMPACTING:
		/**
		 * @brief [Windows] ОС просит запущенные приложения попытаться освободить оперативную память.
		 * Транслируем в OnLowMemory, чтобы очистить кэши текстур.
		 */
		VERIFY_AND_CALL(m_Callbacks.OnLowMemory);
		return { false, 0 };
	}

	return { true, 0 };
}

void WinMSWindows::OnMonitorResolutionChanged()
{
	const auto& monitorProvider = m_Platform.GetMonitorProvider();
	// Обновляем нативный список мониторов
	const_cast<IMonitorProvider&>(monitorProvider).RefreshMonitors();

	Rect2D<zI32> normalRect = GetRestoredWindowRect();
	MonitorInfo monitor = monitorProvider.GetMonitorForRect(normalRect);

	// Обновляем идентификатор текущего монитора окна в NativeState
	m_NativeState.SetMonitorId(monitor.GetPlatformMonitorId());

	// Если окно в обычным оконном режиме и вылезает за новую WorkArea - ужимаем его
	if (!IsMaximized() && !IsMinimized())
	{
		Rect2D<zI32> fittedRect = monitorProvider.FitToWorkArea(normalRect, monitor);
		if (normalRect != fittedRect)
		{
			SetWindowPos(m_hWnd, nullptr,
				fittedRect.position.x, fittedRect.position.y,
				static_cast<int>(fittedRect.size.width), static_cast<int>(fittedRect.size.height),
				SWP_NOZORDER | SWP_NOACTIVATE);
			m_NativeState.SetWindowRect(fittedRect);
		}
	}
}

#endif // defined(Z_WINDOWS)
