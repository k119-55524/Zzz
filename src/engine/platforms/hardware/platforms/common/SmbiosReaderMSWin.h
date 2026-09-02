#pragma once

#include "core/utils/Defines.h"

#if defined(Z_WINDOWS)

#include <vector>
#include <optional>
#include <cstddef>

namespace zzz::engine
{
	/**
	 * @brief Единая точка чтения сырой таблицы SMBIOS (GetSystemFirmwareTable('RSMB', ...)) на Windows.
	 * До этапа 06 таблица читалась и парсилась дважды - отдельно в RAM- и в Motherboard-логике
	 * монолитного Platform::GatherHardwareState(). Здесь результат кэшируется в function-local static,
	 * так что фактическое системное чтение таблицы происходит ровно один раз за время жизни процесса,
	 * независимо от того, сколько коллекторов (RamInfoCollectorMSWin, MotherboardInfoCollectorMSWin)
	 * его запросят - без необходимости прокидывать общий буфер через конструкторы/Collect().
	 */
	class SmbiosReaderMSWin final
	{
	public:
		SmbiosReaderMSWin() = delete;

		[[nodiscard]] static const std::optional<std::vector<std::byte>>& ReadRawTable();
	};
}

#endif // defined(Z_WINDOWS)
