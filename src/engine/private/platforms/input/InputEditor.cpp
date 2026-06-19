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
}

void InputEditor::InjectKeyUp(int /*key*/)
{
}

void InputEditor::InjectMouseMove(int /*x*/, int /*y*/)
{
}

void InputEditor::InjectMouseButtonDown(int /*button*/)
{
}

void InputEditor::InjectMouseButtonUp(int /*button*/)
{
}
