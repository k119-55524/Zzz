
#include "InputMSWindows.h"
#include "../../core/utils/converters.h"

using namespace zzz::engine;

std::expected<void, std::string> InputMSWindows::Initialize()
{
	return {};
}

bool InputMSWindows::ProcessMessage(const NativeMsg& msg)
{
	switch (msg.uMsg)
	{
	case WM_NCCREATE:
		return InitRawInput(msg.hWnd);
	}

	return false;
}

int InputMSWindows::InitRawInput(HWND hWnd)
{
	// Массив из двух устройств: мышь и клавиатура
	RAWINPUTDEVICE rid[2];
	ZeroMemory(rid, sizeof(rid));

	// Мышь
	rid[0].usUsagePage = HID_USAGE_PAGE_GENERIC;
	rid[0].usUsage = HID_USAGE_GENERIC_MOUSE;
	rid[0].dwFlags = 0;
	rid[0].hwndTarget = hWnd;

	// Клавиатура
	rid[1].usUsagePage = HID_USAGE_PAGE_GENERIC;
	rid[1].usUsage = HID_USAGE_GENERIC_KEYBOARD;
	rid[1].dwFlags = 0; // или RIDEV_NOLEGACY
	rid[1].hwndTarget = hWnd;

	if (!RegisterRawInputDevices(rid, 2, sizeof(RAWINPUTDEVICE)))
	{
		DWORD err = GetLastError();

		wchar_t* sysMsg = nullptr;
		FormatMessageW(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			nullptr,
			err,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			reinterpret_cast<LPWSTR>(&sysMsg),
			0,
			nullptr);

		std::string message;
		if (sysMsg)
		{
			message = std::format("Error({}): {}", err, zzz::wstring_to_string(sysMsg));
			LocalFree(sysMsg);
		}
		else
			message = std::format("Error code: {}", err);

		//MsgBox::Error(err);
		THROW_RUNTIME("Raw Input registration failed.", message);

		// Отмена создания окна
		return -1;
	}

	return 0;
}

void InputMSWindows::OnRawInput(HRAWINPUT hRawInput)
{
	UINT size = 0;

	GetRawInputData(hRawInput, RID_INPUT, nullptr, &size, sizeof(RAWINPUTHEADER));
	if (size == 0)
		return;

	std::vector<BYTE> buffer(size);
	if (GetRawInputData(hRawInput, RID_INPUT, buffer.data(), &size, sizeof(RAWINPUTHEADER)) != size)
		return;

	RAWINPUT* raw = reinterpret_cast<RAWINPUT*>(buffer.data());
	switch (raw->header.dwType)
	{
	case RIM_TYPEMOUSE:
		HandleRawMouse(raw->data.mouse);
		break;

	case RIM_TYPEKEYBOARD:
		HandleRawKeyboard(raw->data.keyboard);
		break;
	}
}

void InputMSWindows::HandleRawMouse(const RAWMOUSE& mouse)
{
	// Обрабатываем сдвиг курсора(дельту)
	if (mouse.usFlags == MOUSE_MOVE_RELATIVE)
	{
		if (mouse.lLastX != 0 && mouse.lLastY != 0)
			OnMouseDelta(mouse.lLastX, mouse.lLastY);
	}

	// Обрабатываем нажатие/отпускание кнопок мыши
	{
		MouseButtonMask pressed = MouseButtonMask::None;
		MouseButtonMask released = MouseButtonMask::None;

		// Проверяем каждую кнопку и формируем маски
		if (mouse.usButtonFlags & RI_MOUSE_LEFT_BUTTON_DOWN) pressed |= MouseButtonMask::Left;
		if (mouse.usButtonFlags & RI_MOUSE_LEFT_BUTTON_UP) released |= MouseButtonMask::Left;

		if (mouse.usButtonFlags & RI_MOUSE_RIGHT_BUTTON_DOWN) pressed |= MouseButtonMask::Right;
		if (mouse.usButtonFlags & RI_MOUSE_RIGHT_BUTTON_UP) released |= MouseButtonMask::Right;

		if (mouse.usButtonFlags & RI_MOUSE_MIDDLE_BUTTON_DOWN) pressed |= MouseButtonMask::Middle;
		if (mouse.usButtonFlags & RI_MOUSE_MIDDLE_BUTTON_UP) released |= MouseButtonMask::Middle;

		if (mouse.usButtonFlags & RI_MOUSE_BUTTON_4_DOWN) pressed |= MouseButtonMask::Button4;
		if (mouse.usButtonFlags & RI_MOUSE_BUTTON_4_UP) released |= MouseButtonMask::Button4;

		if (mouse.usButtonFlags & RI_MOUSE_BUTTON_5_DOWN) pressed |= MouseButtonMask::Button5;
		if (mouse.usButtonFlags & RI_MOUSE_BUTTON_5_UP) released |= MouseButtonMask::Button5;

		if (pressed != MouseButtonMask::None || released != MouseButtonMask::None)
			OnMouseButtonsChanged(pressed, released);
	}

	// Колесо вертикальное
	if (mouse.usButtonFlags & RI_MOUSE_WHEEL)
	{
		zI32 delta = static_cast<zI32>(static_cast<SHORT>(mouse.usButtonData)) / WHEEL_DELTA;

		if (delta != 0)
			OnMouseWheelVertical(delta);
	}

	// Колесо горизонтальное (боковое колесо)
	if (mouse.usButtonFlags & RI_MOUSE_HWHEEL)
	{
		zI32 delta = static_cast<zI32>(static_cast<SHORT>(mouse.usButtonData)) / WHEEL_DELTA;

		if (delta != 0)
			OnMouseWheelHorizontal(delta);
	}
}

void InputMSWindows::HandleRawKeyboard(const RAWKEYBOARD& kb)
{
	const bool pressed = !(kb.Flags & RI_KEY_BREAK);
	UINT vk = kb.VKey;

	// Прямая рекомендация Microsoft
	if (vk == 255)
		return;

	bool e0 = (kb.Flags & RI_KEY_E0) != 0;
	KeyCode key = TranslateMSWinKey(vk, e0);
	KeyState state = pressed ? KeyState::Down : KeyState::Up;

	OnKeyStateChanged(key, state);
}