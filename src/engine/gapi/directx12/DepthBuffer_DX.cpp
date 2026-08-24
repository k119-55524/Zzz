#include "DepthBuffer_DX.h"

#if defined(Z_D3D12)

namespace zzz::engine
{
	DepthBuffer_DX::DepthBuffer_DX(std::shared_ptr<DirectX12API> gapi, const Size2D<>& size)
	{
		Initialize(gapi, size);
	}

	DepthBuffer_DX::~DepthBuffer_DX()
	{
		Release();
	}

	void DepthBuffer_DX::Release()
	{
		m_DepthStencilBuffer.Reset();
		m_DsvHeap.Reset();
	}

	void DepthBuffer_DX::OnResize(const Size2D<>& size)
	{
		Initialize(m_GAPI, size);
	}

	void DepthBuffer_DX::Initialize(std::shared_ptr<DirectX12API> gapi, const Size2D<>& size)
	{
		ensure(gapi, "DirectX12API cannot be null.");
		m_GAPI = gapi;

		ID3D12Device* device = gapi->GetDevice();
		ensure(device, "DirectX12 Device cannot be null.");

		Release();

		D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc{};
		dsvHeapDesc.NumDescriptors = 1;
		dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

		HRESULT hr = device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_DsvHeap));
		if (FAILED(hr))
			THROW_RUNTIME("Failed to create DSV Descriptor Heap: 0x{:08X}", static_cast<uint32_t>(hr));

		m_DsvHandle = m_DsvHeap->GetCPUDescriptorHandleForHeapStart();

		D3D12_RESOURCE_DESC depthStencilDesc{};
		depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		depthStencilDesc.Alignment = 0;
		depthStencilDesc.Width = static_cast<UINT64>(size.GetWidth());
		depthStencilDesc.Height = static_cast<UINT>(size.GetHeight());
		depthStencilDesc.DepthOrArraySize = 1;
		depthStencilDesc.MipLevels = 1;
		depthStencilDesc.Format = m_DepthFormat;
		depthStencilDesc.SampleDesc.Count = 1;
		depthStencilDesc.SampleDesc.Quality = 0;
		depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

		D3D12_CLEAR_VALUE optClear{};
		optClear.Format = m_DepthFormat;
		optClear.DepthStencil.Depth = 1.0f;
		optClear.DepthStencil.Stencil = 0;

		D3D12_HEAP_PROPERTIES heapProps{};
		heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;

		hr = device->CreateCommittedResource(
			&heapProps,
			D3D12_HEAP_FLAG_NONE,
			&depthStencilDesc,
			D3D12_RESOURCE_STATE_DEPTH_WRITE,
			&optClear,
			IID_PPV_ARGS(&m_DepthStencilBuffer)
		);

		if (FAILED(hr))
			THROW_RUNTIME("Failed to create Depth Stencil Buffer Resource: 0x{:08X}", static_cast<uint32_t>(hr));

		device->CreateDepthStencilView(m_DepthStencilBuffer.Get(), nullptr, m_DsvHandle);
	}
}

#endif // Z_D3D12
