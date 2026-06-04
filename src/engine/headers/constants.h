#pragma once

#include <array>
#include <string_view>
#include <foundation.h>

namespace zzz
{
#pragma region Config File Constants
	constexpr std::string_view configFileName = "config.dat";
	constexpr std::array<std::byte, 3> configHeader
	{
		static_cast<std::byte>(0x5A),	// 'Z'
		static_cast<std::byte>(0x5A),	// 'Z'
		static_cast<std::byte>(0x5A)	// 'Z'
	};
	constexpr zU8 configFileMajorVersion = 1;
	constexpr zU8 configFileMinorVersion = 0;
	constexpr zU8 configFilePatchVersion = 0;
#pragma endregion
}