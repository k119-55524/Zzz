#pragma once

#include "engine/gapi/IGAPI.h"

#if defined(Z_D3D12)

namespace zzz::engine
{
	class DirectX12API final : public IGAPI
	{
	public:
		explicit DirectX12API(std::shared_ptr<UserSettingsManager> userSettings);
		~DirectX12API() override;

		void SubmitCommandLists() override;
		void BeginRender() override;
		void EndRender() override;

	protected:
		void WaitForGpu() override;

	private:
		friend class Engine;
		void Initialize() override;
		void EnableDebugLayer(UINT& dxgiFactoryFlags);
		void CreateFactory(UINT dxgiFactoryFlags, Microsoft::WRL::ComPtr<IDXGIFactory7>& outFactory);
		void GetAdapter(IDXGIFactory1* pFactory, Microsoft::WRL::ComPtr<IDXGIAdapter1>& outAdapter);
		void CreateDevice(Microsoft::WRL::ComPtr<IDXGIAdapter1> adapter, Microsoft::WRL::ComPtr<ID3D12Device>& outDevice, D3D_FEATURE_LEVEL& outFeatureLevel);
		void InitializeDevice(UINT dxgiFactoryFlags);

		Microsoft::WRL::ComPtr<IDXGIFactory7> m_Factory;
		Microsoft::WRL::ComPtr<IDXGIAdapter3> m_Adapter3;
		Microsoft::WRL::ComPtr<ID3D12Device> m_Device;
		Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_CommandQueue;
		D3D_FEATURE_LEVEL m_FeatureLevel{ D3D_FEATURE_LEVEL_12_0 };
		bool m_IsCanDisableVSync{ false };
	};
}

#endif // Z_D3D12
