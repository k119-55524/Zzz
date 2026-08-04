#include "WinMSWindowEditor.h"
#include "../Platform.h"

using namespace zzz::common;
using namespace zzz::engine;

WinMSWindowEditor::WinMSWindowEditor(const Platform& platform, const std::shared_ptr<Input> input, WindowCallbacks callbacks) :
	WindowBase(platform, input, std::move(callbacks))
{
}

WinMSWindowEditor::~WinMSWindowEditor()
{
}

[[nodiscard]] std::expected<void, std::string> WinMSWindowEditor::Initialize(std::string_view /*appName*/, void* data)
{
	ensure(data != nullptr, "Данные инициализации окна (hwnd) не должны быть null.");
	m_hWnd = reinterpret_cast<HWND>(data);
	return {};
}
