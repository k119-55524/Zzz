#pragma once

#include <array>
#include <foundation.h>

namespace zzz
{
#pragma region Config file constants
	constexpr std::string_view c_ConfigFileName = "config.dat";
	constexpr std::array<std::byte, 3> c_ConfigHeader
	{
		static_cast<std::byte>(0x5A),	// 'Z'
		static_cast<std::byte>(0x5A),	// 'Z'
		static_cast<std::byte>(0x5A)	// 'Z'
	};
	constexpr zU8 c_ConfigFileMajorVersion = 1;
	constexpr zU8 c_ConfigFileMinorVersion = 0;
	constexpr zU8 c_ConfigFilePatchVersion = 0;

	constexpr zU32 c_DefaultWindowWidth = 800;
	constexpr zU32 c_DefaultWindowHeicht = 600;
#pragma endregion

#pragma region Microsoft Windows constants

#pragma endregion
}