#include "SmbiosReaderMSWin.h"

#if defined(Z_WINDOWS)

#include "core/utils/Defines.h"

using namespace zzz::engine;

namespace
{
	std::optional<std::vector<std::byte>> ReadRawTableOnce()
	{
		DWORD smbiosSize = GetSystemFirmwareTable('RSMB', 0, nullptr, 0);
		if (smbiosSize == 0)
			return std::nullopt;

		std::vector<std::byte> smbiosData(smbiosSize);
		if (GetSystemFirmwareTable('RSMB', 0, smbiosData.data(), smbiosSize) != smbiosSize)
			return std::nullopt;

		return smbiosData;
	}
}

const std::optional<std::vector<std::byte>>& SmbiosReaderMSWin::ReadRawTable()
{
	static const std::optional<std::vector<std::byte>> table = ReadRawTableOnce();
	return table;
}

#endif // defined(Z_WINDOWS)
