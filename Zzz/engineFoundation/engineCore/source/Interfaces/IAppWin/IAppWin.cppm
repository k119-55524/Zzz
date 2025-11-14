
#include "pch.h"

export module IAppWin;

import Event;
import Result;
import Size2D;
import Settings;
import IAppWin_MSWin;

namespace zzz
{
	class View;
}

export namespace zzz::core
{
	export class IAppWin abstract :
		public IAppWin_MSWin
	{
		Z_NO_CREATE_COPY(IAppWin);

	public:
		explicit IAppWin(std::shared_ptr<const Settings> Settings);
		virtual ~IAppWin() = default;

		Event<Size2D<>, eTypeWinResize> OnResize;
		Event<> OnResizing;
		const Size2D<> GetWinSize() const noexcept { return m_WinSize; }

	protected:
		const std::shared_ptr<const Settings> m_Settings;
		std::wstring m_Caption;
		Size2D<> m_WinSize;

		[[nodiscard]] virtual Result<> Initialize() = 0;
		friend class zzz::View;
	};

	IAppWin::IAppWin(std::shared_ptr<const Settings> Settings)
		: m_Settings{ Settings },
		m_WinSize{ 0, 0 }
	{
		ensure(Settings, ">>>>> [IAppWin::IAppWin()]. Settings cannot be null.");
	}
}