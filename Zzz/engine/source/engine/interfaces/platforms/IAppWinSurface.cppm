#include "pch.h"
export module IAppWinSurface;

import IGAPI;
import result;
import IAppWin;
import zSize2D;
import zViewSettings;

namespace zzz
{
	class zView;
}

using namespace zzz::platforms;

export namespace zzz
{
	export class IAppWinSurface abstract
	{
		friend class zzz::zView;

	public:
		IAppWinSurface() = delete;
		IAppWinSurface(const IAppWinSurface&) = delete;
		IAppWinSurface(IAppWinSurface&&) = delete;
		IAppWinSurface& operator=(const IAppWinSurface&) = delete;
		IAppWinSurface& operator=(IAppWinSurface&&) = delete;

		explicit IAppWinSurface(
			std::shared_ptr<zViewSettings> _settings,
			std::shared_ptr<IAppWin> _iAppWin,
			std::shared_ptr<IGAPI> _iGAPI);

		virtual ~IAppWinSurface();

		virtual void SetFullScreen(bool fs) {};

	protected:
		[[nodiscard]] virtual result<> Initialize() = 0;
		[[nodiscard]] virtual void OnRender() = 0;
		virtual void OnResize(const zSize2D<>& size) = 0;

		float m_aspectRatio;
		bool isVSync; // Флаг, указывающий, включена ли синхронизация вертикальной развертки (VSync).
		std::shared_ptr<zViewSettings> settings;
		std::shared_ptr<IAppWin> iAppWin;
		std::shared_ptr<IGAPI> iGAPI;

		void WaitRenderForPreviousFrame() { if (iGAPI) iGAPI->WaitForPreviousFrame(); };
	};

	IAppWinSurface::~IAppWinSurface()
	{
		WaitRenderForPreviousFrame();
	}

	IAppWinSurface::IAppWinSurface(
		std::shared_ptr<zViewSettings> _settings,
		std::shared_ptr<IAppWin> _iAppWin,
		std::shared_ptr<IGAPI> _iGAPI) :
		m_aspectRatio{ 0.0f },
		isVSync{ true },
		settings{ _settings },
		iAppWin{ _iAppWin },
		iGAPI{ _iGAPI }
	{
		ensure(settings, ">>>>> [IAppWinSurface::IAppWinSurface()]. Settings cannot be null.");
		ensure(iAppWin, ">>>>> [IAppWinSurface::IAppWinSurface()]. Application window cannot be null.");
		ensure(iGAPI, ">>>>> [IAppWinSurface::IAppWinSurface()]. GAPI cannot be null.");
	}
}