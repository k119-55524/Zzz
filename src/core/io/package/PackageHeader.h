#pragma once

#include <logger/logger.h>

#include "core/IO/FileHeader.h"
#include "core/utils/Version.h"
#include "core/Serialize/Serializer.h"
#include "core/constants/LogCategoryConstants.h"

namespace zzz::core
{
	class PackageHeader final : public ISerializable
	{
	public:
		PackageHeader() = default;
		PackageHeader(const FileHeader<3>& magic, const Version& version, zU32 entryCount)
			: m_Magic(magic)
			, m_Version(version)
			, m_EntryCount(entryCount)
		{}
		PackageHeader(const std::array<std::byte, 3>& magicBytes, const Version& version, zU32 entryCount)
			: m_Magic(magicBytes)
			, m_Version(version)
			, m_EntryCount(entryCount)
		{}

		[[nodiscard]] zU32 GetEntryCount() const noexcept { return m_EntryCount; }

		[[nodiscard]] static constexpr std::size_t BinarySize() noexcept
		{
			// Magic (3 bytes) + Version (3 * sizeof(zU32) = 12 bytes) + entryCount (sizeof(zU32) = 4 bytes) = 19 bytes
			return 3 + 3 * sizeof(zU32) + sizeof(zU32);
		}
		[[nodiscard]] std::expected<void, std::string> Validate(
			const FileHeader<3>& expectedHeader,
			zU8 expectedMajor) const
		{
			if (m_Magic != expectedHeader)
				return UNEXPECTED("Некорректная сигнатура заголовка");

			if (m_Version.GetMajor() != expectedMajor)
				return UNEXPECTED("Несовместимая версия формата пакета: {} (ожидалась мажорная версия {}). Пересоберите ассеты текущим Assets Builder.", m_Version.ToString(), expectedMajor);

			return {};
		}

		inline void LogFileBlock() const
		{
#if Z_ADD_LOGGER
			DOut(::zzz::core::Assets, "[PackageHeader]");
			DOut(::zzz::core::Assets, "  magic: {}", m_Magic.ToString());
			DOut(::zzz::core::Assets, "  version: {}", m_Version.ToString());
			DOut(::zzz::core::Assets, "  entryCount: {}", m_EntryCount);
#endif // Z_ADD_LOGGER
		}

	private:
		FileHeader<3> m_Magic{};
		Version m_Version{};
		zU32 m_EntryCount = 0;

	protected:
		[[nodiscard]] std::expected<void, std::string> Serialize(std::vector<std::byte>& buffer, const Serializer& serializer) const override
		{
			return serializer.Serialize(buffer, m_Magic)
				.and_then([&]() { return serializer.Serialize(buffer, m_Version); })
				.and_then([&]() { return serializer.Serialize(buffer, m_EntryCount); });
		}
		[[nodiscard]] std::expected<void, std::string> Deserialize(std::span<const std::byte> buffer, std::size_t& offset, const Serializer& serializer) override
		{
			return serializer.Deserialize(buffer, offset, m_Magic)
				.and_then([&]() { return serializer.Deserialize(buffer, offset, m_Version); })
				.and_then([&]() { return serializer.Deserialize(buffer, offset, m_EntryCount); });
		}
	};
}
