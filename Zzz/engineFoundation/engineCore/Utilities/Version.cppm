
export module Version;

import Result;
import Serializer;

using namespace zzz;

namespace zzz::core
{
	export class Version final : public ISerializable
	{
	public:
		constexpr Version() :
			m_Major(0),
			m_Minor(0),
			m_Patch(0)
		{
		}
		constexpr Version(zU32 major, zU32 minor, zU32 patch) :
			m_Major(major),
			m_Minor(minor),
			m_Patch(patch)
		{
		}

		~Version() = default;

		zU32 GetMajor() const noexcept { return m_Major; }
		zU32 GetMinor() const noexcept { return m_Minor; }
		zU32 GetPatch() const noexcept { return m_Patch; }

		inline std::string ToString() const { return std::format("{}.{}.{}", m_Major, m_Minor, m_Patch); }
		static Result<Version> Parse(std::string_view str)
		{
			Version v;
			char dot1, dot2;
			std::istringstream iss{ std::string{str} };
			if (!(iss >> v.m_Major >> dot1 >> v.m_Minor >> dot2 >> v.m_Patch) || dot1 != '.' || dot2 != '.')
				return Unexpected(L"Invalid version format");

			return v;
		}

		auto operator<=>(const Version&) const = default;

		constexpr inline Version BumpMajor() const noexcept { return Version(m_Major + 1, 0, 0); }
		constexpr inline Version BumpMinor() const noexcept { return Version(m_Major, m_Minor + 1, 0); }
		constexpr inline Version BumpPatch() const noexcept { return Version(m_Major, m_Minor, m_Patch + 1); }

	private:
		[[nodiscard]] Result<> Serialize(std::vector<std::byte>& buffer, const zzz::Serializer& s) const override;
		[[nodiscard]] Result<> DeSerialize(std::span<const std::byte> buffer, std::size_t& offset, const zzz::Serializer& s) override;

		zU32 m_Major;
		zU32 m_Minor;
		zU32 m_Patch;
	};

	[[nodiscard]] Result<> Version::Serialize(std::vector<std::byte>& buffer, const zzz::Serializer& s) const
	{
		return s.Serialize(buffer, m_Major)
			.and_then([&]() { return s.Serialize(buffer, m_Minor); })
			.and_then([&]() { return s.Serialize(buffer, m_Patch); });
	}
	[[nodiscard]] Result<> Version::DeSerialize(std::span<const std::byte> buffer, std::size_t& offset, const zzz::Serializer& s) 
	{
		return s.DeSerialize(buffer, offset, m_Major)
			.and_then([&]() { return s.DeSerialize(buffer, offset, m_Minor); })
			.and_then([&]() { return s.DeSerialize(buffer, offset, m_Patch); });
	}
}