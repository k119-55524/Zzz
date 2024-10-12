#pragma once

#include "IGAPI.h"
#include "DirectX/RootSignature.h"

namespace Zzz::Platforms
{
#ifdef _GAPI_DX12

	class DirectX12API : public IGAPI
	{
	public:
		DirectX12API() = delete;
		DirectX12API(DirectX12API&) = delete;
		DirectX12API(DirectX12API&&) = delete;
		~DirectX12API() override;

		DirectX12API(const shared_ptr<IWinApp> appWin);

		void Initialize(const DataEngineInitialization& data) override;

	protected:
		void OnUpdate() override;
		void OnRender() override;
		void OnResize(const zSize& size) override;

	private:
		void InitializePipeline();
		void InitializeAssets();
		void GetAdapter(_In_ IDXGIFactory1* pFactory, _Outptr_result_maybenull_ IDXGIAdapter1** ppAdapter);
		void WaitForPreviousFrame();
		void PopulateCommandList();

		static const UINT FrameCount = 2;

		RootSignature m_rootSignature;
		CD3DX12_VIEWPORT m_viewport;
		CD3DX12_RECT m_scissorRect;

		ComPtr<ID3D12Device> m_device;
		ComPtr<ID3D12CommandQueue> m_commandQueue;
		ComPtr<IDXGISwapChain3> m_swapChain;
		ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
		ComPtr<ID3D12Resource> m_renderTargets[FrameCount];
		ComPtr<ID3D12CommandAllocator> m_commandAllocator;
		ComPtr<ID3D12GraphicsCommandList> m_commandList;

		UINT m_RtvDescrSize;
		UINT m_DsvDescrSize;
		UINT m_CbvSrvDescrSize;

		UINT m_frameIndex;
		HANDLE m_fenceEvent;
		ComPtr<ID3D12Fence> m_fence;
		UINT64 m_fenceValue;

		// Создана только для обхода ограничений VisualStudio при передаче параметров
		template <class T>
		constexpr inline auto& keep(T&& x) noexcept
		{
			return x;
		}
	};

#endif // _GAPI_DX12
}
