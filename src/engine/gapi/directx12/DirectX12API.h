#pragma once

#include <atomic>
#include "engine/gapi/IGAPI.h"

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
		void WaitForGpu() override;

		uint64_t SignalFence();
		void WaitForFenceValue(uint64_t fenceValue);

		[[nodiscard]] ID3D12Device* GetDevice() const noexcept { return m_Device.Get(); }
		[[nodiscard]] ID3D12CommandQueue* GetCommandQueue() const noexcept { return m_CommandQueue.Get(); }

	protected:

	private:
		friend class Engine;
		void Initialize(std::shared_ptr<UserSettingsManager> userSettings) override;
		void EnableDebugLayer(UINT& dxgiFactoryFlags);
		[[nodiscard]] Microsoft::WRL::ComPtr<IDXGIFactory7> CreateFactory(UINT dxgiFactoryFlags);
		[[nodiscard]] Microsoft::WRL::ComPtr<IDXGIAdapter1> GetAdapter(IDXGIFactory1* pFactory, const std::shared_ptr<UserSettingsManager>& userSettings);
		void CreateDevice(IDXGIAdapter1* adapter);
		void InitializeDevice(std::shared_ptr<UserSettingsManager> userSettings, UINT dxgiFactoryFlags);
		void SelectMonitor(IDXGIAdapter1* adapter, const std::shared_ptr<UserSettingsManager>& userSettings);

		Microsoft::WRL::ComPtr<IDXGIFactory7> m_Factory;
		Microsoft::WRL::ComPtr<IDXGIAdapter1> m_Adapter1;
		Microsoft::WRL::ComPtr<IDXGIAdapter3> m_Adapter3;
		Microsoft::WRL::ComPtr<ID3D12Device> m_Device;
		Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_CommandQueue;

		std::atomic<uint64_t> m_FenceValue{ 0 };
		HANDLE m_FenceEvent{ nullptr };
		Microsoft::WRL::ComPtr<ID3D12Fence> m_Fence;

		D3D_FEATURE_LEVEL m_FeatureLevel{ zzz::core::c_DefaultFeatureLevel };
	};
}
#endif // Z_D3D12
