#include "pch.h"
export module ISurfaceAppWin;

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
	export class ISurfaceAppWin abstract
	{
		friend class zzz::zView;

	public:
		ISurfaceAppWin() = delete;
		ISurfaceAppWin(const ISurfaceAppWin&) = delete;
		ISurfaceAppWin(ISurfaceAppWin&&) = delete;
		ISurfaceAppWin& operator=(const ISurfaceAppWin&) = delete;
		ISurfaceAppWin& operator=(ISurfaceAppWin&&) = delete;

		explicit ISurfaceAppWin(
			std::shared_ptr<zViewSettings> _settings,
			std::shared_ptr<IAppWin> _iAppWin,
			std::shared_ptr<IGAPI> _iGAPI);

		virtual ~ISurfaceAppWin();

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

	ISurfaceAppWin::~ISurfaceAppWin()
	{
		WaitRenderForPreviousFrame();
	}

	ISurfaceAppWin::ISurfaceAppWin(
		std::shared_ptr<zViewSettings> _settings,
		std::shared_ptr<IAppWin> _iAppWin,
		std::shared_ptr<IGAPI> _iGAPI) :
		m_aspectRatio{ 0.0f },
		isVSync{ true },
		settings{ _settings },
		iAppWin{ _iAppWin },
		iGAPI{ _iGAPI }
	{
		ensure(settings, ">>>>> [ISurfaceAppWin::ISurfaceAppWin()]. Settings cannot be null.");
		ensure(iAppWin, ">>>>> [ISurfaceAppWin::ISurfaceAppWin()]. Application window cannot be null.");
		ensure(iGAPI, ">>>>> [ISurfaceAppWin::ISurfaceAppWin()]. GAPI cannot be null.");
	}
}