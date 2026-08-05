#pragma once

#include <array>
#include <span>
#include <vector>
#include <string>
#include <expected>
#include <cstddef>
#include "core/Serialize/Serializer.h"

namespace zzz::core
{
	template<std::size_t N>
	class FileHeader final : public ISerializable
	{
	public:
		constexpr FileHeader() = default;
		constexpr explicit FileHeader(const std::array<std::byte, N>& magic) noexcept
			: m_Magic(magic)
		{}

		[[nodiscard]] constexpr static std::size_t size() noexcept { return N; }
		[[nodiscard]] constexpr const std::array<std::byte, N>& GetMagic() const noexcept { return m_Magic; }
		[[nodiscard]] std::string ToString() const
		{
			std::string str;
			str.reserve(N);
			for (const auto& b : m_Magic)
				str.push_back(static_cast<char>(b));

			return str;
		}

		[[nodiscard]] constexpr bool operator==(const FileHeader& other) const noexcept { return m_Magic == other.m_Magic; }
		[[nodiscard]] constexpr bool operator==(const std::array<std::byte, N>& expectedMagic) const noexcept { return m_Magic == expectedMagic; }

	protected:
		[[nodiscard]] std::expected<void, std::string> Serialize(std::vector<std::byte>& buffer, const Serializer& serializer) const override
		{
			return serializer.Serialize(buffer, m_Magic);
		}
		[[nodiscard]] std::expected<void, std::string> Deserialize(std::span<const std::byte> buffer, std::size_t& offset, const Serializer& serializer) override
		{
			return serializer.Deserialize(buffer, offset, m_Magic);
		}

	private:
		std::array<std::byte, N> m_Magic = {};


	};
}
