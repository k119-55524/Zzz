#pragma once

#include <core/utils/Version.h>
#include <logger/logger.h>
#include <core/utils/Constants.h>
#include <core/IO/FileHeader.h>
#include <core/Serialize/Serializer.h>

using namespace zzz::io;
using namespace zzz::common;

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

		[[nodiscard]] const FileHeader<3>& GetMagic() const noexcept { return m_Magic; }
		[[nodiscard]] const Version& GetVersion() const noexcept { return m_Version; }
		[[nodiscard]] zU32 GetEntryCount() const noexcept { return m_EntryCount; }
		[[nodiscard]] std::expected<void, std::string> Validate() const
		{
			if (m_Magic != c_GamePackageHeader)
				return std::unexpected("Некорректная сигнатура заголовка");

			return {};
		}

		inline void LogFileBlock() const { DOut("[PackageHeader] Сигнатура: {} | Версия: {} | Количество записей: {}", m_Magic.ToString(), m_Version.ToString(), m_EntryCount); }

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
