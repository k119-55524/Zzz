#include "pch.h"
export module zMsgBox;

export namespace zzz
{
	class zMsgBox
	{
	public:
		static void Error(const std::wstring& message);
	};

	void zMsgBox::Error(const std::wstring& message)
	{
#if defined(_WIN64)
		::MessageBoxW(NULL, message.empty() ? L"No message text. Unknown error." : message.c_str(), L"Error!!!", MB_ICONERROR | MB_OK);
#else
# error ">>>>> [MessageBox.Error( ... )]. Compile error. This branch requires implementation for the current platform"
#endif
	}
}