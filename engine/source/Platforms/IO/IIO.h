#pragma once

#include "../../Constants.h"
#include "../../Types/Types.h"

using namespace Zzz;

namespace Zzz::Platforms
{
	class IIO
	{
	public:
		IIO();
		virtual ~IIO() = 0;

		inline const zStr& GetAppPath() const { return appPath; };
		inline const zStr& GetAppResourcesPath() const { return appResourcesPath; };

		inline const zStr& Separator() const noexcept { separator; };

	protected:
		const zStr separator;

		zStr appPath;
		zStr appResourcesPath;
	};
}
