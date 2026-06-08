#if defined(Z_WINDOWS)

#include "PlatformMSWindows.h"
#include "../platforms/native_view/window/WinMSWindows.h"
#include "../../core/config/platforms/ConfigMSWin.h"
#include "../../inputs/platforms/InputMSWindows.h"

using namespace zzz::engine;

PlatformMSWindows::PlatformMSWindows(std::string_view appName, std::shared_ptr<PlatformNativeData> platformData) :
	IPlatform(appName, platformData)
{
	SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
}

PlatformMSWindows::~PlatformMSWindows()
{
	const BOOL result = UnregisterClass(static_cast<const ConfigMSWin&>(m_ConfigManager->GetPlatformConfig()).GetClassName().c_str(), GetModuleHandle(nullptr));
	if (!result)
	{
		const DWORD error = GetLastError();
		DOutCritical("Failed to unregister window class '{}'. Error code: {}.", static_cast<const ConfigMSWin&>(m_ConfigManager->GetPlatformConfig()).GetClassName(), error);
	}
}

void PlatformMSWindows::InitializeImpl()
{
	HICON iconHandle = (HICON)LoadImage(
		GetModuleHandle(NULL),
		static_cast<const ConfigMSWin&>(m_ConfigManager->GetPlatformConfig()).GetIcoResourceName().c_str(),
		IMAGE_ICON,
		0,
		0,
		LR_DEFAULTSIZE | LR_SHARED);

	if (!iconHandle)
		DOutWarning("Failed to load icon '{}'. Error: {}", static_cast<const ConfigMSWin&>(m_ConfigManager->GetPlatformConfig()).GetIcoResourceName(), GetLastError());

	WNDCLASS wc = { 0 };
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = PlatformMSWindows::WindowProc;
	wc.hInstance = GetModuleHandle(NULL);
	wc.hIcon = iconHandle;
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpszClassName = static_cast<const ConfigMSWin&>(m_ConfigManager->GetPlatformConfig()).GetClassName().c_str();
	ATOM Result = RegisterClass(&wc);
	if (Result == 0)
		THROW_RUNTIME("Failed to register window class. Error code: {}.", GetLastError());
}

LRESULT CALLBACK PlatformMSWindows::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept
{
	struct WinInternalContext
	{
		WinMSWindows* window;
		InputMSWindows* input;
	};

	WinInternalContext* ctx = nullptr;

	if (uMsg == WM_NCCREATE)
	{
		const auto* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
		ctx = static_cast<WinInternalContext*>(pCreate->lpCreateParams);
		SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(ctx));
	}
	else
		ctx = reinterpret_cast<WinInternalContext*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

	if (ctx)
	{
		LRESULT res = ctx->window->MsgProc(hWnd, uMsg, wParam, lParam);

		// Если сообщение - это ввод, отдаем его инпуту
		// В будущем тут будет фильтр, чтобы не слать WM_SIZE в инпут
		struct WinMsg {
			UINT uMsg;
			WPARAM wParam;
			LPARAM lParam;
		} msg = { uMsg, wParam, lParam };

		ctx->input->ProcessMessage(&msg);

		return res;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}
#endif // defined(Z_WINDOWS)