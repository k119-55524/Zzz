#pragma once

#include "engine/gapi/directx12/DirectX12API.h"
#include "engine/platforms/window/NativeWindow.h"

#if defined(Z_D3D12)

namespace zzz::dx12
{
	constexpr uint32_t DX12_FRAMES_IN_FLIGHT = 2;

	class Swapchain_DX final
	{
		Z_NO_COPY_MOVE(Swapchain_DX);

	public:
		Swapchain_DX();
		~Swapchain_DX();

		std::expected<Size2D<>, std::string> Initialize(std::shared_ptr<zzz::engine::DirectX12API> gapi, std::shared_ptr<zzz::engine::NativeWindow> window);
		void Release();

		void Present(bool vSync);
		void OnResize(const Size2D<>& size);

		[[nodiscard]] ID3D12Resource* GetCurrentBackBuffer() const noexcept;
		[[nodiscard]] D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentRTVHandle() const noexcept;
		[[nodiscard]] uint32_t GetCurrentFrameIndex() const noexcept { return m_FrameIndex; }

	private:
		std::shared_ptr<zzz::engine::DirectX12API> m_GAPI;
		Microsoft::WRL::ComPtr<IDXGISwapChain3> m_SwapChain;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_RtvHeap;

		std::array<Microsoft::WRL::ComPtr<ID3D12Resource>, DX12_FRAMES_IN_FLIGHT> m_RenderTargets;
		uint32_t m_RtvDescriptorSize{ 0 };
		uint32_t m_FrameIndex{ 0 };
		DXGI_FORMAT m_BackBufferFormat{ DXGI_FORMAT_R8G8B8A8_UNORM };
	};
}

#endif // Z_D3D12
