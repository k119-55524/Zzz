
export module RenderArea;

export import Size2D;
export import ViewportDesc;

using namespace zzz::core;

export namespace zzz
{
	class RenderArea
	{
	public:
		RenderArea() = delete;
		RenderArea(eAspectType aspectPreset, Size2D<zF32>& size, zF32 minDepth, zF32 maxDepth) noexcept :
			m_AspectPreset{ aspectPreset },
			m_MinDepth{ minDepth },
			m_MaxDepth{ maxDepth }
		{
			Update(size);
		}

		void Update(Size2D<zF32>& size) noexcept;
		const ViewportDesc& GetViewport() const noexcept { return m_Viewport; }
		const ScissorDesc& GetScissor() const noexcept { return m_Scissor; }

	protected:
		eAspectType m_AspectPreset;
		float m_AspectRatio;
		float m_MinDepth;
		float m_MaxDepth;

		ViewportDesc m_Viewport;
		ScissorDesc m_Scissor;
	};

	void RenderArea::Update(Size2D<zF32>& size) noexcept
	{
		m_AspectRatio = GetAspect(m_AspectPreset, size.width, size.height);
		zF32 surface_aspect = size.width / size.height;
		if (m_AspectPreset == eAspectType::FullWindow)
		{
			m_AspectRatio = 1.0f;// surface_aspect;
			m_Viewport = { 0.0f, 0.0f, size.width, size.height, m_MinDepth, m_MaxDepth };
			m_Scissor = { 0, 0, static_cast<uint32_t>(size.width), static_cast<uint32_t>(size.height) };
			return;
		}

		float render_width, render_height;
		float offset_x = 0.0f, offset_y = 0.0f;
		if (surface_aspect > m_AspectRatio)
		{
			render_height = size.height;
			render_width = render_height * m_AspectRatio;
			offset_x = (size.width - render_width) * 0.5f;
		}
		else
		{
			render_width = size.width;
			render_height = render_width / m_AspectRatio;
			offset_y = (size.height - render_height) * 0.5f;
		}

		m_Viewport = { offset_x, offset_y, render_width, render_height, m_MinDepth, m_MaxDepth };
		m_Scissor =
		{
			static_cast<zI32>(std::round(offset_x)),
			static_cast<zI32>(std::round(offset_y)),
			static_cast<uint32_t>(std::round(render_width)),
			static_cast<uint32_t>(std::round(render_height))
		};
	}
}