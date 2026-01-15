
export module IRenderLayers;

export import RenderVolume;

namespace zzz
{
	export interface IRenderLayers
	{
	public:
	IRenderLayers() = delete;
		IRenderLayers(eAspectType aspectPreset, Size2D<zF32>& size, zF32 minDepth, zF32 maxDepth) noexcept :
			m_RenderArea{ aspectPreset, size, minDepth, maxDepth }
		{
		}
		virtual ~IRenderLayers() = default;

		void Update(Size2D<zF32>& size) noexcept { m_RenderArea.Update(size); }

	protected:
		RenderVolume m_RenderArea;

	};
}