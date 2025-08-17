#include "pch.h"
export module ISurfaceView;

import IGAPI;
import result;
import IAppWin;
import size2D;
import settings;

using namespace zzz::platforms;

export namespace zzz
{
	export class ISurfaceView abstract
	{
	public:
		ISurfaceView() = delete;
		ISurfaceView(const ISurfaceView&) = delete;
		ISurfaceView(ISurfaceView&&) = delete;
		ISurfaceView& operator=(const ISurfaceView&) = delete;
		ISurfaceView& operator=(ISurfaceView&&) = delete;

		explicit ISurfaceView(
			std::shared_ptr<settings> _settings,
			std::shared_ptr<IAppWin> _iAppWin,
			std::shared_ptr<IGAPI> _iGAPI);

		virtual ~ISurfaceView();

		[[nodiscard]] virtual result<> Initialize() = 0;
		virtual void OnResize(const size2D<>& size) = 0;

		virtual void BeginRender() = 0;
		virtual void Render() = 0;
		virtual void EndRender() = 0;

		virtual void SetFullScreen(bool fs) {};

	protected:
		float m_aspectRatio;
		bool isVSync; // Флаг, указывающий, включена ли синхронизация вертикальной развертки (VSync).
		std::shared_ptr<settings> m_settings;
		std::shared_ptr<IAppWin> m_iAppWin;
		std::shared_ptr<IGAPI> m_iGAPI;
	};

	ISurfaceView::~ISurfaceView()
	{
		m_iGAPI->WaitForGpu();
	}

	ISurfaceView::ISurfaceView(
		std::shared_ptr<settings> _settings,
		std::shared_ptr<IAppWin> _iAppWin,
		std::shared_ptr<IGAPI> _iGAPI) :
		m_aspectRatio{ 0.0f },
		isVSync{ true },
		m_settings{ _settings },
		m_iAppWin{ _iAppWin },
		m_iGAPI{ _iGAPI }
	{
		ensure(m_settings, ">>>>> [ISurfaceView::ISurfaceView()]. Settings cannot be null.");
		ensure(m_iAppWin, ">>>>> [ISurfaceView::ISurfaceView()]. Application window cannot be null.");
		ensure(m_iGAPI, ">>>>> [ISurfaceView::ISurfaceView()]. GAPI cannot be null.");
	}
}