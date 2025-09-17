#include "pch.h"
export module VertexFormatMapper;

import result;
import CPUVertexBuffer;

export namespace zzz
{
#if defined(RENDER_API_D3D12)
	using VertexAttrDescr = D3D12_INPUT_ELEMENT_DESC;
#elif defined(RENDER_API_VULKAN)
	using VertexAttrDescr = VkVertexInputAttributeDescription;
#elif defined(RENDER_API_METAL)
	struct VertexAttrDescr
	{
		MTLVertexFormat format;
		NSUInteger offset;
		NSUInteger bufferIndex;
	};
#else
#error ">>>>> [Compile error]. This branch requires implementation for the current platform"
#endif

#if defined(RENDER_API_D3D12)
	// Маппинг семантик на строки для D3D12
	constexpr const char* semanticToD3D12String(Semantic semantic)
	{
		switch (semantic)
		{
		case Semantic::Position: return "POSITION";
		case Semantic::Normal:   return "NORMAL";
		case Semantic::TexCoord: return "TEXCOORD";
		case Semantic::Color:    return "COLOR";
		case Semantic::Tangent:  return "TANGENT";
		case Semantic::Bitangent:return "BINORMAL";
		default:
			throw_runtime_error(">>>>> [semanticToD3D12String]. Unsupported Semantic");
		}
	}

	// Маппинг VertexFormat на DXGI_FORMAT (D3D12)
	DXGI_FORMAT vertexFormatToDXGI(VertexFormat format)
	{
		switch (format) {
		case VertexFormat::Unknown: return DXGI_FORMAT_UNKNOWN;

		// 8-bit
		case VertexFormat::R8_UNorm: return DXGI_FORMAT_R8_UNORM;
		case VertexFormat::R8_SNorm: return DXGI_FORMAT_R8_SNORM;
		case VertexFormat::R8_UInt: return DXGI_FORMAT_R8_UINT;
		case VertexFormat::R8_SInt: return DXGI_FORMAT_R8_SINT;
		case VertexFormat::RG8_UNorm: return DXGI_FORMAT_R8G8_UNORM;
		case VertexFormat::RG8_SNorm: return DXGI_FORMAT_R8G8_SNORM;
		case VertexFormat::RG8_UInt: return DXGI_FORMAT_R8G8_UINT;
		case VertexFormat::RG8_SInt: return DXGI_FORMAT_R8G8_SINT;
		case VertexFormat::RGBA8_UNorm: return DXGI_FORMAT_R8G8B8A8_UNORM;
		case VertexFormat::RGBA8_SNorm: return DXGI_FORMAT_R8G8B8A8_SNORM;
		case VertexFormat::RGBA8_UInt: return DXGI_FORMAT_R8G8B8A8_UINT;
		case VertexFormat::RGBA8_SInt: return DXGI_FORMAT_R8G8B8A8_SINT;
		case VertexFormat::RGBA8_UNorm_sRGB: return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

		// 16-bit
		case VertexFormat::R16_UNorm: return DXGI_FORMAT_R16_UNORM;
		case VertexFormat::R16_SNorm: return DXGI_FORMAT_R16_SNORM;
		case VertexFormat::R16_UInt: return DXGI_FORMAT_R16_UINT;
		case VertexFormat::R16_SInt: return DXGI_FORMAT_R16_SINT;
		case VertexFormat::R16_Float: return DXGI_FORMAT_R16_FLOAT;
		case VertexFormat::RG16_UNorm: return DXGI_FORMAT_R16G16_UNORM;
		case VertexFormat::RG16_SNorm: return DXGI_FORMAT_R16G16_SNORM;
		case VertexFormat::RG16_UInt: return DXGI_FORMAT_R16G16_UINT;
		case VertexFormat::RG16_SInt: return DXGI_FORMAT_R16G16_SINT;
		case VertexFormat::RG16_Float: return DXGI_FORMAT_R16G16_FLOAT;
		case VertexFormat::RGBA16_UNorm: return DXGI_FORMAT_R16G16B16A16_UNORM;
		case VertexFormat::RGBA16_SNorm: return DXGI_FORMAT_R16G16B16A16_SNORM;
		case VertexFormat::RGBA16_UInt: return DXGI_FORMAT_R16G16B16A16_UINT;
		case VertexFormat::RGBA16_SInt: return DXGI_FORMAT_R16G16B16A16_SINT;
		case VertexFormat::RGBA16_Float: return DXGI_FORMAT_R16G16B16A16_FLOAT;

	// 32-bit
		case VertexFormat::R32_UInt: return DXGI_FORMAT_R32_UINT;
		case VertexFormat::R32_SInt: return DXGI_FORMAT_R32_SINT;
		case VertexFormat::R32_Float: return DXGI_FORMAT_R32_FLOAT;
		case VertexFormat::RG32_UInt: return DXGI_FORMAT_R32G32_UINT;
		case VertexFormat::RG32_SInt: return DXGI_FORMAT_R32G32_SINT;
		case VertexFormat::RG32_Float: return DXGI_FORMAT_R32G32_FLOAT;
		case VertexFormat::RGB32_UInt: return DXGI_FORMAT_R32G32B32_UINT;
		case VertexFormat::RGB32_SInt: return DXGI_FORMAT_R32G32B32_SINT;
		case VertexFormat::RGB32_Float: return DXGI_FORMAT_R32G32B32_FLOAT;
		case VertexFormat::RGBA32_UInt: return DXGI_FORMAT_R32G32B32A32_UINT;
		case VertexFormat::RGBA32_SInt: return DXGI_FORMAT_R32G32B32A32_SINT;
		case VertexFormat::RGBA32_Float: return DXGI_FORMAT_R32G32B32A32_FLOAT;

			// Special / HDR
		case VertexFormat::RGB10A2_UNorm: return DXGI_FORMAT_R10G10B10A2_UNORM;
		case VertexFormat::RGB10A2_UInt: return DXGI_FORMAT_R10G10B10A2_UINT;
		case VertexFormat::R11G11B10_Float: return DXGI_FORMAT_R11G11B10_FLOAT;
		case VertexFormat::R9G9B9E5_SharedExp: return DXGI_FORMAT_R9G9B9E5_SHAREDEXP;

		default:
			throw_runtime_error(">>>>> [function vertexFormatToDXGI]. Unsupported VertexFormat");
		}
	}
#elif defined(RENDER_API_VULKAN)
	// Маппинг VertexFormat на VkFormat (Vulkan)
	VkFormat vertexFormatToVk(VertexFormat format)
	{
		switch (format) {
		case VertexFormat::Unknown: return VK_FORMAT_UNDEFINED;

		// 8-bit
		case VertexFormat::R8_UNorm: return VK_FORMAT_R8_UNORM;
		case VertexFormat::R8_SNorm: return VK_FORMAT_R8_SNORM;
		case VertexFormat::R8_UInt: return VK_FORMAT_R8_UINT;
		case VertexFormat::R8_SInt: return VK_FORMAT_R8_SINT;
		case VertexFormat::RG8_UNorm: return VK_FORMAT_R8G8_UNORM;
		case VertexFormat::RG8_SNorm: return VK_FORMAT_R8G8_SNORM;
		case VertexFormat::RG8_UInt: return VK_FORMAT_R8G8_UINT;
		case VertexFormat::RG8_SInt: return VK_FORMAT_R8G8_SINT;
		case VertexFormat::RGBA8_UNorm: return VK_FORMAT_R8G8B8A8_UNORM;
		case VertexFormat::RGBA8_SNorm: return VK_FORMAT_R8G8B8A8_SNORM;
		case VertexFormat::RGBA8_UInt: return VK_FORMAT_R8G8B8A8_UINT;
		case VertexFormat::RGBA8_SInt: return VK_FORMAT_R8G8B8A8_SINT;
		case VertexFormat::RGBA8_UNorm_sRGB: return VK_FORMAT_R8G8B8A8_SRGB;

		// 16-bit
		case VertexFormat::R16_UNorm: return VK_FORMAT_R16_UNORM;
		case VertexFormat::R16_SNorm: return VK_FORMAT_R16_SNORM;
		case VertexFormat::R16_UInt: return VK_FORMAT_R16_UINT;
		case VertexFormat::R16_SInt: return VK_FORMAT_R16_SINT;
		case VertexFormat::R16_Float: return VK_FORMAT_R16_SFLOAT;
		case VertexFormat::RG16_UNorm: return VK_FORMAT_R16G16_UNORM;
		case VertexFormat::RG16_SNorm: return VK_FORMAT_R16G16_SNORM;
		case VertexFormat::RG16_UInt: return VK_FORMAT_R16G16_UINT;
		case VertexFormat::RG16_SInt: return VK_FORMAT_R16G16_SINT;
		case VertexFormat::RG16_Float: return VK_FORMAT_R16G16_SFLOAT;
		case VertexFormat::RGBA16_UNorm: return VK_FORMAT_R16G16B16A16_UNORM;
		case VertexFormat::RGBA16_SNorm: return VK_FORMAT_R16G16B16A16_SNORM;
		case VertexFormat::RGBA16_UInt: return VK_FORMAT_R16G16B16A16_UINT;
		case VertexFormat::RGBA16_SInt: return VK_FORMAT_R16G16B16A16_SINT;
		case VertexFormat::RGBA16_Float: return VK_FORMAT_R16G16B16A16_SFLOAT;

		// 32-bit
		case VertexFormat::R32_UInt: return VK_FORMAT_R32_UINT;
		case VertexFormat::R32_SInt: return VK_FORMAT_R32_SINT;
		case VertexFormat::R32_Float: return VK_FORMAT_R32_SFLOAT;
		case VertexFormat::RG32_UInt: return VK_FORMAT_R32G32_UINT;
		case VertexFormat::RG32_SInt: return VK_FORMAT_R32G32_SINT;
		case VertexFormat::RG32_Float: return VK_FORMAT_R32G32_SFLOAT;
		case VertexFormat::RGB32_UInt: return VK_FORMAT_R32G32B32_UINT;
		case VertexFormat::RGB32_SInt: return VK_FORMAT_R32G32B32_SINT;
		case VertexFormat::RGB32_Float: return VK_FORMAT_R32G32B32_SFLOAT;
		case VertexFormat::RGBA32_UInt: return VK_FORMAT_R32G32B32A32_UINT;
		case VertexFormat::RGBA32_SInt: return VK_FORMAT_R32G32B32A32_SINT;
		case VertexFormat::RGBA32_Float: return VK_FORMAT_R32G32B32A32_SFLOAT;

		// Special / HDR
		case VertexFormat::RGB10A2_UNorm: return VK_FORMAT_A2B10G10R10_UNORM_PACK32;
		case VertexFormat::RGB10A2_UInt: return VK_FORMAT_A2B10G10R10_UINT_PACK32;
		case VertexFormat::R11G11B10_Float: return VK_FORMAT_B10G11R11_UFLOAT_PACK32;
		case VertexFormat::R9G9B9E5_SharedExp: return VK_FORMAT_E5B9G9R9_UFLOAT_PACK32;

		default: 
			throw_runtime_error(">>>>> [function vertexFormatToVk]. Unsupported VertexFormat");
		}
	}
#elif defined(RENDER_API_METAL)
// Маппинг VertexFormat на MTLVertexFormat (Metal)
MTLVertexFormat vertexFormatToMTL(VertexFormat format)
{
	switch (format) {
	case VertexFormat::Unknown: return MTLVertexFormatInvalid;

	// 8-bit
	case VertexFormat::R8_UNorm: return MTLVertexFormatUCharNormalized;
	case VertexFormat::R8_SNorm: return MTLVertexFormatCharNormalized;
	case VertexFormat::R8_UInt: return MTLVertexFormatUChar;
	case VertexFormat::R8_SInt: return MTLVertexFormatChar;
	case VertexFormat::RG8_UNorm: return MTLVertexFormatUChar2Normalized;
	case VertexFormat::RG8_SNorm: return MTLVertexFormatChar2Normalized;
	case VertexFormat::RG8_UInt: return MTLVertexFormatUChar2;
	case VertexFormat::RG8_SInt: return MTLVertexFormatChar2;
	case VertexFormat::RGBA8_UNorm: return MTLVertexFormatUChar4Normalized;
	case VertexFormat::RGBA8_SNorm: return MTLVertexFormatChar4Normalized;
	case VertexFormat::RGBA8_UInt: return MTLVertexFormatUChar4;
	case VertexFormat::RGBA8_SInt: return MTLVertexFormatChar4;
	case VertexFormat::RGBA8_UNorm_sRGB: return MTLVertexFormatUChar4Normalized; // sRGB approximation

	// 16-bit
	case VertexFormat::R16_UNorm: return MTLVertexFormatUShortNormalized;
	case VertexFormat::R16_SNorm: return MTLVertexFormatShortNormalized;
	case VertexFormat::R16_UInt: return MTLVertexFormatUShort;
	case VertexFormat::R16_SInt: return MTLVertexFormatShort;
	case VertexFormat::R16_Float: return MTLVertexFormatHalf;
	case VertexFormat::RG16_UNorm: return MTLVertexFormatUShort2Normalized;
	case VertexFormat::RG16_SNorm: return MTLVertexFormatShort2Normalized;
	case VertexFormat::RG16_UInt: return MTLVertexFormatUShort2;
	case VertexFormat::RG16_SInt: return MTLVertexFormatShort2;
	case VertexFormat::RG16_Float: return MTLVertexFormatHalf2;
	case VertexFormat::RGBA16_UNorm: return MTLVertexFormatUShort4Normalized;
	case VertexFormat::RGBA16_SNorm: return MTLVertexFormatShort4Normalized;
	case VertexFormat::RGBA16_UInt: return MTLVertexFormatUShort4;
	case VertexFormat::RGBA16_SInt: return MTLVertexFormatShort4;
	case VertexFormat::RGBA16_Float: return MTLVertexFormatHalf4;

	// 32-bit
	case VertexFormat::R32_UInt: return MTLVertexFormatUInt;
	case VertexFormat::R32_SInt: return MTLVertexFormatInt;
	case VertexFormat::R32_Float: return MTLVertexFormatFloat;
	case VertexFormat::RG32_UInt: return MTLVertexFormatUInt2;
	case VertexFormat::RG32_SInt: return MTLVertexFormatInt2;
	case VertexFormat::RG32_Float: return MTLVertexFormatFloat2;
	case VertexFormat::RGB32_UInt: return MTLVertexFormatUInt3;
	case VertexFormat::RGB32_SInt: return MTLVertexFormatInt3;
	case VertexFormat::RGB32_Float: return MTLVertexFormatFloat3;
	case VertexFormat::RGBA32_UInt: return MTLVertexFormatUInt4;
	case VertexFormat::RGBA32_SInt: return MTLVertexFormatInt4;
	case VertexFormat::RGBA32_Float: return MTLVertexFormatFloat4;

	// Special / HDR
	case VertexFormat::RGB10A2_UNorm: return MTLVertexFormatUInt1010102Normalized;
	case VertexFormat::RGB10A2_UInt: return MTLVertexFormatUInt1010102Normalized; // Approximation
	case VertexFormat::R11G11B10_Float: return MTLVertexFormatFloatRG11B10;
	case VertexFormat::R9G9B9E5_SharedExp: return MTLVertexFormatFloatRGB9E5;

	default:
		throw_runtime_error(">>>>> [function vertexFormatToMTL]. Unsupported VertexFormat");
	}
}
#else
#error ">>>>> [Compile error]. This branch requires implementation for the current platform"
#endif

	export class VertexFormatMapper
	{
	public:
		virtual ~VertexFormatMapper() = default;

		static std::vector<VertexAttrDescr> GetInputLayout(const ICPUVertexBuffer& vb);
	};

	std::vector<VertexAttrDescr> VertexFormatMapper::GetInputLayout(const ICPUVertexBuffer& vb)
	{
		std::vector<LayoutEntry> layout = vb.layout();
		std::vector<VertexAttrDescr> attr;

#if defined(RENDER_API_D3D12)
		attr.reserve(layout.size());
		for (const auto& entry : layout)
		{
			D3D12_INPUT_ELEMENT_DESC desc{};
			desc.SemanticName = semanticToD3D12String(entry.semantic);
			desc.SemanticIndex = entry.semanticIndex;
			desc.Format = vertexFormatToDXGI(entry.format);
			desc.InputSlot = 0;
			desc.AlignedByteOffset = static_cast<UINT>(entry.offset);
			desc.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
			desc.InstanceDataStepRate = 0;
			attr.push_back(desc);
		}
#elif defined(RENDER_API_VULKAN)
		attr.reserve(layout.size());
		for (size_t i = 0; i < layout.size(); ++i)
		{
			const auto& entry = layout[i];
			VkVertexInputAttributeDescription desc{};
			desc.location = static_cast<uint32_t>(i);
			desc.binding = 0;
			desc.format = vertexFormatToVk(entry.format);
			desc.offset = static_cast<uint32_t>(entry.offset);
			attr.push_back(desc);
		}
#elif defined(RENDER_API_METAL)
		attr.reserve(layout.size());
		for (size_t i = 0; i < layout.size(); ++i)
		{
			const auto& entry = layout[i];
			VertexAttrDescr desc{};
			desc.format = vertexFormatToMTL(entry.format);
			desc.offset = static_cast<NSUInteger>(entry.offset);
			desc.bufferIndex = 0;
			attr.push_back(desc);
		}
#else
#error ">>>>> [Compile error]. This branch requires implementation for the current platform"
#endif

		if (attr.empty())
			throw_runtime_error(">>>>> [VertexFormatMapper::GetInputLayout]. Failed to create input layout");

		return attr;
	};
}