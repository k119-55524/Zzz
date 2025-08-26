#include "pch.h"
export module ISurfaceView;

import IGAPI;
import result;
import size2D;
import IAppWin;
import Settings;

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
			std::shared_ptr<Settings> _settings,
			std::shared_ptr<IAppWin> _iAppWin,
			std::shared_ptr<IGAPI> _iGAPI);

		virtual ~ISurfaceView();

		[[nodiscard]] virtual result<> Initialize() = 0;
		virtual void PrepareFrame() = 0;
		virtual void RenderFrame() = 0;
		virtual void OnResize(const size2D<>& size) = 0;

		virtual void SetFullScreen(bool fs) {};
		inline void SetVSync(bool vs) { isVSync = vs; };

	protected:
		zU64 m_frameIndex;
		float m_aspectRatio;
		bool isVSync;
		std::shared_ptr<Settings> m_settings;
		std::shared_ptr<IAppWin> m_iAppWin;
		std::shared_ptr<IGAPI> m_iGAPI;
	};

	ISurfaceView::~ISurfaceView()
	{
		m_iGAPI->WaitForGpu();
	}

	ISurfaceView::ISurfaceView(
		std::shared_ptr<Settings> _settings,
		std::shared_ptr<IAppWin> _iAppWin,
		std::shared_ptr<IGAPI> _iGAPI) :
		m_frameIndex{ 0 },
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