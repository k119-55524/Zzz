#pragma once

#include "engine/gapi/directx12/DirectX12API.h"
#include "engine/platforms/window/NativeWindow.h"

#include "engine/gapi/clear_config/SurfaceClearConfig.h"

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

		void Present(bool vSync);
		void OnResize(const Size2D<>& size);

		void Clear(ID3D12GraphicsCommandList* cmdList);
		void SetClearConfig(const SurfaceClearConfig& config) noexcept { m_ClearConfig = config; }
		[[nodiscard]] const SurfaceClearConfig& GetClearConfig() const noexcept { return m_ClearConfig; }

		[[nodiscard]] Size2D<> GetSize() const noexcept { return m_Size; }
		[[nodiscard]] ID3D12Resource* GetCurrentBackBuffer() const noexcept;
		[[nodiscard]] D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentRTVHandle() const noexcept;
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
		Size2D<> m_Size;
		DXGI_FORMAT m_BackBufferFormat;
	};
}
#endif // Z_D3D12
