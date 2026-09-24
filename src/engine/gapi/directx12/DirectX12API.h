#pragma once

#include <atomic>
#include "engine/gapi/IGAPI.h"
#include "engine/gapi/GAPIDebugLogger.h"

#if defined(Z_D3D12)
namespace zzz::engine
{
	class DirectX12API final : public IGAPI
	{
	public:
		explicit DirectX12API() = default;
		~DirectX12API() override;

		void WaitForGpu() override;
		void Initialize(std::shared_ptr<UserSettingsManager> userSettings) override;

		uint64_t SignalFence();
		void WaitForFenceValue(uint64_t fenceValue);

		template<typename T = void>
		void RegisterPendingTransition([[maybe_unused]] T* resource = nullptr) noexcept {}

		template<typename T = void>
		void FlushPendingTransitions([[maybe_unused]] T* cmdList = nullptr) noexcept {}

		[[nodiscard]] ID3D12Device* GetDevice() const noexcept { return m_Device.Get(); }
		[[nodiscard]] ID3D12CommandQueue* GetCommandQueue() const noexcept { return m_CommandQueue.Get(); }

		template<typename T>
		void SetDebugName(const Microsoft::WRL::ComPtr<T>& object, const char* name) const
		{
#if Z_DEBUG_BUILD || Z_DEVELOPMENT_BUILD
			if (!object || !name)
				return;

			int wlen = MultiByteToWideChar(CP_UTF8, 0, name, -1, nullptr, 0);
			if (wlen <= 0)
				return;

			std::wstring wname(static_cast<size_t>(wlen), L'\0');
			MultiByteToWideChar(CP_UTF8, 0, name, -1, wname.data(), wlen);

			object->SetName(wname.c_str());
#endif
		}

	private:
		void EnableDebugLayer(UINT& dxgiFactoryFlags);
		void InitializeDevice(std::shared_ptr<UserSettingsManager> userSettings, UINT dxgiFactoryFlags);
		[[nodiscard]] Microsoft::WRL::ComPtr<IDXGIFactory7> CreateFactory(UINT dxgiFactoryFlags);
		[[nodiscard]] Microsoft::WRL::ComPtr<IDXGIAdapter1> GetAdapter(IDXGIFactory1* pFactory, const std::shared_ptr<UserSettingsManager>& userSettings);
		void CreateDevice(IDXGIAdapter1* adapter);

		// Подписывается на ID3D12InfoQueue1::RegisterMessageCallback, чтобы сообщения DX12 debug layer
		// (сейчас идущие только в окно Output IDE) маршрутизировались в наш логгер через GAPIDebugLogger,
		// как это уже сделано для Vulkan validation layer.
		void RegisterDebugMessageCallback();

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

		Microsoft::WRL::ComPtr<ID3D12InfoQueue1> m_InfoQueue;
		DWORD m_InfoQueueCookie{ 0 };
	};
}
#endif // Z_D3D12
