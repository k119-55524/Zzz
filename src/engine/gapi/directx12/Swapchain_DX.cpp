#include "Swapchain_DX.h"

#if defined(Z_D3D12)
namespace zzz::engine
{
	Swapchain_DX::Swapchain_DX(std::shared_ptr<DirectX12API> gapi, std::shared_ptr<NativeWindow> window)
	{
		Initialize(gapi, window);
	}

	Swapchain_DX::~Swapchain_DX()
	{
		Release();
	}

	void Swapchain_DX::Release()
	{
		for (auto& rt : m_RenderTargets)
		{
			rt.Reset();
		}
		m_RtvHeap.Reset();
		m_SwapChain.Reset();
	}

	void Swapchain_DX::Initialize(std::shared_ptr<DirectX12API> gapi, std::shared_ptr<NativeWindow> window)
	{
		ensure(gapi, "DirectX12API cannot be null.");
		ensure(window, "Window cannot be null.");

		m_GAPI = gapi;
		ID3D12Device* device = gapi->GetDevice();
		ID3D12CommandQueue* commandQueue = gapi->GetCommandQueue();
		HWND hwnd = window->GetHWnd();

		ensure(device, "Device cannot be null.");
		ensure(commandQueue, "CommandQueue cannot be null.");
		ensure(hwnd, "HWND cannot be null.");

		Release();

		m_Size = window->GetClientRect().GetSize();

		Microsoft::WRL::ComPtr<IDXGIFactory4> factory;
		HRESULT hr = CreateDXGIFactory1(IID_PPV_ARGS(&factory));
		if (FAILED(hr))
			THROW_RUNTIME("Failed to CreateDXGIFactory1: 0x{:08X}", static_cast<uint32_t>(hr));

		DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
		swapChainDesc.BufferCount = DX12_FRAMES_IN_FLIGHT;
		swapChainDesc.Width = static_cast<UINT>(m_Size.GetWidth());
		swapChainDesc.Height = static_cast<UINT>(m_Size.GetHeight());
		swapChainDesc.Format = m_BackBufferFormat;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapChainDesc.SampleDesc.Count = 1;

		Microsoft::WRL::ComPtr<IDXGISwapChain1> swapChain1;
		hr = factory->CreateSwapChainForHwnd(
			commandQueue,
			hwnd,
			&swapChainDesc,
			nullptr,
			nullptr,
			&swapChain1
		);

		if (FAILED(hr))
			THROW_RUNTIME("Failed to CreateSwapChainForHwnd: 0x{:08X}", static_cast<uint32_t>(hr));

		hr = swapChain1.As(&m_SwapChain);
		if (FAILED(hr))
			THROW_RUNTIME("Failed to query IDXGISwapChain3 interface.");

		m_FrameIndex = m_SwapChain->GetCurrentBackBufferIndex();

		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc{};
		rtvHeapDesc.NumDescriptors = DX12_FRAMES_IN_FLIGHT;
		rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

		hr = device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_RtvHeap));
		if (FAILED(hr))
			THROW_RUNTIME("Failed to create RTV Descriptor Heap: 0x{:08X}", static_cast<uint32_t>(hr));

		m_RtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_RtvHeap->GetCPUDescriptorHandleForHeapStart();
		for (UINT i = 0; i < DX12_FRAMES_IN_FLIGHT; i++)
		{
			hr = m_SwapChain->GetBuffer(i, IID_PPV_ARGS(&m_RenderTargets[i]));
			if (FAILED(hr))
				THROW_RUNTIME("Failed to get Swapchain Buffer {}: 0x{:08X}", i, static_cast<uint32_t>(hr));

			D3D12_CPU_DESCRIPTOR_HANDLE currentHandle = rtvHandle;
			currentHandle.ptr += static_cast<SIZE_T>(i * m_RtvDescriptorSize);
			device->CreateRenderTargetView(m_RenderTargets[i].Get(), nullptr, currentHandle);
		}
	}

	ID3D12Resource* Swapchain_DX::GetCurrentBackBuffer() const noexcept
	{
		return m_RenderTargets[m_FrameIndex].Get();
	}

	D3D12_CPU_DESCRIPTOR_HANDLE Swapchain_DX::GetCurrentRTVHandle() const noexcept
	{
		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_RtvHeap->GetCPUDescriptorHandleForHeapStart();
		rtvHandle.ptr += static_cast<SIZE_T>(m_FrameIndex * m_RtvDescriptorSize);
		return rtvHandle;
	}

	void Swapchain_DX::Present(bool vSync)
	{
		if (m_SwapChain)
		{
			UINT syncInterval = vSync ? 1 : 0;
			m_SwapChain->Present(syncInterval, 0);
			m_FrameIndex = m_SwapChain->GetCurrentBackBufferIndex();
		}
	}

	void Swapchain_DX::OnResize(const Size2D<>& size)
	{
		if (!m_SwapChain)
			return;

		for (auto& rt : m_RenderTargets)
		{
			rt.Reset();
		}

		HRESULT hr = m_SwapChain->ResizeBuffers(
			DX12_FRAMES_IN_FLIGHT,
			static_cast<UINT>(size.GetWidth()),
			static_cast<UINT>(size.GetHeight()),
			m_BackBufferFormat,
			0
		);
		if (FAILED(hr))
			THROW_RUNTIME("[Swapchain_DX::OnResize] Failed to ResizeBuffers: 0x{:08X}", static_cast<uint32_t>(hr));

		m_Size = size;
		m_FrameIndex = m_SwapChain->GetCurrentBackBufferIndex();

		ID3D12Device* device = m_GAPI->GetDevice();
		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_RtvHeap->GetCPUDescriptorHandleForHeapStart();
		for (UINT i = 0; i < DX12_FRAMES_IN_FLIGHT; i++)
		{
			hr = m_SwapChain->GetBuffer(i, IID_PPV_ARGS(&m_RenderTargets[i]));
			if (FAILED(hr))
				THROW_RUNTIME("[Swapchain_DX::OnResize] Failed to get Swapchain Buffer {}: 0x{:08X}", i, static_cast<uint32_t>(hr));

			D3D12_CPU_DESCRIPTOR_HANDLE currentHandle = rtvHandle;
			currentHandle.ptr += static_cast<SIZE_T>(i * m_RtvDescriptorSize);
			device->CreateRenderTargetView(m_RenderTargets[i].Get(), nullptr, currentHandle);
		}
	}
}
#endif // Z_D3D12
