#include "pch.h"
export module ISurfaceView;

import IGAPI;
import Scene;
import Colors;
import Result;
import size2D;
import IAppWin;
import Settings;
import RenderQueue;

using namespace zzz;
using namespace zzz::colors;

export namespace zzz::engineCore
{
	export enum class SurfClearType : zU32
	{
		None = 0,	// Без очистки
		Color = 1	// Очистка фона цветом
	};

	export class ISurfaceView abstract
	{
		Z_NO_CREATE_COPY(ISurfaceView);

	public:
		explicit ISurfaceView(
			std::shared_ptr<Settings> _settings,
			std::shared_ptr<IAppWin> _iAppWin,
			std::shared_ptr<IGAPI> _iGAPI);

		virtual ~ISurfaceView() = default;

		[[nodiscard]] virtual Result<> Initialize() = 0;
		virtual void PrepareFrame(std::shared_ptr<Scene> scene, std::shared_ptr<RenderQueue> renderQueue) = 0;
		virtual void RenderFrame() = 0;
		virtual void OnResize(const size2D<>& size) = 0;

		virtual void SetFullScreen(bool fs) {};
		inline void SetVSync(bool vs) { b_IsVSync = vs; };

	protected:
		zU64 m_frameIndex;
		bool b_IsVSync;
		std::shared_ptr<Settings> m_settings;
		std::shared_ptr<IAppWin> m_iAppWin;
		std::shared_ptr<IGAPI> m_iGAPI;
		size2D<> m_SurfSize;

		SurfClearType m_SurfClearType;
		Color m_ClearColor;
		bool b_IsClearDepth;
	};

	ISurfaceView::ISurfaceView(
		std::shared_ptr<Settings> _settings,
		std::shared_ptr<IAppWin> _iAppWin,
		std::shared_ptr<IGAPI> _iGAPI) :
		m_frameIndex{ 0 },
		b_IsVSync{ true },
		m_settings{ _settings },
		m_iAppWin{ _iAppWin },
		m_iGAPI{ _iGAPI },
		m_ClearColor{ colors::DarkMidnightBlue },
		b_IsClearDepth{ true },
		m_SurfSize{}
	{
		ensure(m_settings, ">>>>> [ISurfaceView::ISurfaceView()]. Settings cannot be null.");
		ensure(m_iAppWin, ">>>>> [ISurfaceView::ISurfaceView()]. Application window cannot be null.");
		ensure(m_iGAPI, ">>>>> [ISurfaceView::ISurfaceView()]. GAPI cannot be null.");

		m_SurfClearType = SurfClearType::Color;
	}
}