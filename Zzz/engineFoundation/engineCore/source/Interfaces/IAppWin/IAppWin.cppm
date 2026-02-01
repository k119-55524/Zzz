
#include "pch.h"

export module IAppWin;

import Event;
import Result;
import Size2D;
import AppWinConfig;

namespace zzz
{
	class View;
}

export namespace zzz::core
{
	export class IAppWin abstract
	{
		Z_NO_CREATE_COPY(IAppWin);

	public:
		explicit IAppWin(std::shared_ptr<const AppWinConfig> config);
		virtual ~IAppWin() = default;

		Event<bool> OnActivate;
		Event<bool> OnFocus;
		Event<Size2D<>, eTypeWinResize> OnResize;
		Event<> OnResizing;

		const Size2D<> GetWinSize() const noexcept { return m_WinSize; }

		[[nodiscard]] inline bool IsActive() { return b_IsWinActive; };
	
		virtual void SetCaptionText(std::wstring caption) = 0;
		virtual void AddCaptionText(std::wstring caption) = 0;
		[[nodiscard]] virtual Result<> Initialize() = 0;

	protected:
		const std::shared_ptr<const AppWinConfig> m_Config;
		std::wstring m_Caption;
		Size2D<> m_WinSize;

		bool b_IsWinActive;
	};

	IAppWin::IAppWin(std::shared_ptr<const AppWinConfig> config)
		: m_Config{ config },
		m_WinSize{ 0, 0 },
		b_IsWinActive{ false }
	{
		ensure(m_Config, ">>>>> [IAppWin::IAppWin()]. Settings cannot be null.");
	}
}