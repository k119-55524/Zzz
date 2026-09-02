#include "MotherboardInfoCollectorMSWin.h"

#if defined(Z_WINDOWS)

#include "engine/platforms/hardware/platforms/common/SmbiosReaderMSWin.h"
#include "engine/platforms/hardware/platforms/common/WCharUtilsMSWin.h"

using namespace zzz::engine;
using namespace zzz::core;

MotherboardInfo MotherboardInfoCollectorMSWin::Collect() const
{
	std::string mbVendor = "Unknown";
	std::string mbModel = "Unknown";
	std::string sysUuid = "00000000-0000-0000-0000-000000000000";

	// 1. Материнская плата (Windows Registry)
	HKEY hKey = nullptr;
	if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"HARDWARE\\DESCRIPTION\\System\\BIOS", 0, KEY_READ, &hKey) == ERROR_SUCCESS)
	{
		WCHAR buffer[256]{};
		DWORD bufSize = sizeof(buffer);

		if (RegQueryValueExW(hKey, L"BaseBoardManufacturer", nullptr, nullptr, reinterpret_cast<LPBYTE>(buffer), &bufSize) == ERROR_SUCCESS)
		{
			std::string s = WCharToUtf8MSWin(buffer);
			if (!s.empty()) mbVendor = std::move(s);
		}

		bufSize = sizeof(buffer);
		if (RegQueryValueExW(hKey, L"BaseBoardProduct", nullptr, nullptr, reinterpret_cast<LPBYTE>(buffer), &bufSize) == ERROR_SUCCESS)
		{
			std::string s = WCharToUtf8MSWin(buffer);
			if (!s.empty()) mbModel = std::move(s);
		}

		// Примечание при переносе (1:1 с прежним поведением монолита): здесь читаются SystemSKU /
		// SystemProductName, но результат исторически никуда не присваивался. Поведение сохранено как есть,
		// чтобы этот этап оставался чистым переносом кода, а не незапланированным изменением логики.
		bufSize = sizeof(buffer);
		if (RegQueryValueExW(hKey, L"SystemSKU", nullptr, nullptr, reinterpret_cast<LPBYTE>(buffer), &bufSize) != ERROR_SUCCESS)
		{
			bufSize = sizeof(buffer);
			RegQueryValueExW(hKey, L"SystemProductName", nullptr, nullptr, reinterpret_cast<LPBYTE>(buffer), &bufSize);
		}

		RegCloseKey(hKey);
	}

	// 2. UUID оборудования из SMBIOS Type 1 (System Information) - таблица кэшируется в SmbiosReaderMSWin
	const auto& smbiosTable = SmbiosReaderMSWin::ReadRawTable();
	if (smbiosTable.has_value())
	{
		const std::byte* ptr = smbiosTable->data() + 8;
		const std::byte* end = smbiosTable->data() + smbiosTable->size();

		while (ptr + 4 <= end)
		{
			zU8 type = static_cast<zU8>(ptr[0]);
			zU8 length = static_cast<zU8>(ptr[1]);
			if (length < 4 || ptr + length > end) break;

			if (type == 1 && length >= 0x18) // Type 1: System Information
			{
				const zU8* u = reinterpret_cast<const zU8*>(ptr + 0x08);
				// Форматирование UUID согласно спецификации SMBIOS (Little-Endian первые 3 группы)
				sysUuid = std::format("{:02x}{:02x}{:02x}{:02x}-{:02x}{:02x}-{:02x}{:02x}-{:02x}{:02x}-{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}",
					u[3], u[2], u[1], u[0], u[5], u[4], u[7], u[6],
					u[8], u[9], u[10], u[11], u[12], u[13], u[14], u[15]);
				break;
			}

			ptr += length;
			while (ptr + 1 < end && !(ptr[0] == std::byte{ 0 } && ptr[1] == std::byte{ 0 }))
			{
				ptr++;
			}
			ptr += 2;
		}
	}

	return MotherboardInfo(mbVendor, mbModel, sysUuid);
}

#endif // defined(Z_WINDOWS)
