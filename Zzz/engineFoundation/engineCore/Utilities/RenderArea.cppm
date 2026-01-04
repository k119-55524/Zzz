
export module RenderArea;

import ViewportDesc;

using namespace zzz;

export namespace zzz::core
{
	class RenderArea
	{
	public:
		RenderArea() = delete;
		RenderArea(eAspectType aspectPreset, zF32 x, zF32 y) noexcept :
			m_AspectPreset{ aspectPreset }
		{
		}
		RenderArea(
			eAspectType aspectPreset,
			float x, float y,
			float width, float height,
			float min_depth = 0.0f,
			float max_depth = 1.0f) noexcept :
			m_AspectPreset{ aspectPreset },
			viewport{ x, y, width, height, min_depth, max_depth },
			scissor{ static_cast<int32_t>(x), static_cast<int32_t>(y), static_cast<uint32_t>(width), static_cast<uint32_t>(height) }
		{
			Update(width, height);
		}

		void Update(zF32 surface_width, zF32 surface_height) noexcept;
		const ViewportDesc& GetViewport() const noexcept { return viewport; }
		const ScissorDesc& GetScissor() const noexcept { return scissor; }

	protected:
		eAspectType m_AspectPreset;
		float m_AspectRatio;

		ViewportDesc viewport;
		ScissorDesc scissor;
	};

	void RenderArea::Update(float surface_width, float surface_height) noexcept
	{
		m_AspectRatio = GetAspect(m_AspectPreset, surface_width, surface_height);
		zF32 surface_aspect = surface_width / surface_height;
		if (m_AspectPreset == eAspectType::FullWindow)
		{
			m_AspectRatio = surface_aspect;
			viewport = { 0.0f, 0.0f, surface_width, surface_height, 0.0f, 1.0f };
			scissor = { 0, 0, static_cast<uint32_t>(surface_width), static_cast<uint32_t>(surface_height) };
			return;
		}

		float render_width, render_height;
		float offset_x = 0.0f, offset_y = 0.0f;
		if (surface_aspect > m_AspectRatio)
		{
			render_height = surface_height;
			render_width = render_height * m_AspectRatio;
			offset_x = (surface_width - render_width) * 0.5f;
		}
		else
		{
			render_width = surface_width;
			render_height = render_width / m_AspectRatio;
			offset_y = (surface_height - render_height) * 0.5f;
		}

		viewport = { offset_x, offset_y, render_width, render_height, 0.0f, 1.0f };
		scissor =
		{
			static_cast<zI32>(std::round(offset_x)),
			static_cast<zI32>(std::round(offset_y)),
			static_cast<uint32_t>(std::round(render_width)),
			static_cast<uint32_t>(std::round(render_height))
		};
	}
}