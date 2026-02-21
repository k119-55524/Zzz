
export module PrimitiveTopology;

import Ensure;

export namespace zzz
{
	class PrimitiveTopology
	{
	public:
		PrimitiveTopology() : topology(eTopology::Undefined) {}
		explicit PrimitiveTopology(eTopology topo) : topology(topo) {}

		// Операторы присваивания
		PrimitiveTopology& operator=(eTopology other) noexcept { topology = other; return *this; }
		PrimitiveTopology& operator=(const PrimitiveTopology& other) noexcept { topology = other.topology; return *this; }

		// Операторы сравнения
		bool operator==(eTopology other) const noexcept { return topology == other; }
		bool operator==(const PrimitiveTopology& other) const noexcept { return topology == other.topology; }
		bool operator!=(eTopology other) const noexcept { return topology != other; }
		bool operator!=(const PrimitiveTopology& other) const noexcept { return topology != other.topology; }

		eTopology Get() const noexcept { return topology; }
		bool IsValid() const noexcept { return topology != eTopology::Undefined; }

#if defined(ZRENDER_API_D3D12)
		D3D_PRIMITIVE_TOPOLOGY ToD3D12() const
		{
			ensure(topology != eTopology::Undefined, "Undefined topology cannot be converted.");

			switch (topology)
			{
			case eTopology::PointList:      return D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
			case eTopology::LineList:       return D3D_PRIMITIVE_TOPOLOGY_LINELIST;
			case eTopology::LineStrip:      return D3D_PRIMITIVE_TOPOLOGY_LINESTRIP;
			case eTopology::TriangleList:   return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
			case eTopology::TriangleStrip:  return D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
			default: return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
			}
		}
#elif defined(ZRENDER_API_VULKAN)
		VkPrimitiveTopology ToVulkan() const noexcept
		{
			ensure(topology != eTopology::Undefined, "Undefined topology cannot be converted.");

			switch (topology)
			{
			case eTopology::PointList:      return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
			case eTopology::LineList:       return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
			case eTopology::LineStrip:      return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
			case eTopology::TriangleList:   return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
			case eTopology::TriangleStrip:  return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
			//case eTopology::TriangleFan:    return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
			default: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
			}
		}
#elif defined(ZRENDER_API_METAL)
		MTLPrimitiveType ToMetal() const noexcept
		{
			ensure(topology != eTopology::Undefined, "Undefined topology cannot be converted.");

			switch (topology)
			{
			case eTopology::PointList:      return MTLPrimitiveTypePoint;
			case eTopology::LineList:       return MTLPrimitiveTypeLine;
			case eTopology::LineStrip:      return MTLPrimitiveTypeLineStrip;
			case eTopology::TriangleList:   return MTLPrimitiveTypeTriangle;
			case eTopology::TriangleStrip:  return MTLPrimitiveTypeTriangleStrip;
			default: return MTLPrimitiveTypeTriangle;
			}
		}
#else
#error ">>>>> [TopologyConverter]. Unsupported rendering API for TopologyConverter conversions."
#endif

	protected:
		eTopology topology;
	};
}