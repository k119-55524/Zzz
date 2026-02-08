
export module SurfView_VK_MSWin;

#if defined(ZRENDER_API_VULKAN)

import IGAPI;
import IAppWin;
import RenderQueue;
import ISurfView;

using namespace zzz::core;

namespace zzz::vk
{
	export class SurfView_VK_MSWin final : public ISurfView
	{
		Z_NO_COPY_MOVE(SurfView_VK_MSWin);

	public:
		SurfView_VK_MSWin(std::shared_ptr<IAppWin> _iAppWin, std::shared_ptr<IGAPI> _iGAPI);
		virtual ~SurfView_VK_MSWin() override = default;

		[[nodiscard]] Result<> Initialize() override;

		void PrepareFrame(const std::shared_ptr<RenderQueue> renderQueue) override;
		void RenderFrame() override;
		void OnResize(const Size2D<>& size) override;
	};

	SurfView_VK_MSWin::SurfView_VK_MSWin(std::shared_ptr<IAppWin> _iAppWin, std::shared_ptr<IGAPI> _iGAPI) :
		ISurfView(_iGAPI)
	{
	}

	[[nodiscard]] Result<> SurfView_VK_MSWin::Initialize()
	{
		return {};
	}

	void SurfView_VK_MSWin::PrepareFrame(const std::shared_ptr<RenderQueue> renderQueue)
	{
	}

	void SurfView_VK_MSWin::RenderFrame()
	{
	}

	void SurfView_VK_MSWin::OnResize(const Size2D<>& size)
	{
		//m_SurfSize = size;
	}
}
#endif // ZRENDER_API_VULKAN