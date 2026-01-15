
export module ViewportDesc;

export namespace zzz
{
	class ViewportDesc
	{
	public:
		float x, y;
		float width, height;
		float minDepth = 0.0f;
		float maxDepth = 1.0f;

#if defined(ZRENDER_API_D3D12)
		D3D12_VIEWPORT ToD3D12() const { return { x, y, width, height, minDepth, maxDepth }; }
#elif defined(ZRENDER_API_VULKAN)
		VkViewport ToVulkan() const { return { x, y, width, height, minDepth, maxDepth }; }
#elif defined(ZRENDER_API_METAL)
		MTLViewport ToMetal() const { return { x, y, width, height, minDepth, maxDepth }; }
#else
#error ">>>>> [ViewportDesc]. Unsupported rendering API for ViewportDesc conversions."
#endif
	};

	struct ScissorDesc
	{
		int32_t x, y;
		uint32_t width, height;

#if defined(ZRENDER_API_D3D12)
		D3D12_RECT ToD3D12() const { return { x, y, x + static_cast<LONG>(width), y + static_cast<LONG>(height) }; }
#elif defined(ZRENDER_API_VULKAN)
		VkRect2D ToVulkan() const { return { {x, y}, {width, height} }; }
#elif defined(ZRENDER_API_METAL)
		MTLScissorRect ToMetal() const { return { static_cast<NSUInteger>(x), static_cast<NSUInteger>(y), static_cast<NSUInteger>(width), static_cast<NSUInteger>(height) }; }
#else
#error ">>>>> [ScissorDesc]. Unsupported rendering API for ScissorDesc conversions."
#endif
	};
}