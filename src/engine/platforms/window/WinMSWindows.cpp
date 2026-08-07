
#include "WinMSWindows.h"
#include "../Platform.h"

using namespace zzz::core;

using namespace zzz::engine;

WinMSWindows::WinMSWindows(const Platform& platform, const std::shared_ptr<Input> input, WindowCallbacks callbacks) :
	WindowBase(platform, input, std::move(callbacks)),
	m_hWnd{ nullptr },
	m_IsMinimized{ true }
{}

WinMSWindows::~WinMSWindows()
{
	if (m_hWnd)
	{
		SetWindowLongPtr(m_hWnd, GWLP_USERDATA, 0);
		DestroyWindow(m_hWnd);
		m_hWnd = nullptr;
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

[[nodiscard]] std::expected<void, std::string> WinMSWindows::Initialize(const StartViewPlatformData& startWindowSettings, void* /*data*/)
{
	// Рассчитать размеры прямоугольника окна на основе запрошенных размеров клиентской области.
	Size2D<LONG> winSize;
	// TODO: не правильная архитектура. Подумать как задавать размер окна
	winSize.SetFrom(static_cast<LONG>(startWindowSettings.GetSize().width), static_cast<LONG>(startWindowSettings.GetSize().height));
	DWORD windowStyle = ConverterMSWinTypes::ToNative(startWindowSettings.GetWindowMode());
	if (!startWindowSettings.IsResizable())
		windowStyle &= ~(WS_THICKFRAME | WS_MAXIMIZEBOX);
	RECT R = { 0, 0, winSize.width, winSize.height };
	AdjustWindowRectEx(&R, windowStyle, false, 0);
	int width = R.right - R.left;
	int height = R.bottom - R.top;

	int screenWidth = GetSystemMetrics(SM_CXSCREEN);  // Ширина экрана
	int screenHeight = GetSystemMetrics(SM_CYSCREEN); // Высота экрана
	int xPos = (screenWidth - width) / 2;  // Расчет позиции по оси X
	int yPos = (screenHeight - height) / 2; // Расчет позиции по оси Y

	m_Ctx = { this, m_Input.get() };
	CreateWindowEx(
		0,
		m_Platform.GetProjectPlatformData().GetWindowClassName().c_str(),
		startWindowSettings.GetTitle().c_str(),
		windowStyle,
		xPos, yPos, width, height,
		nullptr,
		nullptr,
		GetModuleHandle(NULL),
		&m_Ctx);

	if (!m_hWnd)
		THROW_RUNTIME("CreateWindowEx( ... ) завершился ошибкой. Код ошибки (Windows): {}", ::GetLastError());

	// [Windows] Системное окно успешно создано.
	// Передаем m_hWnd наверх (во View/Engine), чтобы графическое API (Vulkan/DirectX)
	// могло привязаться к этому окну и создать Swapchain. Без этого рендеринг невозможен.
	VERIFY_AND_CALL(m_Callbacks.OnSurfaceCreated, m_hWnd);

	ShowWindow(m_hWnd, SW_SHOW);
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
		 */
		VERIFY_AND_CALL(m_Callbacks.OnSurfaceDestroyed);
		PostQuitMessage(0);
		return { true, TRUE };

	case WM_SIZE:
		/**
		 * @brief [Windows] Размер клиентской области изменился.
		 * Транслируем в OnResize. Также отслеживаем состояния минимизации (Hide) и восстановления (Show).
		 */
		m_WinSize.SetFrom(static_cast<zU32>(LOWORD(lParam)), static_cast<zU32>(HIWORD(lParam)));
		if (wParam == SIZE_MINIMIZED)
		{
			VERIFY_AND_CALL(m_Callbacks.OnResize, m_WinSize, eWinResize::Hide);
			m_IsMinimized = true;
		}
		else
		{
			if ((wParam == SIZE_RESTORED || wParam == SIZE_MAXIMIZED) && m_IsMinimized)
			{
				VERIFY_AND_CALL(m_Callbacks.OnResize, m_WinSize, eWinResize::Show);
				m_IsMinimized = false;
			}
			else
			{
				VERIFY_AND_CALL(m_Callbacks.OnResize, m_WinSize, eWinResize::Resize);
			}
		}
		return { false, 0 };

	case WM_ENTERSIZEMOVE:
		/**
		 * @brief [Windows] Пользователь захватил рамку окна мышью.
		 */
		VERIFY_AND_CALL(m_Callbacks.OnResizeStart);
		return { false, 0 };

	case WM_SIZING:
		/**
		 * @brief [Windows] Пользователь активно перетаскивает рамку окна.
		 * Windows блокирует главный поток в этот момент, поэтому рендер может замирать.
		 */
		VERIFY_AND_CALL(m_Callbacks.OnSizing);
		return { false, 0 };

	case WM_EXITSIZEMOVE:
		/**
		 * @brief [Windows] Пользователь отпустил рамку окна.
		 */
		VERIFY_AND_CALL(m_Callbacks.OnResizeEnd);
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

		RECT maxRect = { 0, 0, static_cast<LONG>(c_UHD_4K.GetWidth()), static_cast<LONG>(c_UHD_4K.GetHeight()) };
		AdjustWindowRect(&maxRect, WS_OVERLAPPEDWINDOW, FALSE);
		pMinMaxInfo->ptMaxTrackSize.x = maxRect.right - maxRect.left;
		pMinMaxInfo->ptMaxTrackSize.y = maxRect.bottom - maxRect.top;

		return { false, 0 };
	}

	// Обрабатываем изменение DPI в системе
	case WM_DPICHANGED:
	{
		/**
		 * @brief [Windows] Окно было перенесено на монитор с другим масштабом (DPI).
		 */
		m_WinSize.SetFrom(LOWORD(wParam), HIWORD(wParam));
		VERIFY_AND_CALL(m_Callbacks.OnDpiChanged);

		return { false, FALSE };
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
		m_IsActivate = (wParam != 0);
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

