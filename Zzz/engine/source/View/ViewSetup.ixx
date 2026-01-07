
export module ViewSetup;

export import Colors;
export import RenderArea;

using namespace zzz::core;
using namespace zzz::colors;

namespace zzz
{
	export class ViewSetup
	{
	public:
		ViewSetup() = delete;
		ViewSetup(eAspectType aspect, Size2D<zF32>& size, zF32 minDepth, zF32 maxDepth) :
			m_RenderArea(aspect, size, minDepth, maxDepth),
			m_SurfClearType{ eSurfClearType::Color },
			m_ClearColor{ colors::DarkMidnightBlue },
			b_IsClearDepth{ true }
		{
		}

		inline const ViewportDesc& GetViewport() const noexcept { return m_RenderArea.GetViewport(); }
		inline const ScissorDesc& GetScissor() const noexcept { return m_RenderArea.GetScissor(); }
		inline eSurfClearType GetSurfClearType() const noexcept { return m_SurfClearType; }
		inline const Color& GetClearColor() const noexcept { return m_ClearColor; }
		inline bool IsClearDepth() const noexcept { return b_IsClearDepth; }

		inline void Update(Size2D<zF32>& size) noexcept { m_RenderArea.Update(size); }

	protected:
		RenderArea m_RenderArea;

		eSurfClearType m_SurfClearType;
		Color m_ClearColor;
		bool b_IsClearDepth;
	};
}