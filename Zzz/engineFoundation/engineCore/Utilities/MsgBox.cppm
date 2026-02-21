
export module MsgBox;

import DebugOutput;

export namespace zzz
{
	class MsgBox
	{
	public:
		static void Error(const std::wstring& message);
	};

	void MsgBox::Error(const std::wstring& message)
	{
		DOut(message);

#if defined(_WIN64)
		::MessageBoxW(GetActiveWindow(), message.empty() ? L"No message text. Unknown error." : message.c_str(), L"Error!!!", MB_ICONERROR | MB_OK);
#else
# error ">>>>> [MessageBox.Error( ... )]. Compile error. This branch requires implementation for the current platform"
#endif
	}
}