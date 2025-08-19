#include "pch.h"
export module surface_DirectX;

#if defined(_WIN64)
import IGAPI;
import DXAPI;
import result;
import size2D;
import IAppWin;
import winMSWin;
import settings;
import strConvert;
import ISurfaceView;

using namespace zzz::platforms;
using namespace zzz::platforms::directx;

namespace zzz
{
	export class surface_DirectX final : public ISurfaceView
	{
	public:
		surface_DirectX() = delete;
		surface_DirectX(const surface_DirectX&) = delete;
		surface_DirectX(surface_DirectX&&) = delete;
		surface_DirectX& operator=(const surface_DirectX&) = delete;
		surface_DirectX& operator=(surface_DirectX&&) = delete;

		explicit surface_DirectX(
			std::shared_ptr<settings> _settings,
			std::shared_ptr<IAppWin> _iAppWin,
			std::shared_ptr<IGAPI> _iGAPI);

		[[nodiscard]] result<> Initialize() override;
		[[nodiscard]] result<> CreateRTV(ComPtr<ID3D12Device>& m_device);
		[[nodiscard]] result<> InitializeSwapChain();

		void BeginRender() override;
		void Render() override;
		void EndRender() override;
		void OnResize(const size2D<>& size) override;

		void SetFullScreen(bool fs) override;

	private:
		static constexpr DXGI_FORMAT BACK_BUFFER_FORMAT = DXGI_FORMAT_R8G8B8A8_UNORM;
		static constexpr DXGI_FORMAT DEPTH_FORMAT = DXGI_FORMAT_D32_FLOAT;
		static constexpr UINT SWAP_CHAIN_FLAGS = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;

		bool b_IgnoreResize;

		UINT m_frameIndex;
		CD3DX12_VIEWPORT m_viewport;
		CD3DX12_RECT m_scissorRect;
		bool m_tearingSupported;

		std::shared_ptr<DXAPI> m_DXAPI;
		std::shared_ptr<winMSWin> m_Win;

		ComPtr<IDXGISwapChain3> m_swapChain;
		ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
		ComPtr<ID3D12DescriptorHeap> m_srvHeap;
		ComPtr<ID3D12DescriptorHeap> m_dsvHeap;
		ComPtr<ID3D12Resource> m_renderTargets[BACK_BUFFER_COUNT];
		ComPtr<ID3D12Resource> m_depthStencil[BACK_BUFFER_COUNT];

		UINT m_RtvDescrSize;
		UINT m_DsvDescrSize;
		UINT m_CbvSrvDescrSize;

		result<> CreateRTVHeap();
		result<> CreateSRVHeap();
		result<> CreateDSVHeap();
		result<> CreateDS(const size2D<>& size);

		void ResetRTVandDS();
		[[nodiscard]] result<> RecreateRenderTargetsAndDepth();
	};

	surface_DirectX::surface_DirectX(
		std::shared_ptr<settings> _settings,
		std::shared_ptr<IAppWin> _iAppWin,
		std::shared_ptr<IGAPI> _iGAPI)
		: ISurfaceView(_settings, _iAppWin, _iGAPI),
		b_IgnoreResize{ false },
		m_frameIndex{ 0 },
		m_tearingSupported{ false },
		m_RtvDescrSize{ 0 },
		m_DsvDescrSize{ 0 },
		m_CbvSrvDescrSize{ 0 }
	{
		m_DXAPI = std::dynamic_pointer_cast<DXAPI>(_iGAPI);
		ensure(m_DXAPI);
		m_Win = std::dynamic_pointer_cast<winMSWin>(_iAppWin);
		ensure(m_Win);
	}

#pragma region Initialize
	result<> surface_DirectX::Initialize()
	{
		auto m_device = m_DXAPI->GetDevice();
		ensure(m_device, ">>>>> [surface_DirectX::Initialize()]. Device cannot be null.");
		auto m_commandQueue = m_DXAPI->GetCommandQueue();
		ensure(m_commandQueue, ">>>>> [surface_DirectX::Initialize()]. Command queue cannot be null.");
		auto m_factory = m_DXAPI->GetFactory();
		ensure(m_factory, ">>>>> [surface_DirectX::Initialize()]. Factory cannot be null.");

		m_RtvDescrSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		m_DsvDescrSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
		m_CbvSrvDescrSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		auto winSize = m_iAppWin->GetWinSize();
		m_viewport = CD3DX12_VIEWPORT{ 0.0f, 0.0f, static_cast<float>(winSize.width), static_cast<float>(winSize.height) };
		m_scissorRect = CD3DX12_RECT{ 0, 0, static_cast<LONG>(winSize.width), static_cast<LONG>(winSize.height) };
		m_aspectRatio = static_cast<float>(winSize.width) / static_cast<float>(winSize.height);

		auto res = InitializeSwapChain();
		if (!res)
			return Unexpected(eResult::failure, L">>>>> [DXAPI::InitializePipeline()]. Failed to initialize swap chain.");

		HRESULT hr = m_factory->MakeWindowAssociation(m_Win->GetHWND(), DXGI_MWA_NO_ALT_ENTER);
		if (FAILED(hr))
			return Unexpected(eResult::failure, L"Failed to make window association");

		res = CreateRTVHeap()
			.and_then([&]() { return CreateSRVHeap(); })
			.and_then([&]() { return CreateDSVHeap(); })
			.and_then([&]() { return CreateRTV(m_device); });

		CreateDS(winSize);
		if (!res)
			return Unexpected(eResult::failure, L">>>>> [DXAPI::InitializePipeline()]. -> " + res.error().getMessage());

		return {};
	}

	result<> surface_DirectX::CreateRTV(ComPtr<ID3D12Device>& m_device)
	{
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());

		// Create a RTV for each frame.
		for (UINT n = 0; n < BACK_BUFFER_COUNT; n++)
		{
			HRESULT hr = m_swapChain->GetBuffer(n, IID_PPV_ARGS(&m_renderTargets[n]));
			if (FAILED(hr))
				return Unexpected(eResult::failure, std::format(L">>>>> [DXAPI::InitializePipeline()]. Failed to get back buffer. HRESULT = 0x{:08X}", hr));

			m_device->CreateRenderTargetView(m_renderTargets[n].Get(), nullptr, rtvHandle);
			rtvHandle.Offset(1, m_RtvDescrSize);

			SET_RESOURCE_DEBUG_NAME(m_renderTargets[n], std::format(L"RTV Frame {}", n).c_str());
		}

		return {};
	}

	result<> surface_DirectX::InitializeSwapChain()
	{
		BOOL allowTearing = FALSE;
		ComPtr<IDXGIFactory5> factory5;
		if (SUCCEEDED(m_DXAPI->GetFactory().As(&factory5)))
		{
			if (SUCCEEDED(factory5->CheckFeatureSupport(
				DXGI_FEATURE_PRESENT_ALLOW_TEARING,
				&allowTearing,
				sizeof(allowTearing))))
			{
				m_tearingSupported = allowTearing == TRUE;
			}
		}

		auto winSize = m_iAppWin->GetWinSize();
		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
		swapChainDesc.Width = static_cast<UINT>(winSize.width);
		swapChainDesc.Height = static_cast<UINT>(winSize.height);
		swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = BACK_BUFFER_COUNT;
		swapChainDesc.Scaling = DXGI_SCALING_NONE;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
		swapChainDesc.Flags = m_tearingSupported ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;

		ComPtr<IDXGISwapChain1> swapChain1;
		ensure(S_OK == m_DXAPI->GetFactory()->CreateSwapChainForHwnd(
			m_DXAPI->GetCommandQueue().Get(),
			m_Win->GetHWND(),
			&swapChainDesc,
			nullptr,
			nullptr,
			&swapChain1));

		ensure(S_OK == swapChain1.As(&m_swapChain));

		return {};
	}

	result<> surface_DirectX::CreateRTVHeap()
	{
		auto m_device = m_DXAPI->GetDevice();
		ensure(m_device, ">>>>> [surface_DirectX::CreateRTVHeap()]. Device cannot be null.");

		// Describe and create a render target view (RTV) descriptor heap.
		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
		rtvHeapDesc.NumDescriptors = BACK_BUFFER_COUNT;
		rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		HRESULT hr = m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap));
		if (FAILED(hr))
			return Unexpected(eResult::failure, std::format(L"Failed to create RTV heap. HRESULT = 0x{:08X}", hr));

		SET_RESOURCE_DEBUG_NAME(m_rtvHeap, L"RTV Heap");

		return {};
	}

	result<> surface_DirectX::CreateSRVHeap()
	{
		auto m_device = m_DXAPI->GetDevice();
		ensure(m_device, ">>>>> [surface_DirectX::CreateSRVHeap()]. Device cannot be null.");

		// Describe and create a shader resource view (SRV) heap for the texture.
		D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
		srvHeapDesc.NumDescriptors = 1;
		srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		HRESULT hr = m_device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_srvHeap));
		if (FAILED(hr))
			return Unexpected(eResult::failure, std::format(L"Failed to create SRV heap. HRESULT = 0x{:08X}", hr));

		SET_RESOURCE_DEBUG_NAME(m_srvHeap, L"SRV Heap");

		return {};
	}

	result<> surface_DirectX::CreateDSVHeap()
	{
		auto m_device = m_DXAPI->GetDevice();
		ensure(m_device, ">>>>> [surface_DirectX::CreateDSVHeap()]. Device cannot be null.");

		const D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc{
			.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV,
			.NumDescriptors = BACK_BUFFER_COUNT,
			.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
			.NodeMask = 0
		};

		HRESULT hr = m_device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_dsvHeap));
		if (FAILED(hr))
			return Unexpected(eResult::failure, std::format(L"Failed to create DSV heap. HRESULT = 0x{:08X}", hr));

		SET_RESOURCE_DEBUG_NAME(m_dsvHeap, L"DSV Heap");

		return {};
	}

	result<> surface_DirectX::CreateDS(const size2D<>& size)
	{
		auto m_device = m_DXAPI->GetDevice();
		ensure(m_device, ">>>>> [surface_DirectX::CreateDS()]. Device cannot be null.");

		// Очищаем старые ресурсы
		for (auto& ds : m_depthStencil)
			ds.Reset();

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

		// Создаём ресурсы глубины по количеству кадров
		for (UINT i = 0; i < BACK_BUFFER_COUNT; ++i)
		{
			HRESULT hr = m_device->CreateCommittedResource(
				&depthStencilHeapProps,
				D3D12_HEAP_FLAG_NONE,
				&depthStencilTextureDesc,
				D3D12_RESOURCE_STATE_COMMON,
				&depthOptimizedClearValue,
				IID_PPV_ARGS(&m_depthStencil[i]));
			
			if (S_OK != hr)
				return Unexpected(eResult::failure, std::format(L">>>>> [DXAPI::CreateDS()]. Failed to create depth stencil resource. HRESULT = 0x{:08X}", hr));

			std::wstring debugName = L"DepthStencil_" + std::to_wstring(i);
			SET_RESOURCE_DEBUG_NAME(m_depthStencil[i], debugName.c_str());

			// Создаём DSV
			CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(
				m_dsvHeap->GetCPUDescriptorHandleForHeapStart(),
				i,
				m_DsvDescrSize);

			m_device->CreateDepthStencilView(
				m_depthStencil[i].Get(),
				&depthStencilDesc,
				dsvHandle);

			SET_RESOURCE_DEBUG_NAME(m_depthStencil[i], std::format(L"DSV Frame {}", i).c_str());
		}

		return {};
	}

	void surface_DirectX::ResetRTVandDS()
	{
		for (auto& rt : m_renderTargets)
			rt.Reset();

		for (auto& ds : m_depthStencil)
			ds.Reset();
	}

	result<> surface_DirectX::RecreateRenderTargetsAndDepth()
	{
		auto m_device = m_DXAPI->GetDevice();
		auto res = CreateRTV(m_device);
		if (!res)
			return Unexpected(eResult::failure, std::format(L">>>>> [DXAPI::RecreateRenderTargetsAndDepth()]. Failed to create RTV. {}", res.error().getMessage()).c_str());

		DXGI_SWAP_CHAIN_DESC desc{};
		HRESULT hr = m_swapChain->GetDesc(&desc);
		if (S_OK != hr)
			Unexpected(eResult::failure, std::format(L">>>>> [DXAPI::RecreateRenderTargetsAndDepth()]. Failed to get swap chain description. HRESULT = 0x{:08X}", hr));

		size2D<> size{ desc.BufferDesc.Width, desc.BufferDesc.Height };
		res = CreateDS(size);
		if (!res)
			Unexpected(eResult::failure, std::format(L">>>>> [DXAPI::RecreateRenderTargetsAndDepth()]. Failed to create depth stencil view. {}", res.error().getMessage()).c_str());

		return {};
	}
#pragma endregion Initialize

#pragma region Rendring
	void surface_DirectX::BeginRender()
	{
		auto m_device = m_DXAPI->GetDevice();
		ensure(m_device, ">>>>> [surface_DirectX::BeginRender()]. Device cannot be null.");
		auto commandAllocator = m_DXAPI->GetCommandRender()->GetCommandAllocator(m_frameIndex);
		ensure(commandAllocator, ">>>>> [surface_DirectX::BeginRender()]. Command allocator cannot be null.");
		auto commandList = m_DXAPI->GetCommandRender()->GetCommandList(m_frameIndex);
		ensure(commandList, ">>>>> [surface_DirectX::BeginRender()]. Command list cannot be null.");

		ensure(S_OK == commandAllocator->Reset());
		ensure(S_OK == commandList->Reset(commandAllocator.Get(), nullptr));

		commandList->SetGraphicsRootSignature(m_DXAPI->GetRootSignature().Get());
		commandList->RSSetViewports(1, &m_viewport);
		commandList->RSSetScissorRects(1, &m_scissorRect);

		commandList->ResourceBarrier(
			1,
			&keep(
				CD3DX12_RESOURCE_BARRIER::Transition(
					m_renderTargets[m_frameIndex].Get(),
					D3D12_RESOURCE_STATE_PRESENT,
					D3D12_RESOURCE_STATE_RENDER_TARGET)
			)
		);

		commandList->ResourceBarrier(
			1,
			&keep(
				CD3DX12_RESOURCE_BARRIER::Transition(
					m_depthStencil[m_frameIndex].Get(),
					D3D12_RESOURCE_STATE_COMMON, // Предполагаем, что начальное состояние COMMON
					D3D12_RESOURCE_STATE_DEPTH_WRITE)
			)
		);

		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_RtvDescrSize);
		CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(m_dsvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_DsvDescrSize);
		commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);
	}

	void surface_DirectX::Render()
	{
		auto m_device = m_DXAPI->GetDevice();
		ensure(m_device, ">>>>> [surface_DirectX::Render()]. Device cannot be null.");
		auto commandList = m_DXAPI->GetCommandRender()->GetCommandList(m_frameIndex);
		ensure(commandList, ">>>>> [surface_DirectX::Render()]. Command list cannot be null.");

		// Записываем команды рендеринга
		const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_RtvDescrSize);
		commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr );
		CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(m_dsvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_DsvDescrSize);
		commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

		// Здесь можно добавить дополнительные команды рендеринга, например:
		// m_DXAPI->GetCommandRender()->CommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		// m_DXAPI->GetCommandRender()->CommandList()->IASetVertexBuffers(0, 1, &m_vertexBufferView);
		// m_DXAPI->GetCommandRender()->CommandList()->DrawInstanced(3, 1, 0, 0);
	}

	void surface_DirectX::EndRender()
	{
		auto commandList = m_DXAPI->GetCommandRender()->GetCommandList(m_frameIndex);
		ensure(commandList, ">>>>> [surface_DirectX::EndRender()]. Command list cannot be null.");

		// Переводим буфер рендеринга в состояние для отображения
		commandList->ResourceBarrier(
			1,
			&keep(
				CD3DX12_RESOURCE_BARRIER::Transition(
					m_renderTargets[m_frameIndex].Get(),
					D3D12_RESOURCE_STATE_RENDER_TARGET,
					D3D12_RESOURCE_STATE_PRESENT
				)
			)
		);

		commandList->ResourceBarrier(
			1,
			&keep(
				CD3DX12_RESOURCE_BARRIER::Transition(
					m_depthStencil[m_frameIndex].Get(),
					D3D12_RESOURCE_STATE_DEPTH_WRITE,
					D3D12_RESOURCE_STATE_COMMON)
			)
		);

		// Закрываем командный список
		ensure(S_OK == commandList->Close());

		// Выполняем командный список
		m_iGAPI->SubmitCommandLists(m_frameIndex);

		// Настраиваем параметры для Present
		BOOL fullscreen = FALSE;
		ensure(S_OK == m_swapChain->GetFullscreenState(&fullscreen, nullptr));

		UINT syncInterval = isVSync ? 1 : 0;
		UINT presentFlags = (!isVSync && !fullscreen && m_tearingSupported) ? DXGI_PRESENT_ALLOW_TEARING : 0;

		HRESULT hr = m_swapChain->Present(syncInterval, presentFlags);
		m_iGAPI->WaitForGpu();

		// Обновляем индекс текущего буфера
		m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
	}
#pragma endregion Rendring

	void surface_DirectX::OnResize(const size2D<>& size)
	{
		if (m_iGAPI->GetInitState() != eInitState::eInitOK && !m_swapChain || b_IgnoreResize)
			return;

		if (size.width == 0 || size.height == 0)
		{
			DebugOutput(L">>>>> [DXAPI::OnResize()]. Invalid size for resize. Width or height is zero.\n");
			return;
		}

		auto m_device = m_DXAPI->GetDevice();
		ensure(m_device, ">>>>> [surface_DirectX::CreateRTVHeap()]. Device cannot be null.");
		ensure(m_swapChain, ">>>>> [surface_DirectX::CreateRTVHeap()]. Swap chain cannot be null.");

		DXGI_SWAP_CHAIN_DESC1 desc;
		HRESULT hr = m_swapChain->GetDesc1(&desc);
		if (SUCCEEDED(hr))
		{
			// Сравниваем с новым размером
			if (desc.Width == static_cast<UINT>(size.width) &&
				desc.Height == static_cast<UINT>(size.height))
			{
				DebugOutput(std::format(L">>>>> [DXAPI::OnResize({}x{})]. No resize needed, dimensions are unchanged.\n", size.width, size.height).c_str());
				return; // Размеры не изменились
			}
		}

		m_iGAPI->WaitForGpu();
		ResetRTVandDS();

		BOOL fullscreen = FALSE;
		ensure(S_OK == m_swapChain->GetFullscreenState(&fullscreen, nullptr));
		if (fullscreen)
		{
			DXGI_MODE_DESC modeDesc = {};
			modeDesc.Width = static_cast<UINT>(size.width);
			modeDesc.Height = static_cast<UINT>(size.height);
			modeDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			modeDesc.RefreshRate.Numerator = 60;
			modeDesc.RefreshRate.Denominator = 1;

			hr = m_swapChain->ResizeTarget(&modeDesc);
		}
		else
		{
			hr = m_swapChain->ResizeBuffers(
				BACK_BUFFER_COUNT,
				static_cast<UINT>(size.width),
				static_cast<UINT>(size.height),
				DXGI_FORMAT_R8G8B8A8_UNORM,
				m_tearingSupported ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0);
		}
		if (S_OK != hr)
			throw_runtime_error(std::format(">>>>> [DXAPI::OnResize({}x{})].", size.width, size.height));

		//m_iGAPI->WaitForGpu();
		m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

		auto res = CreateRTVHeap()
			.and_then([&]() { return CreateRTV(m_device); })
			.and_then([&]() { return CreateDS(size); })
			.or_else([&](const Unexpected& error) { throw_runtime_error(std::format(">>>>> [DXAPI::OnResize({}x{})]. {}.", size.width, size.height, wstring_to_string(error.getMessage()))); });

		m_viewport = CD3DX12_VIEWPORT{ 0.0f, 0.0f, static_cast<float>(size.width), static_cast<float>(size.height) };
		m_scissorRect = CD3DX12_RECT{ 0, 0, static_cast<LONG>(size.width), static_cast<LONG>(size.height) };
		m_aspectRatio = static_cast<float>(size.width) / static_cast<float>(size.height);
	}

	void surface_DirectX::SetFullScreen(bool fs)
	{
		BOOL fullscreen = FALSE;
		HRESULT hr = m_swapChain->GetFullscreenState(&fullscreen, nullptr);
		if (FAILED(hr))
		{
			DebugOutput(std::format(L">>>>> [surface_DirectX::SetFullScreen({})] Failed to get fullscreen state. HRESULT = 0x{:08X}\n", fs, hr).c_str());
			return;
		}

		if (fs == static_cast<bool>(fullscreen))
		{
			DebugOutput(std::format(L">>>>> [surface_DirectX::SetFullScreen({})] Fullscreen state is already set.\n", fs).c_str());
			return;
		}

		m_iGAPI->WaitForGpu();
		ResetRTVandDS();

		b_IgnoreResize = true;
		hr = m_swapChain->SetFullscreenState(fs, nullptr);
		if (S_OK != hr)
		{
			b_IgnoreResize = false;
			DebugOutput(std::format(L">>>>> [surface_DirectX::SetFullScreen({})] Failed to set fullscreen state. HRESULT = 0x{:08X}\n", fs, hr).c_str());
			auto res = RecreateRenderTargetsAndDepth();
			if (!res)
				throw_runtime_error(std::format(">>>>> #0 [surface_DirectX::SetFullScreen({})]. Failed to recreate render targets and depth stencil view. {}.", fs, wstring_to_string(res.error().getMessage())));

			return;
		}
		b_IgnoreResize = false;

		DXGI_SWAP_CHAIN_DESC desc{};
		hr = m_swapChain->GetDesc(&desc);
		if (S_OK != hr)
		{
			DebugOutput(std::format(L">>>>> [surface_DirectX::SetFullScreen({})] Failed to get swap chain description. HRESULT = 0x{:08X}\n", fs, hr).c_str());
			return;
		}

		hr = m_swapChain->ResizeBuffers(
			desc.BufferCount,
			0, 0, // авто определение размеров
			desc.BufferDesc.Format,
			desc.Flags );
		if (S_OK != hr)
		{
			DebugOutput(std::format(L">>>>> [surface_DirectX::SetFullScreen({})] Failed to resize buffers. HRESULT = 0x{:08X}\n",fs, hr).c_str());
			return;
		}

		auto res = RecreateRenderTargetsAndDepth();
		if (!res)
			throw_runtime_error(std::format(">>>>> #1 [surface_DirectX::SetFullScreen({})]. Failed to recreate render targets and depth stencil view. {}.", fs, wstring_to_string(res.error().getMessage())));

		DebugOutput(std::format(L">>>>> [surface_DirectX::SetFullScreen({})].\n", fs).c_str());
	}
}
#endif // defined(_WIN64)