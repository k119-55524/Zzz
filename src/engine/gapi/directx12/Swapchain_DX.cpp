#include "Swapchain_DX.h"

#if defined(Z_D3D12)
namespace zzz::engine
{
	Swapchain_DX::Swapchain_DX(std::shared_ptr<DirectX12API> gapi, std::shared_ptr<NativeWindow> window)
		: m_GAPI(std::move(gapi))
		, m_SwapChain(nullptr)
		, m_RtvHeap(nullptr)
		, m_RenderTargets{}
		, m_RtvDescriptorSize(0)
		, m_FrameIndex(0)
		, m_Size{}
		, m_BackBufferFormat(zzz::core::c_DefaultBackBufferFormat)
	{
		Initialize(window);
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

	void Swapchain_DX::Clear(ID3D12GraphicsCommandList* cmdList)
	{
		if (!cmdList) return;

		if (m_ClearConfig.mode == eSurfaceClearMode::Color)
		{
			const D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = GetCurrentRTVHandle();
			const zF32* clearColorData = reinterpret_cast<const zF32*>(&m_ClearConfig.color.R);
			cmdList->ClearRenderTargetView(rtvHandle, clearColorData, 0, nullptr);
		}
		else if (m_ClearConfig.mode == eSurfaceClearMode::Shader)
		{
			// Background shader pass stub
		}
	}

	void Swapchain_DX::Initialize(std::shared_ptr<NativeWindow> window)
	{
		ensure(m_GAPI, "DirectX12API cannot be null.");
		ensure(window, "Window cannot be null.");

		ID3D12Device* device = m_GAPI->GetDevice();
		ID3D12CommandQueue* commandQueue = m_GAPI->GetCommandQueue();
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
		swapChainDesc.BufferCount = c_FramesInFlight;
		swapChainDesc.Width = static_cast<UINT>(m_Size.GetWidth());
		swapChainDesc.Height = static_cast<UINT>(m_Size.GetHeight());
		swapChainDesc.Format = m_BackBufferFormat;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.Flags = (m_GAPI && m_GAPI->IsCanDisableVSync()) ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;

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
		rtvHeapDesc.NumDescriptors = c_FramesInFlight;
		rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

		hr = device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_RtvHeap));
		if (FAILED(hr))
			THROW_RUNTIME("Failed to create RTV Descriptor Heap: 0x{:08X}", static_cast<uint32_t>(hr));

		m_RtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_RtvHeap->GetCPUDescriptorHandleForHeapStart();
		for (UINT i = 0; i < c_FramesInFlight; i++)
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
			rt.Reset();

		UINT flags = (m_GAPI && m_GAPI->IsCanDisableVSync()) ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;
		HRESULT hr = m_SwapChain->ResizeBuffers(
			c_FramesInFlight,
			static_cast<UINT>(size.GetWidth()),
			static_cast<UINT>(size.GetHeight()),
			m_BackBufferFormat,
			flags
		);
		if (FAILED(hr))
		{
			DOutError("[Swapchain_DX::OnResize] Failed to ResizeBuffers ({}x{}): 0x{:08X}", size.GetWidth(), size.GetHeight(), static_cast<uint32_t>(hr));
			THROW_RUNTIME("[Swapchain_DX::OnResize] Failed to ResizeBuffers: 0x{:08X}", static_cast<uint32_t>(hr));
		}

		m_Size = size;
		m_FrameIndex = m_SwapChain->GetCurrentBackBufferIndex();
		ID3D12Device* device = m_GAPI->GetDevice();
		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_RtvHeap->GetCPUDescriptorHandleForHeapStart();
		for (UINT i = 0; i < c_FramesInFlight; i++)
		{
			hr = m_SwapChain->GetBuffer(i, IID_PPV_ARGS(&m_RenderTargets[i]));
			if (FAILED(hr))
			{
				DOutError("[Swapchain_DX::OnResize] Failed to get Swapchain Buffer {}: 0x{:08X}", i, static_cast<uint32_t>(hr));
				THROW_RUNTIME("[Swapchain_DX::OnResize] Failed to get Swapchain Buffer {}: 0x{:08X}", i, static_cast<uint32_t>(hr));
			}

			D3D12_CPU_DESCRIPTOR_HANDLE currentHandle = rtvHandle;
			currentHandle.ptr += static_cast<SIZE_T>(i * m_RtvDescriptorSize);
			device->CreateRenderTargetView(m_RenderTargets[i].Get(), nullptr, currentHandle);
		}

		DOut("[Swapchain_DX::OnResize] Successfully resized to {}x{} (BackBufferFormat: {}).", m_Size.GetWidth(), m_Size.GetHeight(), static_cast<uint32_t>(m_BackBufferFormat));
	}
}
#endif // Z_D3D12
