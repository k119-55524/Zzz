
export module Layer3D;

export import Scene;
export import IRenderLayers;

namespace zzz
{
	export interface Layer3D final : public IRenderLayers
	{
	public:
		Layer3D() = delete;
		Layer3D(eAspectType aspectPreset, Size2D<zF32>& size, zF32 minDepth, zF32 maxDepth) noexcept :
			IRenderLayers{ aspectPreset, size, minDepth, maxDepth }
		{
		}
		virtual ~Layer3D() = default;

	private:
		Scene m_Scene;
	};
}