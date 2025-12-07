
export module AppWin_MSWin;

import Event;
import Size2D;
import Result;
import IAppWin;
import ibMSWin;
import AppWinConfig;
import IOPathFactory;

using namespace zzz;

export namespace zzz::core
{
	export class AppWin_MSWin final : public IAppWin
	{
		Z_NO_CREATE_COPY(AppWin_MSWin);

	public:
		explicit AppWin_MSWin(std::shared_ptr<AppWinConfig> _settings);
		virtual ~AppWin_MSWin() override;

		const HWND GetHWND() const noexcept override { return hWnd; }
		void SetCaptionText(std::wstring caption) override;
		void AddCaptionText(std::wstring caption) override;

	protected:
		virtual Result<> Initialize() override;

	private:
		HWND hWnd;
		bool IsMinimized;

		static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept;
		LRESULT MsgProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
	};

	AppWin_MSWin::AppWin_MSWin(std::shared_ptr<AppWinConfig> config) :
		IAppWin(config),
		hWnd{ nullptr },
		IsMinimized{ true }
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
		wc.hIcon = iconHandle;// LoadIcon(GetModuleHandle(NULL), NULL);// MAKEINTRESOURCE(userGS->GetMSWinIcoID()));
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
			m_Config->GetCaption().c_str(),
			WS_OVERLAPPEDWINDOW,
			xPos, yPos, width, height,
			nullptr,
			nullptr,
			GetModuleHandle(NULL),
			this);

		if (!hWnd)
		{
			std::wstring mess = L">>>>> [SW_MSWindows.Initialize( ... )]. CreateWindowEx( ... ). Failed to create window. Error code(Windows): " + std::to_wstring(::GetLastError());
			return Unexpected(eResult::failure, mess);
		}

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
#ifdef _DEBUG
			DebugOutput(L">>>>> [AppWindowMsWin::WindowProc]. Exception in WindowProc!");
#endif
			return DefWindowProc(hwnd, uMsg, wParam, lParam);
		}

		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}

	LRESULT AppWin_MSWin::MsgProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		switch (uMsg)
		{
		case WM_DESTROY:
		{
			PostQuitMessage(0);
			return 0;
		}

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

		case WM_SIZING:
		{
			OnResizing();

			return TRUE;
		}

		// Перехватываем это сообщение, чтобы не допустить слишком маленького/большого размера окна.
		case WM_GETMINMAXINFO:
		{
			MINMAXINFO* pMinMaxInfo = reinterpret_cast<MINMAXINFO*>(lParam);

			DWORD dwStyle = static_cast<DWORD>(GetWindowLongPtr(hWnd, GWL_STYLE));
			DWORD dwExStyle = static_cast<DWORD>(GetWindowLongPtr(hWnd, GWL_EXSTYLE));
			BOOL bMenu = (GetMenu(hWnd) != NULL);

			// Минимальный размер клиентской области
			RECT minRect = { 0, 0, static_cast<LONG>(c_MinimumWindowsWidth), static_cast<LONG>(c_MinimumWindowsHeight) };
			AdjustWindowRectEx(&minRect, dwStyle, bMenu, dwExStyle);
			pMinMaxInfo->ptMinTrackSize.x = minRect.right - minRect.left;
			pMinMaxInfo->ptMinTrackSize.y = minRect.bottom - minRect.top;

			// Максимальный размер клиентской области
			RECT maxRect = { 0, 0, static_cast<LONG>(c_MaximumWindowsWidth), static_cast<LONG>(c_MaximumWindowsHeight) };
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

		//case WM_ENTERSIZEMOVE:
		//{
		//	IsResizeProcess = true;

		//	return 0;
		//}

		//case WM_EXITSIZEMOVE:
		//{
		//	IsResizeProcess = false;
		//	return 0;
		//}

		//case WM_PAINT:
		//{
		//	if (IsResizeProcess)
		//	{
		//		PAINTSTRUCT ps;
		//		HDC hdc = BeginPaint(hWnd, &ps);
		//		onPaint();
		//		EndPaint(hWnd, &ps);
		//	}

		//	return 0;
		//}

		default:
			return DefWindowProc(hWnd, uMsg, wParam, lParam);
		}
	}

	void AppWin_MSWin::SetCaptionText(std::wstring caption)
	{
		SetWindowText(hWnd, m_Config->GetCaption().c_str());
	}

	void AppWin_MSWin::AddCaptionText(std::wstring caption)
	{
		SetWindowText(hWnd, (m_Config->GetCaption() + caption).c_str());
	}
}