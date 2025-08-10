#include "pch.h"
export module IWinSurface;

import result;
import zViewSettings;

using namespace zzz::platforms;

namespace zzz
{
	class zView;
}

namespace zzz
{
	export class IWinSurface abstract
	{
	public:
		IWinSurface() = delete;
		IWinSurface(const IWinSurface&) = delete;
		IWinSurface(IWinSurface&&) = delete;
		IWinSurface& operator=(const IWinSurface&) = delete;
		IWinSurface& operator=(IWinSurface&&) = delete;

		explicit IWinSurface(std::shared_ptr<const zViewSettings> settings);

		virtual ~IWinSurface() = default;

	protected:
		const std::shared_ptr<const zViewSettings> settings;
		[[nodiscard]] virtual result<> Initialize() = 0;
		friend class zzz::zView;
	};

	IWinSurface::IWinSurface(std::shared_ptr<const zViewSettings> settings)
		: settings{ settings }
	{
		ensure(settings, ">>>>> [IWinSurface::IWinSurface()]. Settings cannot be null.");
	}
}