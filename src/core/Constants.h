#pragma once

#include <array>
#include <common/Common.h>
#include <core/IO/FileHeader.h>

using namespace zzz::io;

namespace zzz::common
{
#pragma region Config file constants
	constexpr std::string_view c_ConfigFileName = "user.dat";
	constexpr FileHeader<3> c_ConfigHeader
	{
		std::array<std::byte, 3>{
			static_cast<std::byte>('Z'),
			static_cast<std::byte>('Z'),
			static_cast<std::byte>('Z')
		}
	};
	constexpr zU8 c_ConfigFileMajorVersion = 1;
	constexpr zU8 c_ConfigFileMinorVersion = 0;
	constexpr zU8 c_ConfigFilePatchVersion = 0;

	constexpr zU32 c_DefaultWindowWidth = 800;
	constexpr zU32 c_DefaultWindowHeicht = 600;
#pragma endregion

#pragma region Game Package file constants
	constexpr std::string_view c_GamePackageFileName = "package.dat";
	constexpr FileHeader<3> c_GamePackageHeader
	{
		std::array<std::byte, 3>{
			static_cast<std::byte>('Z'),
			static_cast<std::byte>('Z'),
			static_cast<std::byte>('P')
		}
	};
	constexpr zU8 c_GamePackageFileMajorVersion = 1;
	constexpr zU8 c_GamePackageFileMinorVersion = 0;
	constexpr zU8 c_GamePackageFilePatchVersion = 0;
#pragma endregion

#pragma region Microsoft Windows constants
	constexpr std::string_view c_IcoResourceName = "IDI_ICON1";
	constexpr std::string_view c_RegisterClassName = "ZzzEngineWindowClass";
#pragma endregion

#pragma region Network constants
	constexpr std::string_view c_LocalhostIPv4 = "127.0.0.1";
	constexpr zU16 c_DefaultLoggerPort = 3030;
#pragma endregion
}