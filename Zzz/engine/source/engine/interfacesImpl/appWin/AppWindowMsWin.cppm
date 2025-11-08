#include "pch.h"
export module AppWindowMsWin;

import event;
import size2D;
import result;
import IAppWin;
import ibMSWin;
import Settings;
import IOPathFactory;

using namespace zzz;
using namespace zzz::io;
using namespace zzz::platforms;
using namespace zzz::icoBuilder;

export namespace zzz
{
	export class AppWindowMsWin final : public IAppWin
	{
	public:
		AppWindowMsWin() = delete;
		AppWindowMsWin(const AppWindowMsWin&) = delete;
		AppWindowMsWin(AppWindowMsWin&&) = delete;
		AppWindowMsWin& operator=(const AppWindowMsWin&) = delete;
		AppWindowMsWin& operator=(AppWindowMsWin&&) = delete;

		explicit AppWindowMsWin(std::shared_ptr<Settings> _settings);

		~AppWindowMsWin() override;

		const HWND GetHWND() const noexcept override { return hWnd; }
		void SetCaptionText(std::wstring caption) override;
		void AddCaptionText(std::wstring caption) override;

	protected:
		virtual result<> Initialize() override;

	private:
		HWND hWnd;
		bool IsMinimized;

		static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept;
		LRESULT MsgProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
	};

	AppWindowMsWin::AppWindowMsWin(std::shared_ptr<Settings> _settings) :
		IAppWin(_settings),
		hWnd{ nullptr },
		IsMinimized{ true }
	{
		SetProcessDPIAware();
	}

	AppWindowMsWin::~AppWindowMsWin()
	{
		if (hWnd)
			DestroyWindow(hWnd);
	}

	result<> AppWindowMsWin::Initialize()
	{
#pragma region Получение настроек окна
		std::wstring ClassName;
		{
			auto res = m_Settings->GetParam<std::wstring>(L"Caption")
				.and_then([&](std::wstring name) { m_Caption = name; });

			if (!res)
				return Unexpected(eResult::failure, L">>>>> [SW_MSWindows.Initialize( ... )]. GetParam( ... ). Failed to get 'Caption' parameter.");
		}

		m_Settings->GetParam<std::wstring>(L"MSWin_SpecSettings", L"ClassName")
			.and_then([&](std::wstring name) {ClassName = name; })
			.or_else([&](auto error) { ClassName = m_Caption; });

		LONG swWidth;
		auto res = m_Settings->GetParam<int>(L"Width")
			.and_then([&](int width) {swWidth = width; });
		if (!res)
			return Unexpected(eResult::failure, L">>>>> [SW_MSWindows.Initialize( ... )]. GetParam( ... ). Failed to get 'Width' parameter. More specifically: " + res.error().getMessage());

		LONG swHeight;
		res = m_Settings->GetParam<int>(L"Height")
			.and_then([&](int height) {swHeight = height; });
		if (!res)
			return Unexpected(eResult::failure, L">>>>> [SW_MSWindows.Initialize( ... )]. GetParam( ... ). Failed to get 'Height' parameter. More specifically:" + res.error().getMessage());

		HICON iconHandle = nullptr;
		{
			result<std::wstring> icoPath = m_Settings->GetParam<std::wstring>(L"IcoFullPath");
			if (icoPath)
			{
				ibMSWin icoBuilder;
				auto res = icoBuilder.LoadIco(icoPath.value(), m_Settings->GetParam<int>(L"IcoSize").value_or(32));
				if (res)
					iconHandle = res.value();
			}
		}
#pragma endregion Получение настроек окна

		WNDCLASS wc = { 0 };
		wc.style = CS_HREDRAW | CS_VREDRAW;
		wc.lpfnWndProc = AppWindowMsWin::WindowProc;
		wc.hInstance = GetModuleHandle(NULL);
		wc.hIcon = iconHandle;// LoadIcon(GetModuleHandle(NULL), NULL);// MAKEINTRESOURCE(userGS->GetMSWinIcoID()));
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
			m_Caption.c_str(),
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

	LRESULT CALLBACK AppWindowMsWin::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept
	{
		AppWindowMsWin* pThis = nullptr;

		try
		{
			if (uMsg == WM_NCCREATE)
			{
				const auto* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
				if (!pCreate || !pCreate->lpCreateParams)
					return FALSE; // Ошибка создания

				pThis = static_cast<AppWindowMsWin*>(pCreate->lpCreateParams);
				SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
				pThis->hWnd = hwnd;
			}
			else
				pThis = reinterpret_cast<AppWindowMsWin*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

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

	LRESULT AppWindowMsWin::MsgProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		switch (uMsg)
		{
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;

		case WM_SIZE:
			winSize.width = static_cast<zU64>(LOWORD(lParam));
			winSize.height = static_cast<zU64>(HIWORD(lParam));
			if (wParam == SIZE_MINIMIZED)
			{
				onResize(winSize, eTypeWinResize::eHide);
				IsMinimized = true;
			}
			else
			{
				if ((wParam == SIZE_RESTORED || wParam == SIZE_MAXIMIZED) && IsMinimized)
				{
					onResize(winSize, eTypeWinResize::eShow);
					IsMinimized = false;
				}
				else
				{
					onResize(winSize, eTypeWinResize::eResize);
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

			winSize.width = static_cast<zU64>(prcNewWindow->right - prcNewWindow->left);
			winSize.height = static_cast<zU64>(prcNewWindow->bottom - prcNewWindow->top);
			onResize(winSize, eTypeWinResize::eResize);

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

	void AppWindowMsWin::SetCaptionText(std::wstring caption)
	{
		m_Caption = caption;
		SetWindowText(hWnd, m_Caption.c_str());
	}

	void AppWindowMsWin::AddCaptionText(std::wstring caption)
	{
		std::wstring cap = m_Caption + caption;
		SetWindowText(hWnd, cap.c_str());
	}
}