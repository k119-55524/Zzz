#pragma once

#include <common/version.h>
#include <common/constants.h>
#include <common/io/zFileHeader.h>
#include <common/serialize/Serializer.h>

using namespace zzz::io;
using namespace zzz::common;

namespace zzz::core
{
	class PackageHeader final : public ISerializable
	{
	public:
		PackageHeader() = default;
		PackageHeader(const zFileHeader<3>& magic, const Version& version, zU32 entryCount)
			: m_Magic(magic)
			, m_Version(version)
			, m_EntryCount(entryCount)
		{}
		PackageHeader(const std::array<std::byte, 3>& magicBytes, const Version& version, zU32 entryCount)
			: m_Magic(magicBytes)
			, m_Version(version)
			, m_EntryCount(entryCount)
		{}

		[[nodiscard]] const zFileHeader<3>& GetMagic() const noexcept { return m_Magic; }
		[[nodiscard]] const Version& GetVersion() const noexcept { return m_Version; }
		[[nodiscard]] zU32 GetEntryCount() const noexcept { return m_EntryCount; }

		void SetEntryCount(zU32 count) noexcept { m_EntryCount = count; }

		[[nodiscard]] std::expected<void, std::string> Validate() const
		{
			if (m_Magic != c_GamePackageHeader)
				return std::unexpected("Некорректная сигнатура (magic) заголовка пакета");

			return {};
		}

		void LogFileBlock() const
		{
#if Z_ADD_LOGGER || Z_DEVELOPMENT_BUILD
			DOut("[PackageHeader] Сигнатура: {} | Версия: {} | Количество записей: {}", m_Magic.ToString(), m_Version.ToString(), m_EntryCount);
#endif
		}

	private:
		zFileHeader<3> m_Magic{};
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
