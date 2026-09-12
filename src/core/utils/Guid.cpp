#include "core/utils/Guid.h"
#include <random>
#include <cstring>

namespace zzz::core
{
	Guid Guid::Generate() noexcept
	{
		thread_local std::mt19937_64 rng([] {
			std::random_device rd;
			return (static_cast<uint64_t>(rd()) << 32) | static_cast<uint64_t>(rd());
		}());

		const uint64_t part1 = rng();
		const uint64_t part2 = rng();

		Guid::RawBytes bytes{};
		std::memcpy(bytes.data(), &part1, 8);
		std::memcpy(bytes.data() + 8, &part2, 8);

		// Установка версии 4: биты 12-15 в time_hi_and_version равны 0100 (0x40)
		bytes[6] = static_cast<uint8_t>((bytes[6] & 0x0F) | 0x40);
		// Установка варианта RFC 4122: биты 6-7 в clock_seq_hi_and_reserved равны 10 (0x80)
		bytes[8] = static_cast<uint8_t>((bytes[8] & 0x3F) | 0x80);

		return Guid(bytes);
	}
}
