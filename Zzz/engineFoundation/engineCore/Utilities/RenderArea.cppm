
export module RenderArea;

using namespace zzz;

export namespace zzz::core
{
	class RenderArea
	{
	public:
		// Описание области просмотра
		struct Viewport
		{
			zF32 x, y;
			zF32 width, height;
			zF32 min_depth, max_depth;
		};

		// Описание области отсечения
		struct Scissor
		{
			zI32 x, y;
			zI32 width, height;
		};

		RenderArea() = delete;
		RenderArea(
			eAspectType aspectPreset,
			zF32 x, zF32 y) noexcept :
			m_AspectPreset{ aspectPreset }
		{
		}
		RenderArea(
			eAspectType aspectPreset,
			zF32 x, zF32 y,
			zF32 width, zF32 height,
			zF32 min_depth = 0.0f,
			zF32 max_depth = 1.0f) noexcept :
			m_AspectPreset{ aspectPreset },
			viewport{ x, y, width, height, min_depth, max_depth },
			scissor{ static_cast<zI32>(x), static_cast<zI32>(y), static_cast<zI32>(width), static_cast<zI32>(height) }
		{
			Update(width, height);
		}

		void Update(zF32 surface_width, zF32 surface_height) noexcept;

#if defined(ZRENDER_API_D3D12)
		// Конвертация в D3D12
		D3D12_VIEWPORT GetViewport() const noexcept
		{
			return
			{
				viewport.x, viewport.y,
				viewport.width, viewport.height,
				viewport.min_depth, viewport.max_depth
			};
		}

		D3D12_RECT GetScissor() const noexcept
		{
			return
			{
				(LONG)scissor.x,
				(LONG)scissor.y,
				(LONG)(scissor.x + scissor.width),
				(LONG)(scissor.y + scissor.height)
			};
		}
#elif defined(ZRENDER_API_VULKAN)
		// Конвертация в Vulkan
		VkViewport GetViewport() const noexcept
		{
			return
			{
				viewport.x, viewport.y,
				viewport.width, viewport.height,
				viewport.min_depth, viewport.max_depth
			};
		}

		VkRect2D GetScissor() const noexcept
		{
			return
			{
				{scissor.x, scissor.y},
				{static_cast<zU32>(scissor.width), static_cast<zU32>(scissor.height)}
			};
		}
#elif defined(__APPLE__)
		// Конвертация в Metal
		MTL::Viewport GetViewport() const noexcept
		{
			return
			{
				viewport.x, viewport.y,
				viewport.width, viewport.height,
				viewport.min_depth, viewport.max_depth
			};
		}

		MTL::ScissorRect GetScissor() const noexcept
		{
			return 
			{
				(NS::UInteger)scissor.x,
				(NS::UInteger)scissor.y,
				(NS::UInteger)scissor.width,
				(NS::UInteger)scissor.height
			};
		}
#else
#error ">>>>> []. Unsupported rendering API for RenderArea conversions."
#endif

	protected:
		eAspectType m_AspectPreset;

		Viewport viewport;
		Scissor scissor;
	};

	void RenderArea::Update(zF32 surface_width, zF32 surface_height) noexcept
	{
		float m_AspectRatio = GetAspect(m_AspectPreset, surface_width, surface_height);
		zF32 surface_aspect = surface_width / surface_height;
		if (m_AspectPreset == eAspectType::FullWindow)
		{
			m_AspectRatio = surface_aspect;
			viewport = { 0.0f, 0.0f, surface_width, surface_height, 0.0f, 1.0f };
			scissor = { 0, 0, static_cast<zI32>(surface_width), static_cast<zI32>(surface_height) };
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
		scissor = { static_cast<zI32>(offset_x), static_cast<zI32>(offset_y), static_cast<zI32>(render_width), static_cast<zI32>(render_height) };
	}
}