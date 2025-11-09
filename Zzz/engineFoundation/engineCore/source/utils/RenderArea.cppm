
#include "pch.h"
export module RenderArea;

export namespace zzz::engineCore
{
	class RenderArea
	{
	public:
		struct Viewport
		{
			float x, y;
			float width, height;
			float min_depth, max_depth;
		};

		struct Scissor
		{
			int32_t x, y;
			int32_t width, height;
		};

		Viewport viewport;
		Scissor scissor;

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
				{(zU32)scissor.width, (zU32)scissor.height}
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
	};
}