#pragma once

#include <string>
#include <format>
#include <compare>
#include <sstream>
#include <expected>
#include <string_view>
#include <common/common.h>

namespace zzz::common
{
	class Version final
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
				return std::unexpected("Некорректный формат версии");

			return v;
		}

		constexpr auto operator<=>(const Version& other) const noexcept
		{
			if (auto cmp = m_Major <=> other.m_Major; cmp != 0) return cmp;
			if (auto cmp = m_Minor <=> other.m_Minor; cmp != 0) return cmp;
			return m_Patch <=> other.m_Patch;
		}

		constexpr bool operator==(const Version& other) const noexcept
		{
			return	m_Major == other.m_Major &&
					m_Minor == other.m_Minor &&
					m_Patch == other.m_Patch;
		}

		inline Version BumpMajor() const noexcept { return Version(m_Major + 1, 0, 0); }
		inline Version BumpMinor() const noexcept { return Version(m_Major, m_Minor + 1, 0); }
		inline Version BumpPatch() const noexcept { return Version(m_Major, m_Minor + 1, 0); }

	private:
		zU32 m_Major;
		zU32 m_Minor;
		zU32 m_Patch;
	};
}
