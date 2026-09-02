#include "RamInfoCollectorMSWin.h"

#if defined(Z_WINDOWS)

#include "engine/platforms/hardware/platforms/common/SmbiosReaderMSWin.h"

using namespace zzz::engine;
using namespace zzz::core;

RamInfo RamInfoCollectorMSWin::Collect() const
{
	MEMORYSTATUSEX memStatus{};
	memStatus.dwLength = sizeof(MEMORYSTATUSEX);
	zU64 totalPhys = 1024ULL * 1024 * 1024;
	zU64 availPhys = 512ULL * 1024 * 1024;
	if (GlobalMemoryStatusEx(&memStatus))
	{
		totalPhys = memStatus.ullTotalPhys;
		availPhys = memStatus.ullAvailPhys;
	}

	eRamType ramType = eRamType::Unknown;
	zU32 ramSpeed = 0;

	// Единое чтение таблицы SMBIOS (кэшируется в SmbiosReaderMSWin, читается один раз на процесс);
	// разбирается Type 17 (Memory Device) - тип и частота модулей ОЗУ.
	const auto& smbiosTable = SmbiosReaderMSWin::ReadRawTable();
	if (smbiosTable.has_value())
	{
		const std::byte* ptr = smbiosTable->data() + 8; // Пропускаем заголовок RawSMBIOSData (8 байт)
		const std::byte* end = smbiosTable->data() + smbiosTable->size();

		while (ptr + 4 <= end)
		{
			zU8 type = static_cast<zU8>(ptr[0]);
			zU8 length = static_cast<zU8>(ptr[1]);
			if (length < 4 || ptr + length > end) break;

			if (type == 17 && length >= 0x15) // Type 17: Memory Device
			{
				zU8 smbiosRamType = static_cast<zU8>(ptr[0x12]);
				zU16 configuredSpeed = *reinterpret_cast<const zU16*>(ptr + 0x15);
				if (length >= 0x22 && configuredSpeed == 0)
				{
					configuredSpeed = *reinterpret_cast<const zU16*>(ptr + 0x20); // ConfiguredClockSpeed
				}

				if (configuredSpeed > ramSpeed)
					ramSpeed = configuredSpeed;

				if (ramType == eRamType::Unknown)
				{
					switch (smbiosRamType)
					{
					case 0x18: ramType = eRamType::DDR3; break;
					case 0x1A: ramType = eRamType::DDR4; break;
					case 0x22: ramType = eRamType::DDR5; break;
					case 0x1E: ramType = eRamType::LPDDR4; break;
					case 0x23: ramType = eRamType::LPDDR5; break;
					}
				}
			}

			// Переход к следующей структуре в SMBIOS (пропускаем форматированную часть + текстовый блок с двумя нулевыми байтами)
			ptr += length;
			while (ptr + 1 < end && !(ptr[0] == std::byte{ 0 } && ptr[1] == std::byte{ 0 }))
			{
				ptr++;
			}
			ptr += 2;
		}
	}

	return RamInfo(totalPhys, availPhys, ramType, ramSpeed);
}

#endif // defined(Z_WINDOWS)
