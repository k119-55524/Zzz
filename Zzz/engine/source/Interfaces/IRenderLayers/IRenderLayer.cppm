
export module IRenderLayer;

import Scene;
import RenderVolume;
import SceneEntityFactory;

namespace zzz
{
	export class IRenderLayer
	{
	public:
	IRenderLayer() = delete;
		IRenderLayer(
			const std::shared_ptr<SceneEntityFactory> entityFactory,
			eAspectType aspectPreset,
			Size2D<zF32>& size,
			zF32 minDepth,
			zF32 maxDepth,
			bool isactive = true) noexcept :
			m_EntityFactory{ entityFactory },
			m_RV{ aspectPreset, size, minDepth, maxDepth },
			b_IsActive{ isactive }
		{
			ensure(m_EntityFactory, ">>>>> [IRenderLayer::IRenderLayer()]. Scene entity factory cannot be null.");
		}
		virtual ~IRenderLayer() = 0;

		inline bool IsActive() const noexcept { return b_IsActive.load(std::memory_order_relaxed); }
		inline void SetActive(bool active) noexcept { b_IsActive.store(active, std::memory_order_relaxed); }

		void UpdateSize(Size2D<zF32>& size) noexcept { m_RV.UpdateSize(size); }
		inline const ViewportDesc& GetViewport() const noexcept { return m_RV.GetViewport(); }
		inline const ScissorDesc& GetScissor() const noexcept { return m_RV.GetScissor(); }

		[[nodiscard]] virtual std::shared_ptr<Scene> GetScene() const = 0;

	protected:
		std::shared_ptr<SceneEntityFactory> m_EntityFactory;

		RenderVolume m_RV;
		std::atomic<bool> b_IsActive;
	};

	IRenderLayer::~IRenderLayer()
	{
	}
}