
export module AppWin_MSWin;

import Event;
import MsgBox;
import Size2D;
import Result;
import KeyCode;
import IAppWin;
import ibMSWin;
import AppConfig;
import StrConvert;
import IOPathFactory;
import EngineConstants;

using namespace zzz;

export namespace zzz::core
{
	export class AppWin_MSWin final : public IAppWin
	{
		Z_NO_CREATE_COPY(AppWin_MSWin);

	public:
		explicit AppWin_MSWin(std::shared_ptr<AppConfig> _settings);
		virtual ~AppWin_MSWin() override;

		const HWND GetHWND() const noexcept { return hWnd; }

		void SetCaptionText(std::wstring caption) override;
		void AddCaptionText(std::wstring caption) override;

		Event<bool> OnMouseEnter;
		Event<zI32, zI32> OnMouseDelta;
		Event<MouseButtonMask, MouseButtonMask> OnMouseButtonsChanged;
		Event<zI32> OnMouseWheelVertical;
		Event<zI32> OnMouseWheelHorizontal;
		Event<KeyCode, KeyState> OnKeyStateChanged;

	protected:
		virtual Result<> Initialize() override;

	private:
		HWND hWnd;
		bool IsMinimized;

		bool mouseInside;

		static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept;
		LRESULT MsgProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

		int InitRawInput();
		void OnRawInput(HRAWINPUT hRawInput);
		void HandleRawMouse(const RAWMOUSE& mouse);
		void HandleRawKeyboard(const RAWKEYBOARD& kb);
	};

	AppWin_MSWin::AppWin_MSWin(std::shared_ptr<AppConfig> config) :
		IAppWin(config),
		hWnd{ nullptr },
		IsMinimized{ true },
		mouseInside{ false }
	{
		SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
	}

	AppWin_MSWin::~AppWin_MSWin()
	{
		if (hWnd)
			DestroyWindow(hWnd);
	}

	Result<> AppWin_MSWin::Initialize()
	{
		HICON iconHandle = nullptr;
		{
			Result<std::wstring> icoPath = m_Config->GetIcoFullPath();
			if (icoPath)
			{
				ibMSWin icoBuilder;
				auto res = icoBuilder.LoadIco(icoPath.value(), m_Config->GetIcoSize());
				if (res)
					iconHandle = res.value();
			}
		}

		WNDCLASS wc = { 0 };
		wc.style = CS_HREDRAW | CS_VREDRAW;
		wc.lpfnWndProc = AppWin_MSWin::WindowProc;
		wc.hInstance = GetModuleHandle(NULL);
		wc.hIcon = iconHandle;
		wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
		wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
		wc.lpszClassName = m_Config->GetClassName().c_str();

		ATOM Result = RegisterClass(&wc);
		if (Result == 0)
		{
			std::wstring mess = L">>>>> [SW_MSWindows.Initialize( ... )]. RegisterClass( ... ). Failed to register window class: " + m_Config->GetClassName() + L". Error code(MSWindows):" + std::to_wstring(::GetLastError());
			return Unexpected(eResult::failure, mess);
		}

		// Рассчитать размеры прямоугольника окна на основе запрошенных размеров клиентской области.
		const Size2D<LONG>& winSize = m_Config->GetWinSize();
		RECT R = { 0, 0, winSize.width, winSize.height };
		AdjustWindowRectEx(&R, WS_OVERLAPPEDWINDOW, false, 0);
		int width = R.right - R.left;
		int height = R.bottom - R.top;

		int screenWidth = GetSystemMetrics(SM_CXSCREEN);  // Ширина экрана
		int screenHeight = GetSystemMetrics(SM_CYSCREEN); // Высота экрана
		int xPos = (screenWidth - width) / 2;  // Расчет позиции по оси X
		int yPos = (screenHeight - height) / 2; // Расчет позиции по оси Y
		hWnd = CreateWindowEx(
			0,
			m_Config->GetClassName().c_str(),
			m_Config->GetAppName().c_str(),
			WS_OVERLAPPEDWINDOW,
			xPos, yPos, width, height,
			nullptr,
			nullptr,
			GetModuleHandle(NULL),
			this);

		if (!hWnd)
			throw_runtime_error(wstring_to_string(std::format(L"CreateWindowEx( ... ) failed. Error code (Windows): {}", ::GetLastError())));

		ShowWindow(hWnd, SW_SHOW);
		UpdateWindow(hWnd);

		return {};
	}

	LRESULT CALLBACK AppWin_MSWin::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept
	{
		AppWin_MSWin* pThis = nullptr;

		try
		{
			if (uMsg == WM_NCCREATE)
			{
				const auto* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
				if (!pCreate || !pCreate->lpCreateParams)
					return FALSE; // Ошибка создания

				pThis = static_cast<AppWin_MSWin*>(pCreate->lpCreateParams);
				SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
				pThis->hWnd = hwnd;
			}
			else
				pThis = reinterpret_cast<AppWin_MSWin*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

			if (pThis)
				return pThis->MsgProc(uMsg, wParam, lParam);
		}
		catch (...)
		{
			// В callback функциях Windows нельзя допускать исключения
			DebugOutput(L">>>>> [AppWindowMsWin::WindowProc]. Exception in WindowProc!");
			return DefWindowProc(hwnd, uMsg, wParam, lParam);
		}

		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}

	LRESULT AppWin_MSWin::MsgProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		switch (uMsg)
		{
		case WM_CREATE:
			return InitRawInput();

		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;

		// Обрабатываем изменение размера окна после того, как пользователь закончил его изменять.
		case WM_SIZE:
		{
			m_WinSize.width = static_cast<zU64>(LOWORD(lParam));
			m_WinSize.height = static_cast<zU64>(HIWORD(lParam));
			if (wParam == SIZE_MINIMIZED)
			{
				OnResize(m_WinSize, eTypeWinResize::Hide);
				IsMinimized = true;
			}
			else
			{
				if ((wParam == SIZE_RESTORED || wParam == SIZE_MAXIMIZED) && IsMinimized)
				{
					OnResize(m_WinSize, eTypeWinResize::Show);
					IsMinimized = false;
				}
				else
				{
					OnResize(m_WinSize, eTypeWinResize::Resize);
				}
			}

			return 0;
		}

		// Обрабатываем изменение размера окна в процессе изменения его пользователем.
		case WM_SIZING:
			OnResizing();
			return 0;

		// Перехватываем это сообщение, чтобы не допустить слишком маленького/большого размера окна.
		case WM_GETMINMAXINFO:
		{
			MINMAXINFO* pMinMaxInfo = reinterpret_cast<MINMAXINFO*>(lParam);

			DWORD dwStyle = static_cast<DWORD>(GetWindowLongPtr(hWnd, GWL_STYLE));
			DWORD dwExStyle = static_cast<DWORD>(GetWindowLongPtr(hWnd, GWL_EXSTYLE));
			BOOL bMenu = (GetMenu(hWnd) != NULL);

			// Минимальный размер клиентской области
			RECT minRect = { 0, 0, static_cast<LONG>(g_MinimumWindowsWidth), static_cast<LONG>(g_MinimumWindowsHeight) };
			AdjustWindowRectEx(&minRect, dwStyle, bMenu, dwExStyle);
			pMinMaxInfo->ptMinTrackSize.x = minRect.right - minRect.left;
			pMinMaxInfo->ptMinTrackSize.y = minRect.bottom - minRect.top;

			// Максимальный размер клиентской области
			RECT maxRect = { 0, 0, static_cast<LONG>(g_MaximumWindowsWidth), static_cast<LONG>(g_MaximumWindowsHeight) };
			AdjustWindowRect(&maxRect, WS_OVERLAPPEDWINDOW, FALSE);
			pMinMaxInfo->ptMaxTrackSize.x = maxRect.right - maxRect.left;
			pMinMaxInfo->ptMaxTrackSize.y = maxRect.bottom - maxRect.top;

			return 0;
		}

		// Обрабатываем изменение DPI в системе
		case WM_DPICHANGED:
		{
			// Новый DPI
			UINT dpiX = LOWORD(wParam);
			UINT dpiY = HIWORD(wParam);
			RECT* const prcNewWindow = reinterpret_cast<RECT*>(lParam);
			SetWindowPos(
				hWnd,
				nullptr,
				prcNewWindow->left,
				prcNewWindow->top,
				prcNewWindow->right - prcNewWindow->left,
				prcNewWindow->bottom - prcNewWindow->top,
				SWP_NOZORDER | SWP_NOACTIVATE);

			m_WinSize.width = static_cast<zU64>(prcNewWindow->right - prcNewWindow->left);
			m_WinSize.height = static_cast<zU64>(prcNewWindow->bottom - prcNewWindow->top);
			OnResize(m_WinSize, eTypeWinResize::Resize);

			return 0;
		}

		case WM_MOUSEMOVE:
			if (!mouseInside)
			{
				mouseInside = true;
				OnMouseEnter(true);

				TRACKMOUSEEVENT tme = {};
				tme.cbSize = sizeof(tme);
				tme.dwFlags = TME_LEAVE;
				tme.hwndTrack = hWnd;
				TrackMouseEvent(&tme);
			}

			break;

		case WM_MOUSELEAVE:
			mouseInside = false;
			OnMouseEnter(false);
			return 0;

		case WM_SETFOCUS:
			OnFocus(true);
			break;

		case WM_KILLFOCUS:
			OnFocus(false);
			break;

		case WM_ACTIVATE:
			b_IsWinActive = (wParam != 0);
			OnActivate(b_IsWinActive);
			return 0;

		case WM_INPUT:
			OnRawInput(reinterpret_cast<HRAWINPUT>(lParam));
			return 0;
		}

		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}

	void AppWin_MSWin::SetCaptionText(std::wstring caption)
	{
		SetWindowText(hWnd, m_Config->GetAppName().c_str());
	}

	void AppWin_MSWin::AddCaptionText(std::wstring caption)
	{
		SetWindowText(hWnd, (m_Config->GetAppName() + caption).c_str());
	}

	int AppWin_MSWin::InitRawInput()
	{
		// Массив из двух устройств: мышь и клавиатура
		RAWINPUTDEVICE rid[2];
		ZeroMemory(rid, sizeof(rid));

		// Мышь
		rid[0].usUsagePage = HID_USAGE_PAGE_GENERIC;
		rid[0].usUsage = HID_USAGE_GENERIC_MOUSE;
		rid[0].dwFlags = 0;
		rid[0].hwndTarget = hWnd;

		// Клавиатура
		rid[1].usUsagePage = HID_USAGE_PAGE_GENERIC;
		rid[1].usUsage = HID_USAGE_GENERIC_KEYBOARD;
		rid[1].dwFlags = 0; // или RIDEV_NOLEGACY
		rid[1].hwndTarget = hWnd;

		if (!RegisterRawInputDevices(rid, 2, sizeof(RAWINPUTDEVICE)))
		{
			DWORD err = GetLastError();

			wchar_t* sysMsg = nullptr;
			FormatMessageW(
				FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
				nullptr,
				err,
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
				reinterpret_cast<LPWSTR>(&sysMsg),
				0,
				nullptr);

			wchar_t buf[1024];

			if (sysMsg)
			{
				swprintf(buf, 1024,
					L"Raw Input registration failed!\n\n"
					L"Error code: %lu\n"
					L"Description: %s",
					err,
					sysMsg);

				LocalFree(sysMsg); // free allocated buffer
			}
			else
			{
				swprintf(buf, 1024,
					L"Raw Input registration failed!\n\n"
					L"Error code: %lu\n"
					L"Description: could not retrieve system message",
					err);
			}

			MsgBox::Error(buf);

			return -1;
		}

		return 0;
	}

	void AppWin_MSWin::OnRawInput(HRAWINPUT hRawInput)
	{
		UINT size = 0;

		GetRawInputData(hRawInput, RID_INPUT, nullptr, &size, sizeof(RAWINPUTHEADER));
		if (size == 0)
			return;

		std::vector<BYTE> buffer(size);
		if (GetRawInputData(hRawInput, RID_INPUT, buffer.data(), &size, sizeof(RAWINPUTHEADER)) != size)
			return;

		RAWINPUT* raw = reinterpret_cast<RAWINPUT*>(buffer.data());
		switch (raw->header.dwType)
		{
		case RIM_TYPEMOUSE:
			HandleRawMouse(raw->data.mouse);
			break;

		case RIM_TYPEKEYBOARD:
			HandleRawKeyboard(raw->data.keyboard);
			break;
		}
	}

	void AppWin_MSWin::HandleRawMouse(const RAWMOUSE& mouse)
	{
		// Обрабатываем сдвиг курсора(дельту)
		if (mouse.usFlags == MOUSE_MOVE_RELATIVE)
		{
			if (mouse.lLastX != 0 && mouse.lLastY != 0)
				OnMouseDelta(mouse.lLastX, mouse.lLastY);
		}

		// Обрабатываем нажатие/отпускание кнопок мыши
		{
			MouseButtonMask pressed = MouseButtonMask::None;
			MouseButtonMask released = MouseButtonMask::None;

			// Проверяем каждую кнопку и формируем маски
			if (mouse.usButtonFlags & RI_MOUSE_LEFT_BUTTON_DOWN) pressed |= MouseButtonMask::Left;
			if (mouse.usButtonFlags & RI_MOUSE_LEFT_BUTTON_UP) released |= MouseButtonMask::Left;

			if (mouse.usButtonFlags & RI_MOUSE_RIGHT_BUTTON_DOWN) pressed |= MouseButtonMask::Right;
			if (mouse.usButtonFlags & RI_MOUSE_RIGHT_BUTTON_UP) released |= MouseButtonMask::Right;

			if (mouse.usButtonFlags & RI_MOUSE_MIDDLE_BUTTON_DOWN) pressed |= MouseButtonMask::Middle;
			if (mouse.usButtonFlags & RI_MOUSE_MIDDLE_BUTTON_UP) released |= MouseButtonMask::Middle;

			if (mouse.usButtonFlags & RI_MOUSE_BUTTON_4_DOWN) pressed |= MouseButtonMask::Button4;
			if (mouse.usButtonFlags & RI_MOUSE_BUTTON_4_UP) released |= MouseButtonMask::Button4;

			if (mouse.usButtonFlags & RI_MOUSE_BUTTON_5_DOWN) pressed |= MouseButtonMask::Button5;
			if (mouse.usButtonFlags & RI_MOUSE_BUTTON_5_UP) released |= MouseButtonMask::Button5;

			if (pressed != MouseButtonMask::None || released != MouseButtonMask::None)
				OnMouseButtonsChanged(pressed, released);
		}

		// Колесо вертикальное
		if (mouse.usButtonFlags & RI_MOUSE_WHEEL)
		{
			zI32 delta = static_cast<zI32>(static_cast<SHORT>(mouse.usButtonData)) / WHEEL_DELTA;

			if (delta != 0)
				OnMouseWheelVertical(delta);
		}

		// Колесо горизонтальное (боковое колесо)
		if (mouse.usButtonFlags & RI_MOUSE_HWHEEL)
		{
			zI32 delta = static_cast<zI32>(static_cast<SHORT>(mouse.usButtonData)) / WHEEL_DELTA;

			if (delta != 0)
				OnMouseWheelHorizontal(delta);
		}
	}

	void AppWin_MSWin::HandleRawKeyboard(const RAWKEYBOARD& kb)
	{
		const bool pressed = !(kb.Flags & RI_KEY_BREAK);
		UINT vk = kb.VKey;

		// Прямая рекомендация Microsoft
		if (vk == 255)
			return;

		bool e0 = (kb.Flags & RI_KEY_E0) != 0;
		KeyCode key = TranslateMSWinKey(vk, e0);
		KeyState state = pressed ? KeyState::Down : KeyState::Up;

		OnKeyStateChanged(key, state);
	}
}