#include "WinMSWindowEditor.h"
#include "../Platform.h"

using namespace zzz::core;
using namespace zzz::engine;

WinMSWindowEditor::WinMSWindowEditor(const Platform& platform, const std::shared_ptr<Input> input, WindowCallbacks callbacks) :
	WindowBase(platform, input, std::move(callbacks))
{
}

WinMSWindowEditor::~WinMSWindowEditor()
{
}

[[nodiscard]] std::expected<void, std::string> WinMSWindowEditor::Initialize(const StartViewPlatformData& /*settings*/, void* data)
{
	ensure(data != nullptr, "Данные инициализации окна (hwnd) не должны быть null.");
	m_hWnd = reinterpret_cast<HWND>(data);
	return {};
}
