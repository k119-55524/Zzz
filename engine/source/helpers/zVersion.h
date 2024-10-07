#pragma once

#include "../Types.h"

namespace Zzz
{
	class zVersion
	{
	public:
		zVersion() = delete;
		zVersion(const zVersion& other) = default;
		zVersion(zVersion&& other) = default;
		zVersion(int maj = 1, int min = 0, int pat = 0, int bld = 0)
			: major(maj), minor(min), patch(pat), build(bld) {}

		inline int getMajor() const noexcept { return major; }
		inline int getMinor() const noexcept { return minor; }
		inline int getPatch() const noexcept { return patch; }
		inline int getBuild() const noexcept { return build; }

		inline void setMajor(int maj) noexcept { major = maj; }
		inline void setMinor(int min) noexcept { minor = min; }
		inline void setPatch(int pat) noexcept { patch = pat; }
		inline void setBuild(int bld) noexcept { build = bld; }

		inline zStr ToString() const { return to_wstring(major) + L"." + to_wstring(minor) + L"." + to_wstring(patch) + L":" + to_wstring(build); }

		bool operator==(const zVersion& other) const
		{
			return 
				major == other.major &&
				minor == other.minor &&
				patch == other.patch &&
				build == other.build;
		}

		bool operator!=(const zVersion& other) const { return !(*this == other); }

		bool operator<(const zVersion& other) const
		{
			if (major != other.major)
				return major < other.major;

			if (minor != other.minor)
				return minor < other.minor;

			if (patch != other.patch)
				return patch < other.patch;

			return build < other.build;
		}

		bool operator>(const zVersion& other) const
		{
			return other < *this;
		}

		bool operator<=(const zVersion& other) const
		{
			return !(other < *this);
		}

		bool operator>=(const zVersion& other) const
		{
			return !(*this < other);
		}

	private:
		int major;
		int minor;
		int patch;
		int build;
	};
}
