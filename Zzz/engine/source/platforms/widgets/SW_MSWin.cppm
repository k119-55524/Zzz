#include "pch.h"
export module SW_MSWin;

#ifdef _WIN64
import result;
import zSize2D;
import SWSettings;
import ISuperWidget;

using namespace zzz;
using namespace zzz::result;
using namespace zzz::platforms;

export namespace zzz::platforms
{
	class SW_MSWin final : public ISuperWidget
	{
	public:
		SW_MSWin() = delete;
		SW_MSWin(SW_MSWin&) = delete;
		SW_MSWin(SW_MSWin&&) = delete;

		SW_MSWin(const std::function<void(const zSize2D<>& size, e_TypeWinAppResize resType)> _resizeWindows);
		~SW_MSWin() override;

	protected:
		zResult<> Init() override;

	private:
		HWND hWnd;
		bool IsMinimized;

		static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		LRESULT MsgProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
	};

	SW_MSWin::SW_MSWin(const std::function<void(const zSize2D<>& size, e_TypeWinAppResize resType)> _resizeWindows) :
		ISuperWidget(_resizeWindows),
		hWnd{ nullptr },
		IsMinimized{ true }
	{
		//SetProcessDPIAware();
	}

	SW_MSWin::~SW_MSWin()
	{
		if (hWnd)
			DestroyWindow(hWnd);
	}

	zResult<> SW_MSWin::Init()
	{
		std::wstring className = L"zEngineClassName";

		WNDCLASS wc = { 0 };
		wc.style = CS_HREDRAW | CS_VREDRAW;
		wc.lpfnWndProc = SW_MSWin::WindowProc;
		wc.hInstance = GetModuleHandle(NULL);
		wc.hIcon = LoadIcon(GetModuleHandle(NULL), NULL);// MAKEINTRESOURCE(userGS->GetMSWinIcoID()));
		wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
		wc.lpszClassName = className.c_str();

		ATOM result = RegisterClass(&wc);
		if (result == 0)
		{
			std::wstring mess = L">>>>> [SW_MSWindows.Initialize( ... )]. RegisterClass( ... ). Failed to register window class: ";
			mess += className;
			mess += L"\n+--- error code(Windows): ";
			mess += std::to_wstring(::GetLastError());
			return Unexpected(eResult::failure, mess);
		}

		// Рассчитать размеры прямоугольника окна на основе запрошенных размеров клиентской области.
		RECT R = { 0, 0, static_cast<LONG>(640), static_cast<LONG>(480) };
		AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false);
		int width = R.right - R.left;
		int height = R.bottom - R.top;

		int screenWidth = GetSystemMetrics(SM_CXSCREEN);  // Ширина экрана
		int screenHeight = GetSystemMetrics(SM_CYSCREEN); // Высота экрана
		int xPos = (screenWidth - width) / 2;  // Расчет позиции по оси X
		int yPos = (screenHeight - height) / 2; // Расчет позиции по оси Y
		hWnd = CreateWindowEx(
			0,
			className.c_str(),
			L"Game",
			WS_OVERLAPPEDWINDOW,
			xPos, yPos, width, height,
			nullptr,
			nullptr,
			GetModuleHandle(NULL),
			this);

		if (!hWnd)
		{
			std::wstring mess = L">>>>> [SW_MSWindows.Initialize( ... )]. CreateWindowEx( ... ). Failed to create window: ";
			mess += L"\n +--- error code(Windows): ";
			mess += std::to_wstring(::GetLastError());
			return Unexpected(eResult::failure, mess);
		}

		ShowWindow(hWnd, SW_SHOW);
		UpdateWindow(hWnd);

		return {};
	}

	LRESULT CALLBACK SW_MSWin::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		SW_MSWin* pThis;

		if (uMsg == WM_NCCREATE)
		{
			CREATESTRUCT* pCreate = (CREATESTRUCT*)lParam;
			pThis = (SW_MSWin*)pCreate->lpCreateParams;
			SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pThis);

			pThis->hWnd = hwnd;
		}
		else
			pThis = (SW_MSWin*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

		if (pThis)
			return pThis->MsgProc(uMsg, wParam, lParam);

		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}

	LRESULT SW_MSWin::MsgProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		switch (uMsg)
		{
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;

		case WM_SIZE:
			winSize.width = static_cast<zU64>(LOWORD(lParam));
			winSize.height = static_cast<zU64>(HIWORD(lParam));
			if (resizeWindows != nullptr)
			{
				if (wParam == SIZE_MINIMIZED)
				{
					resizeWindows(winSize, e_TypeWinAppResize::eHide);
					IsMinimized = true;
				}
				else
				{
					if ((wParam == SIZE_RESTORED || wParam == SIZE_MAXIMIZED) && IsMinimized)
					{
						resizeWindows(winSize, e_TypeWinAppResize::eShow);
						IsMinimized = false;
					}
					else
					{
						resizeWindows(winSize, e_TypeWinAppResize::eResize);
					}
				}
			}
			return 0;

			// Перехватываем это сообщение, чтобы не допустить слишком маленького/большого размера окна.
		case WM_GETMINMAXINFO:
		{
			((MINMAXINFO*)lParam)->ptMinTrackSize.x = c_MinimumWindowsWidth;
			((MINMAXINFO*)lParam)->ptMinTrackSize.y = c_MinimumWindowsHeight;

			RECT R = { 0, 0, static_cast<LONG>(c_MaximumWindowsWidth), static_cast<LONG>(c_MaximumWindowsHeight) };
			AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false);
			LONG maxWidth = R.right - R.left;
			LONG maxHeight = R.bottom - R.top;
			((MINMAXINFO*)lParam)->ptMaxTrackSize.x = maxWidth;
			((MINMAXINFO*)lParam)->ptMaxTrackSize.y = maxHeight;
			return 0;
		}

		// брабатываем изменение DPI в системе
		case WM_DPICHANGED:
		{
			// Получаем новое значение DPI
			int newDPI = LOWORD(wParam);

			// Устанавливаем новый размер окна, если это необходимо
			RECT* prc = (RECT*)lParam;
			SetWindowPos(hWnd, NULL, prc->left, prc->top,
				prc->right - prc->left, prc->bottom - prc->top,
				SWP_NOZORDER | SWP_NOACTIVATE);
			return 0;
		}

		//case WM_PAINT:
		//{
		//	PAINTSTRUCT ps;
		//	HDC hdc = BeginPaint(hWnd, &ps);
		//	// Здесь можно выполнить рисование, если это необходимо.
		//	EndPaint(hWnd, &ps);
		//	return 0;
		//}

		default:
			return DefWindowProc(hWnd, uMsg, wParam, lParam);
		}
	}
}
#endif // _WIN64