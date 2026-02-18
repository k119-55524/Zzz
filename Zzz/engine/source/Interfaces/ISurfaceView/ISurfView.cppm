
export module ISurfView;

import IGAPI;
import Scene;
import Result;
import Size2D;
import IAppWin;
import RenderQueue;

using namespace zzz;

export namespace zzz::core
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
		inline void SetVSync(bool vs)
		{
			if (m_iGAPI->IsCanDisableVsync())
				b_IsVSync = vs;
			else
				b_IsVSync = true; // Если отключение VSync не поддерживается, всегда включаем его
		};
		inline bool IsVSync() const noexcept { return b_IsVSync; }

	protected:
		std::shared_ptr<IGAPI> m_iGAPI;
		Size2D<> m_SurfSize;

	private:
		bool b_IsVSync;
	};

	ISurfView::ISurfView(std::shared_ptr<IGAPI> _iGAPI) :
		b_IsVSync{ true },
		m_iGAPI{ _iGAPI },
		m_SurfSize{}
	{
		ensure(m_iGAPI, ">>>>> [ISurfView::ISurfView()]. GAPI cannot be null.");
	}
}