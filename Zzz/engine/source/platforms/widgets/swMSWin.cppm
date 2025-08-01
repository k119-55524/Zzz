#include "pch.h"
export module swMSWin;

#ifdef _WIN64
import result;
import zSize2D;
import mlMSWin;
import swSettings;
import ISuperWidget;

using namespace zzz;
using namespace zzz::result;
using namespace zzz::platforms;

export namespace zzz::platforms
{
	class swMSWin final : public ISuperWidget
	{
	public:
		swMSWin() = delete;
		swMSWin(swMSWin&) = delete;
		swMSWin(swMSWin&&) = delete;

		swMSWin(std::shared_ptr<swSettings> _settings, const std::function<void(const zSize2D<>& size, e_TypeWinAppResize resType)> _resizeSW);
		~swMSWin() override;

	protected:
		zResult<> _Initialize() override;

	private:
		HWND hWnd;
		bool IsMinimized;

		static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		LRESULT MsgProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
	};

	swMSWin::swMSWin(std::shared_ptr<swSettings> _settings, const std::function<void(const zSize2D<>& size, e_TypeWinAppResize resType)> _resizeSW) :
		ISuperWidget(_settings, _resizeSW),
		hWnd{ nullptr },
		IsMinimized{ true }
	{
		//SetProcessDPIAware();
	}

	swMSWin::~swMSWin()
	{
		if (hWnd)
			DestroyWindow(hWnd);
	}

	zResult<> swMSWin::_Initialize()
	{
		std::wstring Caption;
		std::wstring ClassName;
		{
			auto res = settings->GetParam<std::wstring>(L"caption")
				.and_then([&](std::wstring name) { Caption = name; });

			if (!res)
				return Unexpected(eResult::failure, L">>>>> [SW_MSWindows.Initialize( ... )]. GetParam( ... ). Failed to get 'Caption' parameter.");
		}

		settings->GetParam<std::wstring>(L"MSWin_SpecSettings", L"ClassName")
			.and_then([&](std::wstring name) {ClassName = name; })
			.or_else([&](auto error) { ClassName = Caption; });

		LONG swWidth;
		auto res = settings->GetParam<int>(L"width")
			.and_then([&](int width) {swWidth = width; });
		if (!res)
			return Unexpected(eResult::failure, L">>>>> [SW_MSWindows.Initialize( ... )]. GetParam( ... ). Failed to get 'width' parameter. More specifically: " + res.error().getMessage());

		LONG swHeight;
		res = settings->GetParam<int>(L"height")
			.and_then([&](int height) {swHeight = height; });
		if (!res)
			return Unexpected(eResult::failure, L">>>>> [SW_MSWindows.Initialize( ... )]. GetParam( ... ). Failed to get 'height' parameter. More specifically:" + res.error().getMessage());

		WNDCLASS wc = { 0 };
		wc.style = CS_HREDRAW | CS_VREDRAW;
		wc.lpfnWndProc = swMSWin::WindowProc;
		wc.hInstance = GetModuleHandle(NULL);
		wc.hIcon = LoadIcon(GetModuleHandle(NULL), NULL);// MAKEINTRESOURCE(userGS->GetMSWinIcoID()));
		wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
		wc.lpszClassName = ClassName.c_str();

		ATOM result = RegisterClass(&wc);
		if (result == 0)
		{
			std::wstring mess = L">>>>> [SW_MSWindows.Initialize( ... )]. RegisterClass( ... ). Failed to register window class: " + ClassName + L". Error code(MSWindows):" + std::to_wstring(::GetLastError());
			return Unexpected(eResult::failure, mess);
		}

		// Рассчитать размеры прямоугольника окна на основе запрошенных размеров клиентской области.
		RECT R = { 0, 0, static_cast<LONG>(swWidth), static_cast<LONG>(swHeight) };
		AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false);
		int width = R.right - R.left;
		int height = R.bottom - R.top;

		int screenWidth = GetSystemMetrics(SM_CXSCREEN);  // Ширина экрана
		int screenHeight = GetSystemMetrics(SM_CYSCREEN); // Высота экрана
		int xPos = (screenWidth - width) / 2;  // Расчет позиции по оси X
		int yPos = (screenHeight - height) / 2; // Расчет позиции по оси Y
		hWnd = CreateWindowEx(
			0,
			ClassName.c_str(),
			Caption.c_str(),
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

	LRESULT CALLBACK swMSWin::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		swMSWin* pThis;

		if (uMsg == WM_NCCREATE)
		{
			CREATESTRUCT* pCreate = (CREATESTRUCT*)lParam;
			pThis = (swMSWin*)pCreate->lpCreateParams;
			SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pThis);

			pThis->hWnd = hwnd;
		}
		else
			pThis = (swMSWin*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

		if (pThis)
			return pThis->MsgProc(uMsg, wParam, lParam);

		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}

	LRESULT swMSWin::MsgProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		switch (uMsg)
		{
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;

		case WM_SIZE:
			winSize.width = static_cast<zU64>(LOWORD(lParam));
			winSize.height = static_cast<zU64>(HIWORD(lParam));
			if (callbackResizeSW != nullptr)
			{
				if (wParam == SIZE_MINIMIZED)
				{
					callbackResizeSW(winSize, e_TypeWinAppResize::eHide);
					IsMinimized = true;
				}
				else
				{
					if ((wParam == SIZE_RESTORED || wParam == SIZE_MAXIMIZED) && IsMinimized)
					{
						callbackResizeSW(winSize, e_TypeWinAppResize::eShow);
						IsMinimized = false;
					}
					else
					{
						callbackResizeSW(winSize, e_TypeWinAppResize::eResize);
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