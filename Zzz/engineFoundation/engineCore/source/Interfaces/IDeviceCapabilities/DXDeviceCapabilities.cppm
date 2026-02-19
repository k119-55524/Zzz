
export module DXDeviceCapabilities;

#if defined(ZRENDER_API_D3D12)
import Ensure;
import StrConvert;
import IDeviceCapabilities;

namespace zzz::dx
{
	export class DXDeviceCapabilities : public IDeviceCapabilities
	{
		Z_NO_COPY_MOVE(DXDeviceCapabilities);

	public:
		explicit DXDeviceCapabilities(ComPtr<ID3D12Device> device, ComPtr<IDXGIAdapter3> adapter3);
		~DXDeviceCapabilities() = default;

		[[nodiscard]] std::wstring GetHighestShaderModelAsString(eShaderType eShaderType) const noexcept override;

	protected:
		void CheckSupported() override;

	private:
		ComPtr<ID3D12Device> m_device;
		ComPtr<IDXGIAdapter3> m_adapter3;
	};

	DXDeviceCapabilities::DXDeviceCapabilities(ComPtr<ID3D12Device> device, ComPtr<IDXGIAdapter3> adapter3) :
		m_device(device),
		m_adapter3(adapter3)
	{
		ensure(device);
		ensure(adapter3);
		CheckSupported();
	}

	void DXDeviceCapabilities::CheckSupported()
	{
		// --- Получение информации о GPU ---
		uint32_t vendorId = 0;
		uint32_t deviceId = 0;
		uint64_t dedicatedVideoMemoryMB = 0;
		uint64_t sharedSystemMemoryMB = 0;

		if (m_adapter3)
		{
			DXGI_ADAPTER_DESC2 desc{};
			if (SUCCEEDED(m_adapter3->GetDesc2(&desc)))
			{
				m_GPUName = desc.Description;
				vendorId = desc.VendorId;
				deviceId = desc.DeviceId;
				dedicatedVideoMemoryMB = desc.DedicatedVideoMemory / (1024 * 1024);
				sharedSystemMemoryMB = desc.SharedSystemMemory / (1024 * 1024);
			}
		}

		// --- Проверка DX12 Ultimate функций ---
		D3D12_FEATURE_DATA_D3D12_OPTIONS5 options5{};
		if (SUCCEEDED(m_device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &options5, sizeof(options5))))
			m_RayTracingSupported = (options5.RaytracingTier >= D3D12_RAYTRACING_TIER_1_0);

		D3D12_FEATURE_DATA_D3D12_OPTIONS6 options6{};
		if (SUCCEEDED(m_device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS6, &options6, sizeof(options6))))
			m_VariableRateShadingSupported = (options6.VariableShadingRateTier >= D3D12_VARIABLE_SHADING_RATE_TIER_1);

		D3D12_FEATURE_DATA_D3D12_OPTIONS7 options7{};
		if (SUCCEEDED(m_device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS7, &options7, sizeof(options7))))
		{
			m_MeshShadersSupported = (options7.MeshShaderTier >= D3D12_MESH_SHADER_TIER_1);
			m_SamplerFeedbackSupported = (options7.SamplerFeedbackTier >= D3D12_SAMPLER_FEEDBACK_TIER_0_9);
		}

		// --- Shader Model ---
		D3D12_FEATURE_DATA_SHADER_MODEL shaderModel{};
		shaderModel.HighestShaderModel = D3D_HIGHEST_SHADER_MODEL;
		if (SUCCEEDED(m_device->CheckFeatureSupport(D3D12_FEATURE_SHADER_MODEL, &shaderModel, sizeof(shaderModel))))
		{
			switch (shaderModel.HighestShaderModel)
			{
			case D3D_SHADER_MODEL_5_1: m_HighestShaderModel = L"5_1"; break;
			case D3D_SHADER_MODEL_6_0: m_HighestShaderModel = L"6_0"; break;
			case D3D_SHADER_MODEL_6_1: m_HighestShaderModel = L"6_1"; break;
			case D3D_SHADER_MODEL_6_2: m_HighestShaderModel = L"6_2"; break;
			case D3D_SHADER_MODEL_6_3: m_HighestShaderModel = L"6_3"; break;
			case D3D_SHADER_MODEL_6_4: m_HighestShaderModel = L"6_4"; break;
			case D3D_SHADER_MODEL_6_5: m_HighestShaderModel = L"6_5"; break;
			case D3D_SHADER_MODEL_6_6: m_HighestShaderModel = L"6_6"; break;
			case D3D_SHADER_MODEL_6_7: m_HighestShaderModel = L"6_7"; break;
			case D3D_SHADER_MODEL_6_8: m_HighestShaderModel = L"6_8"; break;
			case D3D_SHADER_MODEL_6_9: m_HighestShaderModel = L"6_9"; break;
			default: throw_runtime_error("Unsupported shader model");
			}
		}

		// --- Copy Queue и Dedicated DMA ---
		D3D12_COMMAND_QUEUE_DESC copyQueueDesc{};
		copyQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_COPY;
		copyQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		ComPtr<ID3D12CommandQueue> tempCopyQueue;
		if (SUCCEEDED(m_device->CreateCommandQueue(&copyQueueDesc, IID_PPV_ARGS(&tempCopyQueue))))
		{
			m_CopyQueueSupported = true;

			D3D12_FEATURE_DATA_D3D12_OPTIONS options{};
			if (SUCCEEDED(m_device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS, &options, sizeof(options))))
				m_DedicatedDMASupported = (options.ResourceBindingTier >= D3D12_RESOURCE_BINDING_TIER_2);
		}

		// --- Вывод всех данных ---
		DebugOutput(std::format(L"DirectX 12 Ultimate Device Capabilities:\n"
			L"     +- GPU: {}\n"
			L"     +- VendorId: 0x{:X}, DeviceId: 0x{:X}\n"
			L"     +- Dedicated Video Memory: {} MB, Shared System Memory: {} MB\n"
			L"     +- Hi shader model: {}\n"
			L"     +- Ray Tracing: {}\n"
			L"     +- Variable Rate Shading: {}\n"
			L"     +- Mesh Shaders: {}\n"
			L"     +- Sampler Feedback: {}\n"
			L"     +- Copy Queue: {}\n"
			L"     +- Likely Dedicated DMA: {}",
			m_GPUName,
			vendorId,
			deviceId,
			dedicatedVideoMemoryMB,
			sharedSystemMemoryMB,
			m_HighestShaderModel,
			m_RayTracingSupported ? L"Yes" : L"No",
			m_VariableRateShadingSupported ? L"Yes" : L"No",
			m_MeshShadersSupported ? L"Yes" : L"No",
			m_SamplerFeedbackSupported ? L"Yes" : L"No",
			m_CopyQueueSupported ? L"Yes" : L"No",
			m_DedicatedDMASupported ? L"Yes" : L"No"
		));
	}

	[[nodiscard]] std::wstring DXDeviceCapabilities::GetHighestShaderModelAsString(eShaderType eShaderType) const noexcept
	{
		std::wstring shaderModelStr;
		switch (eShaderType)
		{
		case eShaderType::Vertex:   shaderModelStr = L"vs_" + m_HighestShaderModel; break;
		case eShaderType::Pixel:    shaderModelStr = L"ps_" + m_HighestShaderModel; break;
		case eShaderType::Geometry: shaderModelStr = L"gs_" + m_HighestShaderModel; break;
		case eShaderType::Hull:     shaderModelStr = L"hs_" + m_HighestShaderModel; break;
		case eShaderType::Domain:   shaderModelStr = L"ds_" + m_HighestShaderModel; break;
		case eShaderType::Compute:  shaderModelStr = L"cs_" + m_HighestShaderModel; break;
		default: throw_runtime_error("Unsupported shader type in GetHighestShaderModelAsString");
		}
		return shaderModelStr;
	}
};
#endif // defined(ZRENDER_API_D3D12)
