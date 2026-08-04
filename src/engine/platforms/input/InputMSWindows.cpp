
#include "InputMSWindows.h"
#include <core/utils/converters.h>

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

	case WM_MOUSEMOVE:
		if (!m_IsMouseInside)
		{
			m_IsMouseInside = true;
			OnMouseEnter(true);

			TRACKMOUSEEVENT tme = {};
			tme.cbSize = sizeof(tme);
			tme.dwFlags = TME_LEAVE;
			tme.hwndTrack = msg.hWnd;
			TrackMouseEvent(&tme);
		}

		break;

	case WM_MOUSELEAVE:
	{
		m_IsMouseInside = false;
		OnMouseEnter(false);

		break;
	}

	case WM_INPUT:
		OnRawInput(reinterpret_cast<HRAWINPUT>(msg.lParam));

		break;
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
			message = std::format("Ошибка({}): {}", err, zzz::wstring_to_string(sysMsg));
			LocalFree(sysMsg);
		}
		else
			message = std::format("Код ошибки: {}", err);

		//MsgBox::Error(err);
		THROW_RUNTIME("Не удалось зарегистрировать Raw Input. {}", message);

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
		if (mouse.lLastX != 0 || mouse.lLastY != 0)
			OnMouseDelta(mouse.lLastX, mouse.lLastY);
	}

	// Обрабатываем нажатие/отпускание кнопок мыши
	{
		if (mouse.usButtonFlags & RI_MOUSE_LEFT_BUTTON_DOWN) UpdateMouseButtonState(MouseButton::Left, true);
		if (mouse.usButtonFlags & RI_MOUSE_LEFT_BUTTON_UP) UpdateMouseButtonState(MouseButton::Left, false);

		if (mouse.usButtonFlags & RI_MOUSE_RIGHT_BUTTON_DOWN) UpdateMouseButtonState(MouseButton::Right, true);
		if (mouse.usButtonFlags & RI_MOUSE_RIGHT_BUTTON_UP) UpdateMouseButtonState(MouseButton::Right, false);

		if (mouse.usButtonFlags & RI_MOUSE_MIDDLE_BUTTON_DOWN) UpdateMouseButtonState(MouseButton::Middle, true);
		if (mouse.usButtonFlags & RI_MOUSE_MIDDLE_BUTTON_UP) UpdateMouseButtonState(MouseButton::Middle, false);

		if (mouse.usButtonFlags & RI_MOUSE_BUTTON_4_DOWN) UpdateMouseButtonState(MouseButton::Button4, true);
		if (mouse.usButtonFlags & RI_MOUSE_BUTTON_4_UP) UpdateMouseButtonState(MouseButton::Button4, false);

		if (mouse.usButtonFlags & RI_MOUSE_BUTTON_5_DOWN) UpdateMouseButtonState(MouseButton::Button5, true);
		if (mouse.usButtonFlags & RI_MOUSE_BUTTON_5_UP) UpdateMouseButtonState(MouseButton::Button5, false);
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
	KeyCode key = TranslateMSWinKey(vk, e0, kb.MakeCode);
	UpdateKeyState(key, pressed);
}
