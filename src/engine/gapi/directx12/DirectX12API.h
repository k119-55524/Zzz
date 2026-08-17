#pragma once

#include "engine/gapi/IGAPI.h"
#include "engine/gapi/selectors/gpu/directx12/DirectX12GpuSelector.h"

#if defined(Z_D3D12)

namespace zzz::engine
{
	class DirectX12API final : public IGAPI
	{
	public:
		explicit DirectX12API() = default;
		~DirectX12API() override;

		void SubmitCommandLists() override;
		void BeginRender() override;
		void EndRender() override;

	protected:
		void WaitForGpu() override;

	private:
		friend class Engine;
		void Initialize(std::shared_ptr<UserSettingsManager> userSettings) override;
		void EnableDebugLayer(UINT& dxgiFactoryFlags);
		[[nodiscard]] Microsoft::WRL::ComPtr<IDXGIFactory7> CreateFactory(UINT dxgiFactoryFlags);
		[[nodiscard]] Microsoft::WRL::ComPtr<IDXGIAdapter1> GetAdapter(IDXGIFactory1* pFactory, const std::shared_ptr<UserSettingsManager>& userSettings);
		void CreateDevice(IDXGIAdapter1* adapter);
		void InitializeDevice(std::shared_ptr<UserSettingsManager> userSettings, UINT dxgiFactoryFlags);

		Microsoft::WRL::ComPtr<IDXGIFactory7> m_Factory;
		Microsoft::WRL::ComPtr<IDXGIAdapter3> m_Adapter3;
		Microsoft::WRL::ComPtr<ID3D12Device> m_Device;
		Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_CommandQueue;

		D3D_FEATURE_LEVEL m_FeatureLevel{ D3D_FEATURE_LEVEL_12_0 };
	};
}

#endif // Z_D3D12
