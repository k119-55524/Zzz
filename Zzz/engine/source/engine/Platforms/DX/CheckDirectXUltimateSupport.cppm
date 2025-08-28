#include "pch.h"
export module CheckDirectXSupport;

import ICheckGapiSupport;

using namespace zzz::platforms;

#if defined(_WIN64)
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
		DebugOutput(std::format(L">>>>> [DXAPI::CheckDirectX12UltimateSupport()]. DirectX 12 Ultimate Support:\n"
			L" +- Ray Tracing: {}\n"
			L" +- Variable Rate Shading: {}\n"
			L" +- Mesh Shaders: {}\n"
			L" +- Sampler Feedback: {}\n"
			L" +- Copy Queue: {}\n"
			L" +- Likely Dedicated DMA: {}",
			m_supportsRayTracing ? L"Yes" : L"No",
			m_supportsVariableRateShading ? L"Yes" : L"No",
			m_supportsMeshShaders ? L"Yes" : L"No",
			m_supportsSamplerFeedback ? L"Yes" : L"No",
			m_supportsCopyQueue ? L"Yes" : L"No",
			m_supportsDedicatedDMA ? L"Yes (Tier 2+)" : L"No (Tier 1 or lower, may be emulated)").c_str());
#endif
	}
};
#endif // defined(_WIN64)