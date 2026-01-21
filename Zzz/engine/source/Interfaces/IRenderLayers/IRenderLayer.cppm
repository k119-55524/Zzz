
export module IRenderLayer;

import Scene;
import RenderVolume;

namespace zzz
{
	export class IRenderLayer
	{
	public:
	IRenderLayer() = delete;
		IRenderLayer(eAspectType aspectPreset, Size2D<zF32>& size, zF32 minDepth, zF32 maxDepth, bool isactive = true) noexcept :
			m_RV{ aspectPreset, size, minDepth, maxDepth },
			b_IsActive{ isactive }
		{
		}
		virtual ~IRenderLayer() = 0;

		inline bool IsActive() const noexcept { return b_IsActive.load(std::memory_order_relaxed); }
		inline void SetActive(bool active) noexcept { b_IsActive.store(active, std::memory_order_relaxed); }

		void UpdateSize(Size2D<zF32>& size) noexcept { m_RV.UpdateSize(size); }
		inline const ViewportDesc& GetViewport() const noexcept { return m_RV.GetViewport(); }
		inline const ScissorDesc& GetScissor() const noexcept { return m_RV.GetScissor(); }

		[[nodiscard]] virtual std::shared_ptr<Scene> GetScene() const = 0;

	protected:
		RenderVolume m_RV;
		std::atomic<bool> b_IsActive;
	};

	IRenderLayer::~IRenderLayer()
	{
	}
}