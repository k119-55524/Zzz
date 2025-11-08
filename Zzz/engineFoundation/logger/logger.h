#pragma once

namespace zzz
{
	inline void DebugOutput(const std::wstring& msg)
	{
#if defined(_DEBUG)
#if defined(ZPLATFORM_MSWINDOWS)
		OutputDebugStringW((msg + L"\n").c_str());
#else
		std::wcerr << msg << std::endl;
#endif	// ZPLATFORM_MSWINDOWS
#endif	// _DEBUG
	}
} // namespace zzz