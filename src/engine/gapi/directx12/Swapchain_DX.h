#pragma once

#include "engine/gapi/directx12/DirectX12API.h"
#include "engine/platforms/window/NativeWindow.h"

#include "engine/gapi/clear_config/SurfaceClearConfig.h"

#include <mutex>
#include <condition_variable>

#if defined(Z_D3D12)

namespace zzz::engine
{
	class Swapchain_DX final
	{
		Z_NO_COPY_MOVE(Swapchain_DX);

	public:
		Swapchain_DX(std::shared_ptr<DirectX12API> gapi, std::shared_ptr<NativeWindow> window);
		~Swapchain_DX();

		void Release();

		uint32_t AcquireNextImage();
		void Present(bool vSync);
		void OnResize(const Size2D<>& size);

		void Clear(ID3D12GraphicsCommandList* cmdList);
		void Clear(ID3D12GraphicsCommandList* cmdList, uint32_t index);
		void SetClearConfig(const SurfaceClearConfig& config) noexcept { m_ClearConfig = config; }
		[[nodiscard]] const SurfaceClearConfig& GetClearConfig() const noexcept { return m_ClearConfig; }

		[[nodiscard]] Size2D<> GetSize() const noexcept { return m_Size; }
		[[nodiscard]] ID3D12Resource* GetCurrentBackBuffer() const noexcept;
		[[nodiscard]] D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentRTVHandle() const noexcept;

		[[nodiscard]] ID3D12Resource* GetBackBuffer(uint32_t index) const noexcept;
		[[nodiscard]] D3D12_CPU_DESCRIPTOR_HANDLE GetRTVHandle(uint32_t index) const noexcept;

		[[nodiscard]] uint32_t GetCurrentFrameIndex() const noexcept { return m_FrameIndex; }

	private:
		void Initialize(std::shared_ptr<NativeWindow> window);

		std::shared_ptr<DirectX12API> m_GAPI;
		SurfaceClearConfig m_ClearConfig;
		Microsoft::WRL::ComPtr<IDXGISwapChain3> m_SwapChain;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_RtvHeap;

		std::array<Microsoft::WRL::ComPtr<ID3D12Resource>, zzz::core::c_FramesInFlight> m_RenderTargets;
		uint32_t m_RtvDescriptorSize;
		uint32_t m_FrameIndex;
		uint32_t m_NextAcquireIndex;
		uint64_t m_PresentCount;
		std::mutex m_PresentMutex;
		std::condition_variable m_PresentCV;
		Size2D<> m_Size;
		DXGI_FORMAT m_BackBufferFormat;
	};
}
#endif // Z_D3D12
