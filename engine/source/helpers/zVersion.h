#pragma once

#include "../SceneEntities/zSerialize.h"

namespace Zzz
{
	class zVersion : protected zSerialize
	{
	public:
		zVersion() : major(0), minor(0), patch(0), build(0) {};
		zVersion(zVersion&& other) = default;

		zVersion& operator=(const zVersion& other) = default;
		zVersion& operator=(zVersion&& other) = default;

		zVersion(const zVersion& ver) : major(ver.major), minor(ver.minor), patch(ver.patch), build(ver.build) {}
		zVersion(int _major, int _minor = 0, int _patch = 0, int _buld = 0) : major(_major), minor(_minor), patch(_patch), build(_buld) {}

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

		inline void Serialize(stringstream& buffer) const
		{
			zSerialize::Serialize(buffer, major);
			zSerialize::Serialize(buffer, minor);
			zSerialize::Serialize(buffer, patch);
			zSerialize::Serialize(buffer, build);
		}

		void DeSerialize(istringstream& buffer)
		{
			zSerialize::DeSerialize(buffer, major);
			zSerialize::DeSerialize(buffer, minor);
			zSerialize::DeSerialize(buffer, patch);
			zSerialize::DeSerialize(buffer, build);
		}

	private:
		int major;
		int minor;
		int patch;
		int build;
	};
}
