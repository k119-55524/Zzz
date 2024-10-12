#pragma once

#include "MessageBox/ISysMB.h"

namespace Zzz
{
	class zEngine;
}

namespace Zzz::Platforms
{
	enum e_PlatformType : zU32
	{
		Unknow,
		WindowsEditor,
		Windows,
		Android,
		MacOS,
		iOS,
		Linux,
		//NintendoSwitch,
		//XBOX,
		//PlayStation
	};

	//enum e_PlatformCurrent : zU32
	//{
	//	Unknow,
	//	Windows10,
	//	Windows11,
	//	Android,
	//	MacOS,
	//	iOS,
	//	NintendoSwitch,
	//	NintendoSwitch2,
	//	XBOX_ONE,
	//	PlayStation5,
	//	PlayStation5Pro,
	//};

	class Platform
	{
		friend class zEngine;

	public:
		Platform() = delete;
		Platform(Platform&) = delete;
		Platform(Platform&&) = delete;
		Platform(unique_ptr<ISysMB> mb);

		inline void ShowErrorMessageBox(const zStr& message) { messageBox->ShowError(message); };

		constexpr inline static e_PlatformType GetPlatformType()
		{
#ifdef WINDOWS_EDITOR
			return e_PlatformType::WindowsEditor;
#elif defined(_WINDOWS) || defined(_SERVICES) || defined(_EDITOR)
			// ...
			return e_PlatformType::Windows;
#else
			return e_PlatformType::Unknow;
#endif
		}

		static zStr StringToWstring(const string& str)
		{
			size_t length = str.length();
			wstring wstr(length + 1, L'\0');

			size_t convertedChars = 0;
			errno_t err = mbstowcs_s(&convertedChars, &wstr[0], wstr.size(), str.c_str(), length);

			if (err != 0)
				throw runtime_error(">>>>> [StringToWstring( ... )]. Conversion failed.");

			// Уменьшаем размер wstring до фактической длины
			wstr.resize(convertedChars);

			return wstr;
		}

	private:
		unique_ptr<ISysMB> messageBox;
	};
}