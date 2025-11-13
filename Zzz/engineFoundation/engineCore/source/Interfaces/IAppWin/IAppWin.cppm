
#include "pch.h"

export module IAppWin;

import event;
import result;
import size2D;
import Settings;
import IAppWin_MSWin;

namespace zzz
{
	class View;
}

export namespace zzz::engineCore
{
	export class IAppWin abstract :
		public IAppWin_MSWin
	{
		Z_NO_CREATE_COPY(IAppWin);

	public:
		explicit IAppWin(std::shared_ptr<const Settings> Settings);
		virtual ~IAppWin() = default;

		event<size2D<>, eTypeWinResize> OnResize;
		event<> OnResizing;
		const size2D<> GetWinSize() const noexcept { return winSize; }

	protected:
		const std::shared_ptr<const Settings> m_Settings;
		std::wstring m_Caption;
		size2D<> winSize;

		[[nodiscard]] virtual result<> Initialize() = 0;
		friend class zzz::View;
	};

	IAppWin::IAppWin(std::shared_ptr<const Settings> Settings)
		: m_Settings{ Settings },
		winSize{ 0, 0 }
	{
		ensure(Settings, ">>>>> [IAppWin::IAppWin()]. Settings cannot be null.");
	}
}