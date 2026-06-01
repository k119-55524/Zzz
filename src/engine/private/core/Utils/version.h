#pragma once

#include "Serialize/Serializer.h"

namespace zzz::engine
{
	class Version final : public ISerializable
	{
	public:
		constexpr Version() :
			m_Major(0),
			m_Minor(0),
			m_Patch(0)
		{}
		constexpr Version(zU32 major, zU32 minor, zU32 patch) :
			m_Major(major),
			m_Minor(minor),
			m_Patch(patch)
		{}

		~Version() = default;

		inline zU32 GetMajor() const noexcept { return m_Major; }
		inline zU32 GetMinor() const noexcept { return m_Minor; }
		inline zU32 GetPatch() const noexcept { return m_Patch; }

		inline std::string ToString() const { return std::format("{}.{}.{}", m_Major, m_Minor, m_Patch); }
		static std::expected<Version, std::string> Parse(std::string_view str)
		{
			Version v;
			char dot1, dot2;
			std::istringstream iss{ std::string{str} };
			if (!(iss >> v.m_Major >> dot1 >> v.m_Minor >> dot2 >> v.m_Patch) || dot1 != '.' || dot2 != '.')
				UNEXPECTED("Invalid version format");

			return v;
		}

		auto operator<=>(const Version&) const = default;

		inline Version BumpMajor() const noexcept { return Version(m_Major + 1, 0, 0); }
		inline Version BumpMinor() const noexcept { return Version(m_Major, m_Minor + 1, 0); }
		inline Version BumpPatch() const noexcept { return Version(m_Major, m_Minor, m_Patch + 1); }

	private:
		[[nodiscard]] std::expected<void, std::string> Serialize(std::vector<std::byte>& buffer, const Serializer& s) const override;
		[[nodiscard]] std::expected<void, std::string> DeSerialize(std::span<const std::byte> buffer, std::size_t& offset, const Serializer& s) override;

		zU32 m_Major;
		zU32 m_Minor;
		zU32 m_Patch;
	};
}