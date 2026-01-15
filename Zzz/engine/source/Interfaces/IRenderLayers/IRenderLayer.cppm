
export module IRenderLayer;

export import Scene;
export import RenderVolume;

namespace zzz
{
	export class IRenderLayer
	{
	public:
	IRenderLayer() = delete;
		IRenderLayer(eAspectType aspectPreset, Size2D<zF32>& size, zF32 minDepth, zF32 maxDepth) noexcept :
			m_RenderArea{ aspectPreset, size, minDepth, maxDepth }
		{
		}
		virtual ~IRenderLayer() = 0;

		void Update(Size2D<zF32>& size) noexcept { m_RenderArea.Update(size); }
		inline const ViewportDesc& GetViewport() const noexcept { return m_RenderArea.GetViewport(); }
		inline const ScissorDesc& GetScissor() const noexcept { return m_RenderArea.GetScissor(); }

		virtual const [[nodiscard]] std::shared_ptr<Scene> GetScene() const = 0;

	protected:
		RenderVolume m_RenderArea;

	};

	IRenderLayer::~IRenderLayer()
	{
	}
}