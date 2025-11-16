
#include "pch.h"

export module IAppWin;

import Event;
import Result;
import Size2D;
import AppWinConfig;
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
		explicit IAppWin(std::shared_ptr<const AppWinConfig> config);
		virtual ~IAppWin() = default;

		Event<Size2D<>, eTypeWinResize> OnResize;
		Event<> OnResizing;
		const Size2D<> GetWinSize() const noexcept { return m_WinSize; }

	protected:
		const std::shared_ptr<const AppWinConfig> m_Config;
		std::wstring m_Caption;
		Size2D<> m_WinSize;

		[[nodiscard]] virtual Result<> Initialize() = 0;
		friend class zzz::View;
	};

	IAppWin::IAppWin(std::shared_ptr<const AppWinConfig> config)
		: m_Config{ config },
		m_WinSize{ 0, 0 }
	{
		ensure(m_Config, ">>>>> [IAppWin::IAppWin()]. Settings cannot be null.");
	}
}