#include "WinMSWindowEditor.h"
#include "../Platform.h"

using namespace zzz::common;
using namespace zzz::engine;

WinMSWindowEditor::WinMSWindowEditor(const Platform& platform, const std::shared_ptr<Input> input, WindowCallbacks callbacks) :
	WindowBase(platform, input, std::move(callbacks)),
	m_hWnd{ nullptr }
{
}

WinMSWindowEditor::~WinMSWindowEditor()
{
	m_hWnd = nullptr;
}

[[nodiscard]] std::expected<void, std::string> WinMSWindowEditor::Initialize(std::string_view /*appName*/)
{
	return {};
}
