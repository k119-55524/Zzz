
export module Layer3D;

export import Scene;
export import IRenderLayer;

namespace zzz
{
	export interface Layer3D final : public IRenderLayer
	{
	public:
		Layer3D() = delete;
		Layer3D(std::shared_ptr<Scene> scene, eAspectType aspectPreset, Size2D<zF32>& size, zF32 minDepth, zF32 maxDepth) noexcept :
			IRenderLayer{ aspectPreset, size, minDepth, maxDepth },
			m_Scene{ scene }
		{
		}
		virtual ~Layer3D() = default;

		[[nodiscard]] std::shared_ptr<Scene> GetScene() const override { return m_Scene; };

	private:
		std::shared_ptr<Scene> m_Scene;
	};
}