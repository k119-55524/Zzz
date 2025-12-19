
export module ISurfaceView;

import IGAPI;
import Scene;
import Result;
import Size2D;
import IAppWin;
import RenderQueue;

using namespace zzz;

export namespace zzz::core
{
	export class ISurfaceView abstract
	{
		Z_NO_CREATE_COPY(ISurfaceView);

	public:
		explicit ISurfaceView(
			std::shared_ptr<IAppWin> _iAppWin,
			std::shared_ptr<IGAPI> _iGAPI);

		virtual ~ISurfaceView() = default;

		[[nodiscard]] virtual Result<> Initialize() = 0;
		virtual void PrepareFrame(std::shared_ptr<Scene> scene, const RenderQueue& renderQueue) = 0;
		virtual void RenderFrame() = 0;
		virtual void OnResize(const Size2D<>& size) = 0;

		virtual void SetFullScreen(bool fs) {};
		inline void SetVSync(bool vs) { b_IsVSync = vs; };

	protected:
		zU64 m_frameIndex;
		bool b_IsVSync;
		std::shared_ptr<IAppWin> m_iAppWin;
		std::shared_ptr<IGAPI> m_iGAPI;
		Size2D<> m_SurfSize;
	};

	ISurfaceView::ISurfaceView(
		std::shared_ptr<IAppWin> _iAppWin,
		std::shared_ptr<IGAPI> _iGAPI) :
		m_frameIndex{ 0 },
		b_IsVSync{ true },
		m_iAppWin{ _iAppWin },
		m_iGAPI{ _iGAPI },
		m_SurfSize{}
	{
		ensure(m_iAppWin, ">>>>> [ISurfaceView::ISurfaceView()]. Application window cannot be null.");
		ensure(m_iGAPI, ">>>>> [ISurfaceView::ISurfaceView()]. GAPI cannot be null.");
	}
}