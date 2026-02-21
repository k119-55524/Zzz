
export module ISurfView;

import IGAPI;
import Scene;
import Ensure;
import Result;
import Size2D;
import IAppWin;
import RenderQueue;

using namespace zzz;

namespace zzz::core
{
	export class ISurfView abstract
	{
		Z_NO_CREATE_COPY(ISurfView);

	public:
		explicit ISurfView(std::shared_ptr<IGAPI> _iGAPI);
		virtual ~ISurfView() = default;

		[[nodiscard]] virtual Result<> Initialize() = 0;
		virtual void PrepareFrame(const std::shared_ptr<RenderQueue> renderQueue) = 0;
		virtual void RenderFrame() = 0;
		virtual void OnResize(const Size2D<>& size) = 0;

		virtual void SetFullScreen(bool fs) {};

	protected:
		std::shared_ptr<IGAPI> m_GAPI;
		Size2D<> m_SurfSize;
	};

	ISurfView::ISurfView(std::shared_ptr<IGAPI> _iGAPI) :
		m_GAPI{ _iGAPI },
		m_SurfSize{}
	{
		ensure(m_GAPI, "GAPI cannot be null.");
	}
}