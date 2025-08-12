#include "pch.h"
export module surfaceAppMSWin_DirectX;

#if defined(_WIN64)
import IGAPI;
import DXAPI;
import result;
import zSize2D;
import IAppWin;
import winMSWin;
import strConver;
import ISurfaceAppWin;
import zViewSettings;

using namespace zzz::platforms;
using namespace zzz::platforms::directx;

namespace zzz
{
	export class surfaceAppMSWin_DirectX final : public ISurfaceAppWin
	{
	public:
		surfaceAppMSWin_DirectX() = delete;
		surfaceAppMSWin_DirectX(const surfaceAppMSWin_DirectX&) = delete;
		surfaceAppMSWin_DirectX(surfaceAppMSWin_DirectX&&) = delete;
		surfaceAppMSWin_DirectX& operator=(const surfaceAppMSWin_DirectX&) = delete;
		surfaceAppMSWin_DirectX& operator=(surfaceAppMSWin_DirectX&&) = delete;

		explicit surfaceAppMSWin_DirectX(
			std::shared_ptr<zViewSettings> _settings,
			std::shared_ptr<IAppWin> _iAppWin,
			std::shared_ptr<IGAPI> _iGAPI);

		[[nodiscard]] result<> Initialize() override;
		[[nodiscard]] void OnRender() override;
		void OnResize(const zSize2D<>& size) override;

	private:
		static constexpr UINT FRAME_COUNT = 2;
		static constexpr DXGI_FORMAT BACK_BUFFER_FORMAT = DXGI_FORMAT_R8G8B8A8_UNORM;
		static constexpr DXGI_FORMAT DEPTH_FORMAT = DXGI_FORMAT_D32_FLOAT;

		UINT m_swapChainFlags;
		float m_aspectRatio;
		CD3DX12_VIEWPORT m_viewport;
		CD3DX12_RECT m_scissorRect;

		std::shared_ptr<DXAPI> m_DXAPI;
		std::shared_ptr<winMSWin> m_Win;

		UINT m_frameIndex;

		ComPtr<IDXGISwapChain3> m_swapChain;
		ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
		ComPtr<ID3D12DescriptorHeap> m_srvHeap;
		ComPtr<ID3D12DescriptorHeap> m_dsvHeap;
		ComPtr<ID3D12Resource> m_renderTargets[FRAME_COUNT];
		ComPtr<ID3D12Resource> m_depthStencil;

		UINT m_RtvDescrSize;
		UINT m_DsvDescrSize;
		UINT m_CbvSrvDescrSize;

		result<> CreateRTVHeap();
		result<> CreateSRVHeap();
		result<> CreateDSVHeap();
		result<> CreateDS(const zSize2D<>& size);
		void BeginFrame();
		void EndFrame();
		void PopulateCommandList();
	};

	surfaceAppMSWin_DirectX::surfaceAppMSWin_DirectX(
		std::shared_ptr<zViewSettings> _settings,
		std::shared_ptr<IAppWin> _iAppWin,
		std::shared_ptr<IGAPI> _iGAPI)
		: ISurfaceAppWin(_settings, _iAppWin, _iGAPI),
		m_frameIndex{ 0 },
		m_RtvDescrSize{ 0 },
		m_DsvDescrSize{ 0 },
		m_CbvSrvDescrSize{ 0 },
		m_aspectRatio{ 0.0f },
		m_swapChainFlags{ DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING }
	{
		m_DXAPI = std::dynamic_pointer_cast<DXAPI>(_iGAPI);
		ensure(m_DXAPI);
		m_Win = std::dynamic_pointer_cast<winMSWin>(_iAppWin);
		ensure(m_Win);
	}

	result<> surfaceAppMSWin_DirectX::Initialize()
	{
		auto m_device = m_DXAPI->GetDevice();
		ensure(m_device, ">>>>> [surfaceAppMSWin_DirectX::Initialize()]. Device cannot be null.");
		auto m_commandQueue = m_DXAPI->GetCommandQueue();
		ensure(m_commandQueue, ">>>>> [surfaceAppMSWin_DirectX::Initialize()]. Command queue cannot be null.");
		auto m_factory = m_DXAPI->GetFactory();
		ensure(m_factory, ">>>>> [surfaceAppMSWin_DirectX::Initialize()]. Factory cannot be null.");

		m_RtvDescrSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		m_DsvDescrSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
		m_CbvSrvDescrSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		auto winSize = iAppWin->GetWinSize();
		m_viewport = CD3DX12_VIEWPORT{ 0.0f, 0.0f, static_cast<float>(winSize.width), static_cast<float>(winSize.height) };
		m_scissorRect = CD3DX12_RECT{ 0, 0, static_cast<LONG>(winSize.width), static_cast<LONG>(winSize.height) };
		m_aspectRatio = static_cast<float>(winSize.width) / static_cast<float>(winSize.height);

		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
		swapChainDesc.BufferCount = FRAME_COUNT;
		swapChainDesc.Width = static_cast<UINT>(winSize.width);
		swapChainDesc.Height = static_cast<UINT>(winSize.height);
		swapChainDesc.Format = BACK_BUFFER_FORMAT;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;

		ComPtr<IDXGISwapChain1> swapChain;
		HRESULT hr = m_factory->CreateSwapChainForHwnd(
			m_commandQueue.Get(),
			m_Win->GetHWND(),
			&swapChainDesc,
			nullptr,
			nullptr,
			&swapChain
		);
		if (FAILED(hr))
			return Unexpected(eResult::failure, L"Failed to create swap chain");

		hr = m_factory->MakeWindowAssociation(m_Win->GetHWND(), DXGI_MWA_NO_ALT_ENTER);
		if (FAILED(hr))
			return Unexpected(eResult::failure, L"Failed to make window association");

		hr = swapChain.As(&m_swapChain);
		if (FAILED(hr))
			return Unexpected(eResult::failure, L"Failed to query IDXGISwapChain3");

		m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

		auto res = CreateRTVHeap()
			.and_then([&]() { return CreateSRVHeap(); })
			.and_then([&]() { return CreateDSVHeap(); });

		// Create frame resources.
		{
			CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());

			// Create a RTV for each frame.
			for (UINT n = 0; n < FRAME_COUNT; n++)
			{
				HRESULT hr = m_swapChain->GetBuffer(n, IID_PPV_ARGS(&m_renderTargets[n]));
				if (FAILED(hr))
					return Unexpected(eResult::failure, std::format(L">>>>> [DXAPI::InitializePipeline()]. Failed to get back buffer. HRESULT = 0x{:08X}", hr));

				m_device->CreateRenderTargetView(m_renderTargets[n].Get(), nullptr, rtvHandle);
				rtvHandle.Offset(1, m_RtvDescrSize);
			}
		};

		CreateDS(winSize);
		if (!res)
			return Unexpected(eResult::failure, L">>>>> [DXAPI::InitializePipeline()]. -> " + res.error().getMessage());

		return {};
	}

	result<> surfaceAppMSWin_DirectX::CreateRTVHeap()
	{
		auto m_device = m_DXAPI->GetDevice();
		ensure(m_device, ">>>>> [surfaceAppMSWin_DirectX::CreateRTVHeap()]. Device cannot be null.");

		// Describe and create a render target view (RTV) descriptor heap.
		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
		rtvHeapDesc.NumDescriptors = FRAME_COUNT;
		rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		HRESULT hr = m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap));
		if (FAILED(hr))
			return Unexpected(eResult::failure, std::format(L"Failed to create RTV heap. HRESULT = 0x{:08X}", hr));

		return {};
	}

	result<> surfaceAppMSWin_DirectX::CreateSRVHeap()
	{
		auto m_device = m_DXAPI->GetDevice();
		ensure(m_device, ">>>>> [surfaceAppMSWin_DirectX::CreateSRVHeap()]. Device cannot be null.");

		// Describe and create a shader resource view (SRV) heap for the texture.
		D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
		srvHeapDesc.NumDescriptors = 1;
		srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		HRESULT hr = m_device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_srvHeap));
		if (FAILED(hr))
			return Unexpected(eResult::failure, std::format(L"Failed to create SRV heap. HRESULT = 0x{:08X}", hr));

		return {};
	}

	result<> surfaceAppMSWin_DirectX::CreateDSVHeap()
	{
		auto m_device = m_DXAPI->GetDevice();
		ensure(m_device, ">>>>> [surfaceAppMSWin_DirectX::CreateDSVHeap()]. Device cannot be null.");

		const D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc{
			.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV,
			.NumDescriptors = 1,
			.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
			.NodeMask = 0
		};

		HRESULT hr = m_device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_dsvHeap));
		if (FAILED(hr))
			return Unexpected(eResult::failure, std::format(L"Failed to create DSV heap. HRESULT = 0x{:08X}", hr));

		return {};
	}

	result<> surfaceAppMSWin_DirectX::CreateDS(const zSize2D<>& size)
	{
		auto m_device = m_DXAPI->GetDevice();
		ensure(m_device, ">>>>> [surfaceAppMSWin_DirectX::CreateDS()]. Device cannot be null.");

		m_depthStencil.Reset();

		// Описание вида глубины/трафарета
		D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = {};
		depthStencilDesc.Format = DEPTH_FORMAT;
		depthStencilDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		depthStencilDesc.Flags = D3D12_DSV_FLAG_NONE;

		// Значение по умолчанию для очистки глубины
		D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
		depthOptimizedClearValue.Format = DEPTH_FORMAT;
		depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
		depthOptimizedClearValue.DepthStencil.Stencil = 0;

		// Свойства кучи для текстуры глубины
		const CD3DX12_HEAP_PROPERTIES depthStencilHeapProps(D3D12_HEAP_TYPE_DEFAULT);

		// Описание ресурса глубины
		const CD3DX12_RESOURCE_DESC depthStencilTextureDesc =
			CD3DX12_RESOURCE_DESC::Tex2D(
				DEPTH_FORMAT,
				static_cast<UINT>(size.width),
				static_cast<UINT>(size.height),
				1, // массив из одной текстуры
				0, // mip-уровни
				1, 0,
				D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL
			);

		// Создаём ресурс глубины
		if (S_OK != m_device->CreateCommittedResource(
			&depthStencilHeapProps,
			D3D12_HEAP_FLAG_NONE,
			&depthStencilTextureDesc,
			D3D12_RESOURCE_STATE_DEPTH_WRITE,
			&depthOptimizedClearValue,
			IID_PPV_ARGS(&m_depthStencil)))
			return Unexpected(eResult::failure, L">>>>> [DXAPI::CreateDSView()]. Не удалось создать ресурс глубины.");

		SET_RESOURCE_DEBUG_NAME(m_depthStencil, L"Main DepthStenciI(D3D12Resource).");

		// Создаём DSV
		m_device->CreateDepthStencilView(
			m_depthStencil.Get(),
			&depthStencilDesc,
			m_dsvHeap->GetCPUDescriptorHandleForHeapStart());

		return {};
	}

	void surfaceAppMSWin_DirectX::BeginFrame()
	{
		// Обновляем frameIndex
		m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

		// Устанавливаем вьюпорт и scissors
		m_DXAPI->SetViewport(m_viewport, m_scissorRect);

		// Готовим ресурс к записи
		m_DXAPI->TransitionToRenderTarget(m_renderTargets[m_frameIndex], m_rtvHeap, m_RtvDescrSize, m_frameIndex);
	}

	void surfaceAppMSWin_DirectX::EndFrame()
	{
		m_DXAPI->TransitionToPresent(m_renderTargets[m_frameIndex]);
		m_swapChain->Present(1, 0);
	}

	void surfaceAppMSWin_DirectX::OnRender()
	{
		PopulateCommandList();

		// Execute the command list.
		m_DXAPI->ExecuteCommandList();

		// Present the frame.
		//ensure(S_OK == m_swapChain->Present(1, 0));
		HRESULT hr = m_swapChain->Present(1, 0);

		m_DXAPI->WaitForPreviousFrame();
		m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
	}

	void surfaceAppMSWin_DirectX::PopulateCommandList()
	{
		// Command list allocators can only be reset when the associated 
		// command lists have finished execution on the GPU; apps should use 
		// fences to determine GPU execution progress.
		ensure(S_OK == m_DXAPI->GetCommandRender()->CommandAllocator()->Reset());

		// However, when ExecuteCommandList() is called on a particular command 
		// list, that command list can then be reset at any time and must be before 
		// re-recording.
		//ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), m_pipelineState.Get()));
		ensure(S_OK == m_DXAPI->GetCommandRender()->CommandList()->Reset(m_DXAPI->GetCommandRender()->CommandAllocator().Get(), nullptr));

		// Set necessary state.
		m_DXAPI->GetCommandRender()->CommandList()->SetGraphicsRootSignature(m_DXAPI->GetRootSignature().Get());
		m_DXAPI->GetCommandRender()->CommandList()->RSSetViewports(1, &m_viewport);
		m_DXAPI->GetCommandRender()->CommandList()->RSSetScissorRects(1, &m_scissorRect);

		// Indicate that the back buffer will be used as a render target.
		m_DXAPI->GetCommandRender()->CommandList()->ResourceBarrier(
			1,
			&keep(
				CD3DX12_RESOURCE_BARRIER::Transition(
					m_renderTargets[m_frameIndex].Get(),
					D3D12_RESOURCE_STATE_PRESENT,
					D3D12_RESOURCE_STATE_RENDER_TARGET)
			)
		);

		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_RtvDescrSize);
		m_DXAPI->GetCommandRender()->CommandList()->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

		// Record commands.
		const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
		m_DXAPI->GetCommandRender()->CommandList()->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
		//m_commandListRender->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		//m_commandListRender->IASetVertexBuffers(0, 1, &m_vertexBufferView);
		//m_commandListRender->DrawInstanced(3, 1, 0, 0);

		// Indicate that the back buffer will now be used to present.
		m_DXAPI->GetCommandRender()->CommandList()->ResourceBarrier(
			1,
			&keep(
				CD3DX12_RESOURCE_BARRIER::Transition(
					m_renderTargets[m_frameIndex].Get(),
					D3D12_RESOURCE_STATE_RENDER_TARGET,
					D3D12_RESOURCE_STATE_PRESENT
				)
			)
		);

		ensure(S_OK == m_DXAPI->GetCommandRender()->CommandList()->Close());
	}

	void surfaceAppMSWin_DirectX::OnResize(const zSize2D<>& size)
	{
		if (iGAPI->GetInitState() != eInitState::eInitOK && !m_swapChain)
			return;

		if (size.width == 0 || size.height == 0)
		{
			OutputDebugString(L">>>>> [DXAPI::OnResize()]. Invalid size for resize. Width or height is zero.\n");
			return;
		}

		auto m_device = m_DXAPI->GetDevice();
		ensure(m_device, ">>>>> [surfaceAppMSWin_DirectX::CreateRTVHeap()]. Device cannot be null.");
		ensure(m_swapChain);

		DXGI_SWAP_CHAIN_DESC1 desc;
		HRESULT hr = m_swapChain->GetDesc1(&desc);
		if (SUCCEEDED(hr))
		{
			// Сравниваем с новым размером
			if (desc.Width == static_cast<UINT>(size.width) &&
				desc.Height == static_cast<UINT>(size.height))
			{
				OutputDebugString(std::format(L">>>>> [DXAPI::OnResize({}x{})]. No resize needed, dimensions are unchanged.\n", size.width, size.height).c_str());
				return; // Размеры не изменились
			}
		}

		iGAPI->WaitForPreviousFrame();

		for (UINT i = 0; i < FRAME_COUNT; i++)
			m_renderTargets[i].Reset();
		m_rtvHeap.Reset();

		hr = m_swapChain->ResizeBuffers(
			FRAME_COUNT,
			static_cast<UINT>(size.width),
			static_cast<UINT>(size.height),
			DXGI_FORMAT_UNKNOWN,
			m_swapChainFlags
		);
		if (FAILED(hr))
			throw_runtime_error(std::format(">>>>> [DXAPI::OnResize({}x{})].", size.width, size.height));

		m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

		auto res = CreateRTVHeap();
		if (!res)
			throw_runtime_error(std::format(">>>>> [DXAPI::OnResize({}x{})]. {}.", size.width, size.height, wstring_to_string(res.error().getMessage())));

		for (UINT i = 0; i < FRAME_COUNT; i++)
		{
			Microsoft::WRL::ComPtr<ID3D12Resource> backBuffer;
			if (S_OK != m_swapChain->GetBuffer(i, IID_PPV_ARGS(&backBuffer)))
				throw_runtime_error(std::format(">>>>> [DXAPI::OnResize({}x{})]. Failed to get back buffer {}.", size.width, size.height, i));

			m_device->CreateRenderTargetView(
				backBuffer.Get(),
				nullptr,
				CD3DX12_CPU_DESCRIPTOR_HANDLE(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), i, m_RtvDescrSize));
			m_renderTargets[i] = backBuffer;
		}

		res = CreateDS(size);
		if (!res)
			throw_runtime_error(std::format(">>>>> [DXAPI::OnResize({}x{})]. {}.", size.width, size.height, wstring_to_string(res.error().getMessage())));

		m_viewport = CD3DX12_VIEWPORT{ 0.0f, 0.0f, static_cast<float>(size.width), static_cast<float>(size.height) };
		m_scissorRect = CD3DX12_RECT{ 0, 0, static_cast<LONG>(size.width), static_cast<LONG>(size.height) };
		m_aspectRatio = static_cast<float>(size.width) / static_cast<float>(size.height);
	}
}
#endif // defined(_WIN64)