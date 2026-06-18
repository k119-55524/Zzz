#include "WinEditor.h"
#include "../Platform.h"

using namespace zzz::common;
using namespace zzz::engine;

WinEditor::WinEditor(const Platform& platform, const std::shared_ptr<Input> input, WindowCallbacks callbacks) :
	WindowBase(platform, input, std::move(callbacks)),
	m_hWnd{ nullptr }
{
}

WinEditor::~WinEditor()
{
	m_hWnd = nullptr;
}

[[nodiscard]] std::expected<void, std::string> WinEditor::Initialize(std::string_view /*appName*/)
{
	auto nativeData = m_Platform.GetNativeData();
	if (!nativeData || !nativeData->hwnd)
		return UNEXPECTED("WinEditor requires a valid HWND in NativeAppData.");

	m_hWnd = nativeData->hwnd;

	// Передаем m_hWnd наверх (во View/Engine), чтобы графическое API (Vulkan/DirectX)
	// могло привязаться к этому окну и создать Swapchain. Без этого рендеринг невозможен.
	VERIFY_AND_CALL(m_Callbacks.OnSurfaceCreated, m_hWnd);

	return {};
}
