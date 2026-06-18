#include "InputEditor.h"

using namespace zzz::engine;

InputEditor::InputEditor()
{
}

InputEditor::~InputEditor()
{
}

[[nodiscard]] std::expected<void, std::string> InputEditor::Initialize()
{
	return {};
}

void InputEditor::InjectKeyDown(int /*key*/)
{
	// TODO: маппинг WPF -> KeyCode и вызов UpdateKeyState(code, true)
}

void InputEditor::InjectKeyUp(int /*key*/)
{
	// TODO: маппинг WPF -> KeyCode и вызов UpdateKeyState(code, false)
}

void InputEditor::InjectMouseMove(int /*x*/, int /*y*/)
{
	// TODO: вызов OnMouseDelta(...)
}

void InputEditor::InjectMouseButtonDown(int /*button*/)
{
}

void InputEditor::InjectMouseButtonUp(int /*button*/)
{
}
