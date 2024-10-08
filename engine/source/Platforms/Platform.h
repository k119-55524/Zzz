#pragma once

#include "pch.h"
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
#elif defined(_WINDOWS)
			// ...
			return e_PlatformType::Windows;
#else
			return e_PlatformType::Unknow;
#endif
		}

	private:
		unique_ptr<ISysMB> messageBox;
	};
}