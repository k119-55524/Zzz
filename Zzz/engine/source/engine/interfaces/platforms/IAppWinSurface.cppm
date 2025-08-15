#include "pch.h"
export module IAppWinSurface;

import IGAPI;
import result;
import IAppWin;
import size2D;
import settings;

namespace zzz
{
	class view;
}

using namespace zzz::platforms;

export namespace zzz
{
	export class IAppWinSurface abstract
	{
		friend class zzz::view;

	public:
		IAppWinSurface() = delete;
		IAppWinSurface(const IAppWinSurface&) = delete;
		IAppWinSurface(IAppWinSurface&&) = delete;
		IAppWinSurface& operator=(const IAppWinSurface&) = delete;
		IAppWinSurface& operator=(IAppWinSurface&&) = delete;

		explicit IAppWinSurface(
			std::shared_ptr<settings> _settings,
			std::shared_ptr<IAppWin> _iAppWin,
			std::shared_ptr<IGAPI> _iGAPI);

		virtual ~IAppWinSurface();

		virtual void SetFullScreen(bool fs) {};

	protected:
		[[nodiscard]] virtual result<> Initialize() = 0;
		[[nodiscard]] virtual void OnRender() = 0;
		virtual void OnResize(const size2D<>& size) = 0;

		float m_aspectRatio;
		bool isVSync; // Флаг, указывающий, включена ли синхронизация вертикальной развертки (VSync).
		std::shared_ptr<settings> m_settings;
		std::shared_ptr<IAppWin> m_iAppWin;
		std::shared_ptr<IGAPI> m_iGAPI;

		void WaitRenderForPreviousFrame() { if (m_iGAPI) m_iGAPI->WaitForPreviousFrame(); };
	};

	IAppWinSurface::~IAppWinSurface()
	{
		WaitRenderForPreviousFrame();
	}

	IAppWinSurface::IAppWinSurface(
		std::shared_ptr<settings> _settings,
		std::shared_ptr<IAppWin> _iAppWin,
		std::shared_ptr<IGAPI> _iGAPI) :
		m_aspectRatio{ 0.0f },
		isVSync{ true },
		m_settings{ _settings },
		m_iAppWin{ _iAppWin },
		m_iGAPI{ _iGAPI }
	{
		ensure(m_settings, ">>>>> [IAppWinSurface::IAppWinSurface()]. Settings cannot be null.");
		ensure(m_iAppWin, ">>>>> [IAppWinSurface::IAppWinSurface()]. Application window cannot be null.");
		ensure(m_iGAPI, ">>>>> [IAppWinSurface::IAppWinSurface()]. GAPI cannot be null.");
	}
}