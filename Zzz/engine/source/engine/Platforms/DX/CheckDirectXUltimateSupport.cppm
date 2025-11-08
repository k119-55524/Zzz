#include "pch.h"
export module CheckDirectXSupport;

import StrConvert;
import ICheckGapiSupport;

using namespace zzz::platforms;

#if defined(ZRENDER_API_D3D12)
export namespace zzz::platforms::directx
{
	export class CheckDirectXSupport : public ICheckGapiSupport
	{
	public:
		CheckDirectXSupport() = delete;
		CheckDirectXSupport(const CheckDirectXSupport&) = delete;
		CheckDirectXSupport(CheckDirectXSupport&&) = delete;
		~CheckDirectXSupport() = default;

		explicit CheckDirectXSupport(ComPtr<ID3D12Device> device);
		[[nodiscard]] std::wstring GetHighestShaderModelAsString(eShaderType eShaderType) const override;

	protected:
		void CheckSupported() override;

	private:
		ComPtr<ID3D12Device> m_device;
	};

	CheckDirectXSupport::CheckDirectXSupport(ComPtr<ID3D12Device> device) :
		m_device(device)
	{
		ensure(device);
		CheckSupported();
	}

	void CheckDirectXSupport::CheckSupported()
	{
		// Проверка поддержки Ray Tracing (DXR)
		D3D12_FEATURE_DATA_D3D12_OPTIONS5 options5 = {};
		if (SUCCEEDED(m_device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &options5, sizeof(options5))))
			m_supportsRayTracing = (options5.RaytracingTier >= D3D12_RAYTRACING_TIER_1_0);

		// Проверка поддержки Variable Rate Shading
		D3D12_FEATURE_DATA_D3D12_OPTIONS6 options6 = {};
		if (SUCCEEDED(m_device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS6, &options6, sizeof(options6))))
			m_supportsVariableRateShading = (options6.VariableShadingRateTier >= D3D12_VARIABLE_SHADING_RATE_TIER_1);

		// Проверка поддержки Sampler Feedback и Mesh Shaders
		D3D12_FEATURE_DATA_D3D12_OPTIONS7 options7{};
		if (SUCCEEDED(m_device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS7, &options7, sizeof(options7))))
		{
			m_supportsMeshShaders = (options7.MeshShaderTier >= D3D12_MESH_SHADER_TIER_1);
			m_supportsSamplerFeedback = (options7.SamplerFeedbackTier >= D3D12_SAMPLER_FEEDBACK_TIER_0_9);
		}

		D3D12_FEATURE_DATA_SHADER_MODEL shaderModel = {};
		shaderModel.HighestShaderModel = D3D_HIGHEST_SHADER_MODEL;
		if (SUCCEEDED(m_device->CheckFeatureSupport(D3D12_FEATURE_SHADER_MODEL, &shaderModel, sizeof(shaderModel))))
		{
			switch (shaderModel.HighestShaderModel)
			{
			case D3D_SHADER_MODEL_5_1:
				m_strSupportHiShaderModel = L"5_1";
				break;
			case D3D_SHADER_MODEL_6_0:
				m_strSupportHiShaderModel = L"6_0";
				break;
			case D3D_SHADER_MODEL_6_1:
				m_strSupportHiShaderModel = L"6_1";
				break;
			case D3D_SHADER_MODEL_6_2:
				m_strSupportHiShaderModel = L"6_2";
				break;
			case D3D_SHADER_MODEL_6_3:
				m_strSupportHiShaderModel = L"6_3";
				break;
			case D3D_SHADER_MODEL_6_4:
				m_strSupportHiShaderModel = L"6_4";
				break;
			case D3D_SHADER_MODEL_6_5:
				m_strSupportHiShaderModel = L"6_5";
				break;
			case D3D_SHADER_MODEL_6_6:
				m_strSupportHiShaderModel = L"6_6";
				break;
			case D3D_SHADER_MODEL_6_7:
				m_strSupportHiShaderModel = L"6_7";
				break;
			case D3D_SHADER_MODEL_6_8:
				m_strSupportHiShaderModel = L"6_8";
				break;
			case D3D_SHADER_MODEL_6_9:
				m_strSupportHiShaderModel = L"6_9";
				break;
			default:
				throw_runtime_error(">>>>> [CheckDirectXSupport::CheckSupported()]. Unsupported shader model in GetHighestShaderModelAsString");
			}
		}

		// Проверка поддержки Copy Queue и DMA
		D3D12_COMMAND_QUEUE_DESC copyQueueDesc = {};
		copyQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_COPY;
		copyQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		ComPtr<ID3D12CommandQueue> tempCopyQueue;
		HRESULT hr = m_device->CreateCommandQueue(&copyQueueDesc, IID_PPV_ARGS(&tempCopyQueue));
		if (SUCCEEDED(hr))
		{
			m_supportsCopyQueue = true;
			// Проверка ResourceBindingTier для косвенной оценки DMA
			D3D12_FEATURE_DATA_D3D12_OPTIONS options = {};
			if (SUCCEEDED(m_device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS, &options, sizeof(options))))
			{
				m_supportsDedicatedDMA = (options.ResourceBindingTier >= D3D12_RESOURCE_BINDING_TIER_2);
			}
		}

#ifdef _DEBUG
		DebugOutput(std::format(L">>>>> [CheckDirectXSupport::CheckSupported()]. DirectX 12 Ultimate Support:\n"
			L" +- Hi shader model: {}\n"
			L" +- Ray Tracing: {}\n"
			L" +- Variable Rate Shading: {}\n"
			L" +- Mesh Shaders: {}\n"
			L" +- Sampler Feedback: {}\n"
			L" +- Copy Queue: {}\n"
			L" +- Likely Dedicated DMA: {}",
			m_strSupportHiShaderModel,
			m_supportsRayTracing ? L"Yes" : L"No",
			m_supportsVariableRateShading ? L"Yes" : L"No",
			m_supportsMeshShaders ? L"Yes" : L"No",
			m_supportsSamplerFeedback ? L"Yes" : L"No",
			m_supportsCopyQueue ? L"Yes" : L"No",
			m_supportsDedicatedDMA ? L"Yes" : L"No"));
#endif
	}

	[[nodiscard]] std::wstring CheckDirectXSupport::GetHighestShaderModelAsString(eShaderType eShaderType) const
	{
		std::wstring shaderModelStr;
		switch (eShaderType)
		{
		case eShaderType::Vertex:
			shaderModelStr = L"vs_" + m_strSupportHiShaderModel;
			break;
		case eShaderType::Pixel:
			shaderModelStr = L"ps_" + m_strSupportHiShaderModel;
			break;
		case eShaderType::Geometry:
			shaderModelStr = L"gs_" + m_strSupportHiShaderModel;
			break;
		case eShaderType::Hull:
			shaderModelStr = L"hs_" + m_strSupportHiShaderModel;
			break;
		case eShaderType::Domain:
			shaderModelStr = L"ds_" + m_strSupportHiShaderModel;
			break;
		case eShaderType::Compute:
			shaderModelStr = L"cs_" + m_strSupportHiShaderModel;
			break;
		default:
			throw_runtime_error("Unsupported shader type in GetHighestShaderModelAsString");
		}

		return shaderModelStr;
	}
};
#endif // defined(ZRENDER_API_D3D12)