#include "pch.h"
export module IAppWin;

import result;
import zEvent;
import zSize2D;
import zViewSettings;

using namespace zzz::platforms;

namespace zzz
{
	class zView;
}

export namespace zzz
{
	export class IAppWin abstract
	{
	public:
		IAppWin() = delete;
		IAppWin(const IAppWin&) = delete;
		IAppWin(IAppWin&&) = delete;
		IAppWin& operator=(const IAppWin&) = delete;
		IAppWin& operator=(IAppWin&&) = delete;

		explicit IAppWin(std::shared_ptr<const zViewSettings> settings);

		virtual ~IAppWin() = default;

		zEvent<zSize2D<>, e_TypeWinResize> onResize;

		const zSize2D<> GetWinSize() const noexcept { return winSize; }

	protected:
		const std::shared_ptr<const zViewSettings> settings;
		zSize2D<> winSize;

		[[nodiscard]] virtual result<> Initialize() = 0;
		friend class zzz::zView;
	};

	IAppWin::IAppWin(std::shared_ptr<const zViewSettings> settings)
		: settings{ settings },
		winSize{ 0, 0 }
	{
		ensure(settings, ">>>>> [IAppWin::IAppWin()]. Settings cannot be null.");
	}
}