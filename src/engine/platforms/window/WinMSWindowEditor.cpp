#include "core/utils/Defines.h"

#if defined(Z_EDITOR)

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

std::expected<void, std::string> WinMSWindowEditor::Initialize(const ViewPlatformData& /*settings*/, const View* /*parentView*/)
{
	return {};
}

#endif // defined(Z_EDITOR)
