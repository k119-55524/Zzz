#include "pch.h"
export module DXAPI;

using namespace zzz::platforms::directx;

#if defined(_WIN64)
import IGAPI;
import result;
import swMSWin;
import strConver;
import RootSignature;

using namespace zzz::result;

namespace zzz::platforms
{
#if defined(_DEBUG)
#define SET_RESOURCE_DEBUG_NAME(resource, name) {			\
	std::wstring debugName = L">>>>> ";						\
	std::wstring tempName = name;							\
	if (!tempName.empty()) {								\
	    debugName += tempName;								\
	}														\
	debugName += L"\n+--- [" + std::wstring(__FILEW__) +	\
	L"]\n+--- [" + std::wstring(__FUNCTIONW__) +			\
	L"]\n+--- [Line " + std::to_wstring(__LINE__) + L"]";	\
	resource->SetName(debugName.c_str());					\
}
#else
#define SET_RESOURCE_DEBUG_NAME(resource, name) {}
#endif
}

export namespace zzz::platforms
{
	class DXAPI final : public IGAPI
	{
	public:
		class CommandWrapper
		{
		public:
			CommandWrapper() = delete;
			CommandWrapper(CommandWrapper&) = delete;
			CommandWrapper(CommandWrapper&&) = delete;
			CommandWrapper(ComPtr<ID3D12Device>& m_device)
			{
				Initialize(m_device);
			};

			inline const ComPtr<ID3D12CommandAllocator>& CommandAllocator() const noexcept { return m_commandAllocator; };
			inline const ComPtr<ID3D12GraphicsCommandList>& CommandList() const noexcept { return m_commandList; };

		private:
			void Initialize(ComPtr<ID3D12Device>& m_device)
			{
				ensure(S_OK == m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator)));

				// К одному ID3D12CommandAllocator(m_commandAllocator) можно привязать несколько ID3D12GraphicsCommandList(m_commandList) но одновременно записывать можно только в один.
				// А остальные ID3D12GraphicsCommandList, привязанные к ID3D12CommandAllocator, должны быть закрыты.
				// Так как ID3D12GraphicsCommandList при создании всегда открыт, сразу закрываем его.
				ensure(S_OK == m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator.Get(), nullptr, IID_PPV_ARGS(&m_commandList)));
				ensure(S_OK == m_commandList->Close());
			}

			ComPtr<ID3D12CommandAllocator> m_commandAllocator;
			ComPtr<ID3D12GraphicsCommandList> m_commandList;
		};

		explicit DXAPI();
		DXAPI(DXAPI&) = delete;
		DXAPI(DXAPI&&) = delete;

		DXAPI& operator=(const DXAPI&) = delete;

		virtual ~DXAPI() override;

	protected:
		virtual zResult<> Initialize(const std::shared_ptr<ISuperWidget> appWin) override;

		void OnUpdate() override;
		void OnRender() override;
		void OnResize(const zSize2D<>& size) override;

	private:
		static const UINT FrameCount = 2;

		RootSignature m_rootSignature;
		CD3DX12_VIEWPORT m_viewport;
		CD3DX12_RECT m_scissorRect;

		ComPtr<IDXGIAdapter1> m_adapter;
		ComPtr<IDXGIAdapter3> m_adapter3;
		ComPtr<ID3D12Device> m_device;
		ComPtr<ID3D12CommandQueue> m_commandQueue;
		ComPtr<IDXGISwapChain3> m_swapChain;
		ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
		ComPtr<ID3D12DescriptorHeap> m_srvHeap;
		ComPtr<ID3D12Resource> m_renderTargets[FrameCount];

		std::mutex commandMutex;
		std::unique_ptr<CommandWrapper> m_commandRender;
		std::unique_ptr<CommandWrapper> m_commandAccum;

		UINT m_RtvDescrSize;
		UINT m_DsvDescrSize;
		UINT m_CbvSrvDescrSize;

		UINT m_frameIndex;
		HANDLE m_fenceEvent;
		ComPtr<ID3D12Fence> m_fence;
		UINT64 m_fenceValue;

		zResult<> InitializePipeline(const std::shared_ptr<ISuperWidget> appWin);
		zResult<> InitializeAssets();
		zResult<> GetAdapter(_In_ IDXGIFactory1* pFactory, _Outptr_result_maybenull_ IDXGIAdapter1** ppAdapter);
		zResult<> CreateRTVHeap();
		zResult<> CreateSRVHeap();
		zResult<> CreateDSVHeap();
		zResult<> CreateDSView(const zSize2D<>& size);

		void PopulateCommandList();
		void WaitForPreviousFrame();

		// Создана только для обхода ограничений VisualStudio при передаче параметров
		template <class T>
		constexpr inline auto& keep(T&& x) noexcept
		{
			return x;
		}
	};

	DXAPI::DXAPI() :
		IGAPI(eGAPIType::DirectX12),
		m_frameIndex{ 0 },
		m_fenceEvent{ 0 },
		m_fenceValue{ 0 },
		////m_aspectRatio{ 0 },
		m_RtvDescrSize{ 0 },
		m_DsvDescrSize{ 0 },
		m_CbvSrvDescrSize{ 0 }
	{
	}

	DXAPI::~DXAPI()
	{
		WaitForPreviousFrame();
		CloseHandle(m_fenceEvent);
	}

	void DXAPI::WaitForPreviousFrame()
	{
		// TODO Переделывать
		// ОЖИДАНИЕ ЗАВЕРШЕНИЯ КАДРА ПЕРЕД ПРОДОЛЖЕНИЕМ — НЕ ЛУЧШАЯ ПРАКТИКА.
		// This is code implemented as such for simplicity. The D3D12HelloFrameBuffering
		// sample illustrates how to use fences for efficient resource usage and to
		// maximize GPU utilization.

		// Add an instruction to the command queue to set a new fence point.
		// Because we are on the GPU timeline, the new fence point won’t be
		// set until the GPU finishes processing all the commands prior to
		// this Signal().
		// 
		// Signal and increment the fence value.

		const UINT64 fence = m_fenceValue;
		ensure(S_OK == m_commandQueue->Signal(m_fence.Get(), fence));

		m_fenceValue++;

		// Ожидаем завершения предыдущего кадра.
		if (m_fence->GetCompletedValue() < fence)
		{
			ensure(S_OK == m_fence->SetEventOnCompletion(fence, m_fenceEvent));

			WaitForSingleObject(m_fenceEvent, INFINITE);
		}

		m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
	}

	zResult<> DXAPI::Initialize(const std::shared_ptr<ISuperWidget> appWin)
	{
		auto res = InitializePipeline(appWin)
			.and_then([&]() { return InitializeAssets(); });
		
		if(res)
			initState = eInitState::eInitOK;

		return res;
	}

	zResult<> DXAPI::InitializePipeline(const std::shared_ptr<ISuperWidget> appWin)
	{
		UINT dxgiFactoryFlags = 0;

		zSize2D winSize = appWin->GetWinSize();
		//m_aspectRatio = static_cast<float>(winSize->x) / static_cast<float>(winSize->y);
		m_viewport = CD3DX12_VIEWPORT{ 0.0f, 0.0f, static_cast<float>(winSize.width), static_cast<float>(winSize.height) };
		m_scissorRect = CD3DX12_RECT{ 0, 0, static_cast<LONG>(winSize.width), static_cast<LONG>(winSize.height) };

#if defined(_DEBUG)
		// Включаем уровень отладки.
		{
			ComPtr<ID3D12Debug> debugController;
			if (S_OK == D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))
			{
				debugController->EnableDebugLayer();

				// Включить дополнительные уровни отладки.
				dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
			}
		}
#endif

		ComPtr<IDXGIFactory4> factory;
		ensure(S_OK == CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory)));

		zResult<> res = GetAdapter(factory.Get(), &m_adapter);
		if (!res)
			return Unexpected(eResult::failure, L">>>>> [DXAPI::InitializePipeline()]. Failed to get adapter. More specifically: " + res.error().getMessage());

		ensure(S_OK == m_adapter.As(&m_adapter3));
		ensure(S_OK == D3D12CreateDevice(m_adapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&m_device)));
		SET_RESOURCE_DEBUG_NAME(m_device, L"Main ID3D12Device");

		m_RtvDescrSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		m_DsvDescrSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
		m_CbvSrvDescrSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		// Настройка и создание CommandQueue.
		D3D12_COMMAND_QUEUE_DESC queueDesc = {};
		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		ensure(S_OK == m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue)));

		// Настройка и создание SwapChain
		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
		swapChainDesc.BufferCount = FrameCount;
		swapChainDesc.Width = static_cast<UINT>(winSize.width);
		swapChainDesc.Height = static_cast<UINT>(winSize.width);
		swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapChainDesc.SampleDesc.Count = 1;

		ComPtr<IDXGISwapChain1> swapChain;
		ensure(S_OK ==
			factory->CreateSwapChainForHwnd(
				m_commandQueue.Get(),	// SwapChain нужна CommandQueue, чтобы она могла принудительно очистить ее.
				dynamic_cast<swMSWin*>(appWin.get())->GetHWND(),
				&swapChainDesc,
				nullptr,
				nullptr,
				&swapChain)
		);

		// Этот пример не поддерживает полноэкранные переходы.
		ensure(S_OK == factory->MakeWindowAssociation(dynamic_cast<swMSWin*>(appWin.get())->GetHWND(), DXGI_MWA_NO_ALT_ENTER));
		ensure(S_OK == swapChain.As(&m_swapChain));
		m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

		res = CreateRTVHeap()
			.and_then([&]() { return CreateSRVHeap(); })
			.and_then([&]() { return CreateDSVHeap(); });
		if (!res)
			return Unexpected(eResult::failure, L">>>>> [DXAPI::InitializePipeline()]. -> " + res.error().getMessage());

		// Create frame resources.
		{
			CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());

			// Create a RTV for each frame.
			for (UINT n = 0; n < FrameCount; n++)
			{
				ensure(S_OK == m_swapChain->GetBuffer(n, IID_PPV_ARGS(&m_renderTargets[n])));

				m_device->CreateRenderTargetView(m_renderTargets[n].Get(), nullptr, rtvHandle);
				rtvHandle.Offset(1, m_RtvDescrSize);
			}
		}

		res = CreateDSView(winSize);
		if (!res)
			return Unexpected(eResult::failure, L">>>>> [DXAPI::InitializePipeline()]. -> " + res.error().getMessage());

		m_commandRender = safe_make_unique<CommandWrapper>(m_device);
		m_commandAccum = safe_make_unique<CommandWrapper>(m_device);

		return {};
	}

	zResult<> DXAPI::CreateDSView(const zSize2D<>& size)
	{
		//D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = {};
		//depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
		//depthStencilDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		//depthStencilDesc.Flags = D3D12_DSV_FLAG_NONE;

		//D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
		//depthOptimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
		//depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
		//depthOptimizedClearValue.DepthStencil.Stencil = 0;

		//const CD3DX12_HEAP_PROPERTIES depthStencilHeapProps(D3D12_HEAP_TYPE_DEFAULT);
		//const CD3DX12_RESOURCE_DESC depthStencilTextureDesc =
		//	CD3DX12_RESOURCE_DESC::Tex2D(
		//		DXGI_FORMAT_D32_FLOAT,
		//		static_cast<UINT>(winSize->x),
		//		static_cast<UINT>(winSize->y),
		//		1, 0, 1, 0,
		//		D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL
		//	);

		//ThrowIfFailed(m_device->CreateCommittedResource(
		//	&depthStencilHeapProps,
		//	D3D12_HEAP_FLAG_NONE,
		//	&depthStencilTextureDesc,
		//	D3D12_RESOURCE_STATE_DEPTH_WRITE,
		//	&depthOptimizedClearValue,
		//	IID_PPV_ARGS(&m_depthStencil)
		//));

		//NAME_D3D12_OBJECT(m_depthStencil);

		//m_device->CreateDepthStencilView(m_depthStencil.Get(), &depthStencilDesc, m_dsvHeap->GetCPUDescriptorHandleForHeapStart());
		return {};
	}

	zResult<> DXAPI::CreateRTVHeap()
	{
		// Describe and create a render target view (RTV) descriptor heap.
		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
		rtvHeapDesc.NumDescriptors = FrameCount;
		rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		ensure(S_OK == m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap)));

		return {};
	}

	zResult<> DXAPI::CreateSRVHeap()
	{
		// Describe and create a shader resource view (SRV) heap for the texture.
		D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
		srvHeapDesc.NumDescriptors = 1;
		srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		ensure(S_OK == m_device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_srvHeap)));

		return {};
	}

	zResult<> DXAPI::CreateDSVHeap()
	{ 
		//D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
		//dsvHeapDesc.NumDescriptors = 1;
		//dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		//dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		//ThrowIfFailed(m_device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_dsvHeap)));

		return {};
	}

	zResult<> DXAPI::GetAdapter(_In_ IDXGIFactory1* pFactory, _Outptr_result_maybenull_ IDXGIAdapter1** ppAdapter)
	{
		*ppAdapter = nullptr;
		ComPtr<IDXGIAdapter1> adapter;
		ComPtr<IDXGIFactory6> factory6;
		if (S_OK == pFactory->QueryInterface(IID_PPV_ARGS(&factory6)))
		{
			for (
				UINT adapterIndex = 0;
				S_OK == factory6->EnumAdapterByGpuPreference(
					adapterIndex,
					DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
					IID_PPV_ARGS(&adapter));
					++adapterIndex)
			{
				DXGI_ADAPTER_DESC1 desc;
				adapter->GetDesc1(&desc);

				OutputDebugString(L">>>>> [DirectX12API::GetAdapter()]. EnumAdapterByGpuPreference: ");
				OutputDebugString(desc.Description);
				OutputDebugString(L"\n");

				// Пропускаем Basic Render Driver адаптер.
				if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
					continue;

				// Проверяем, поддерживает ли адаптер Direct3D 12(без фактического создания адаптера)
				if (S_OK == D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_0, _uuidof(ID3D12Device), nullptr))
					break;
			}
		}

		// Если подходящий адаптер создать не удалось пытаемся найти другим способом
		if (adapter.Get() == nullptr)
		{
			for (
				UINT adapterIndex = 0;
				S_OK == pFactory->EnumAdapters1(adapterIndex, &adapter);
				++adapterIndex)
			{
				DXGI_ADAPTER_DESC1 desc;
				adapter->GetDesc1(&desc);

				OutputDebugString(L">>>>> [DirectX12API::GetAdapter()]. EnumAdapters1: ");
				OutputDebugString(desc.Description);
				OutputDebugString(L"\n");

				// Пропускаем Basic Render Driver адаптер.
				if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
					continue;

				// Проверяем, поддерживает ли адаптер Direct3D 12(без фактического создания адаптера)
				if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
					break;
			}
		}

		{
			ComPtr<IDXGIAdapter3> pDXGIAdapter3;
			ensure(S_OK == adapter->QueryInterface(IID_PPV_ARGS(&pDXGIAdapter3)));

			// Now I can query video/system memory usage.
			DXGI_QUERY_VIDEO_MEMORY_INFO vm_info;
			ensure(S_OK == pDXGIAdapter3->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &vm_info));
			ensure(S_OK == pDXGIAdapter3->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_NON_LOCAL, &vm_info));
		}

		*ppAdapter = adapter.Detach();

		return zResult<void>{};
	}

	zResult<> DXAPI::InitializeAssets()
	{
		auto res = m_rootSignature.Initialize(m_device);
		if (!res)
			return Unexpected(eResult::failure, L">>>>> [DXAPI::InitializeAssets()]. -> " + res.error().getMessage());

		// Create the vertex buffer.
		{
			// Define the geometry for a triangle.
			//Vertex_PosCol triangleVertices[] =
			//{
			//	{ { 0.0f, 0.25f * m_aspectRatio, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
			//	{ { 0.25f, -0.25f * m_aspectRatio, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
			//	{ { -0.25f, -0.25f * m_aspectRatio, 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } }
			//};

			//const UINT vertexBufferSize = sizeof(triangleVertices);

			//// Note: using upload heaps to transfer static data like vert buffers is not 
			//// recommended. Every time the GPU needs it, the upload heap will be marshalled 
			//// over. Please read up on Default Heap usage. An upload heap is used here for 
			//// code simplicity and because there are very few verts to actually transfer.
			//ThrowIfFailed(m_device->CreateCommittedResource(
			//	&keep(CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD)),
			//	D3D12_HEAP_FLAG_NONE,
			//	&keep(CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize)),
			//	D3D12_RESOURCE_STATE_GENERIC_READ,
			//	nullptr,
			//	IID_PPV_ARGS(&m_vertexBuffer)));

			//// Copy the triangle data to the vertex buffer.
			//UINT8* pVertexDataBegin;
			//CD3DX12_RANGE readRange(0, 0);        // We do not intend to read from this resource on the CPU.
			//ThrowIfFailed(m_vertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));
			//memcpy(pVertexDataBegin, triangleVertices, sizeof(triangleVertices));
			//m_vertexBuffer->Unmap(0, nullptr);

			//// Initialize the vertex buffer view.
			//m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
			//m_vertexBufferView.StrideInBytes = sizeof(Vertex_PosCol);
			//m_vertexBufferView.SizeInBytes = vertexBufferSize;
		}

		// Create synchronization objects.
		{
			ensure(S_OK == m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));

			m_fenceValue = 1;

			// Create an event handle to use for frame synchronization.
			m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
			if (m_fenceEvent == nullptr)
			{
				ensure(S_OK == HRESULT_FROM_WIN32(GetLastError()));
			}
		}

		//gapiInterfaces = make_shared<IGAPI>(m_device, m_rootSignature, m_commandList);
		return {};
	}

	void DXAPI::OnUpdate()
	{
	}

	void DXAPI::OnRender()
	{
		// Record all the commands we need to render the scene into the command list.
		PopulateCommandList();

		// Execute the command list.
		ID3D12CommandList* ppCommandLists[] = { m_commandRender->CommandList().Get() };
		m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

		// Present the frame.
		ensure(S_OK == m_swapChain->Present(1, 0));

		WaitForPreviousFrame();
	}

	void DXAPI::PopulateCommandList()
	{
		// Command list allocators can only be reset when the associated 
		// command lists have finished execution on the GPU; apps should use 
		// fences to determine GPU execution progress.
		ensure(S_OK == m_commandRender->CommandAllocator()->Reset());

		// However, when ExecuteCommandList() is called on a particular command 
		// list, that command list can then be reset at any time and must be before 
		// re-recording.
		//ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), m_pipelineState.Get()));
		ensure(S_OK == m_commandRender->CommandList()->Reset(m_commandRender->CommandAllocator().Get(), nullptr));

		// Set necessary state.
		m_commandRender->CommandList()->SetGraphicsRootSignature(m_rootSignature.Get().Get());
		m_commandRender->CommandList()->RSSetViewports(1, &m_viewport);
		m_commandRender->CommandList()->RSSetScissorRects(1, &m_scissorRect);

		// Indicate that the back buffer will be used as a render target.
		m_commandRender->CommandList()->ResourceBarrier(
			1,
			&keep(
				CD3DX12_RESOURCE_BARRIER::Transition(
					m_renderTargets[m_frameIndex].Get(),
					D3D12_RESOURCE_STATE_PRESENT,
					D3D12_RESOURCE_STATE_RENDER_TARGET)
			)
		);

		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_RtvDescrSize);
		m_commandRender->CommandList()->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

		// Record commands.
		const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
		m_commandRender->CommandList()->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
		//m_commandListRender->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		//m_commandListRender->IASetVertexBuffers(0, 1, &m_vertexBufferView);
		//m_commandListRender->DrawInstanced(3, 1, 0, 0);

		// Indicate that the back buffer will now be used to present.
		m_commandRender->CommandList()->ResourceBarrier(
			1,
			&keep(
				CD3DX12_RESOURCE_BARRIER::Transition(
					m_renderTargets[m_frameIndex].Get(),
					D3D12_RESOURCE_STATE_RENDER_TARGET,
					D3D12_RESOURCE_STATE_PRESENT
				)
			)
		);

		ensure(S_OK == m_commandRender->CommandList()->Close());
	}

	void DXAPI::OnResize(const zSize2D<>& size)
	{
		if (initState != eInitState::eInitOK && !m_swapChain)
			return;

		// Ждём завершения GPU-работы перед изменением буферов
		WaitForPreviousFrame();

		// Освобождаем старые ресурсы
		for (UINT i = 0; i < FrameCount; i++)
		{
			m_renderTargets[i].Reset();
		}
		m_rtvHeap.Reset();

		// Изменяем размер буферов swap chain
		DXGI_SWAP_CHAIN_DESC desc = {};
		m_swapChain->GetDesc(&desc);
		HRESULT hr = m_swapChain->ResizeBuffers(
			FrameCount,
			static_cast<UINT>(size.width),
			static_cast<UINT>(size.height),
			desc.BufferDesc.Format,
			desc.Flags
		);
		if (FAILED(hr))
			throw_runtime_error(std::format(">>>>> [DXAPI::OnResize({}x{})].", std::to_string(size.width), std::to_string(size.height)));

		m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

		// Пересоздаём RTV heap
		auto res = CreateRTVHeap();
		if (!res)
			throw_runtime_error(std::format(">>>>> [DXAPI::OnResize({}x{})]. {}.", std::to_string(size.width), std::to_string(size.height), wstring_to_string(res.error().getMessage())));

		// Пересоздаём render target view для каждого буфера
		for (UINT i = 0; i < FrameCount; i++)
		{
			Microsoft::WRL::ComPtr<ID3D12Resource> backBuffer;
			m_swapChain->GetBuffer(i, IID_PPV_ARGS(&backBuffer));
			m_device->CreateRenderTargetView(backBuffer.Get(), nullptr,
				CD3DX12_CPU_DESCRIPTOR_HANDLE(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(),
					i, m_RtvDescrSize));
			m_renderTargets[i] = backBuffer;
		}

		// Если есть depth-stencil — пересоздаём
		res = CreateDSView(size);
		if (!res)
			throw_runtime_error(std::format(">>>>> [DXAPI::OnResize({}x{})]. {}.", std::to_string(size.width), std::to_string(size.height), wstring_to_string(res.error().getMessage())));
	}
}
#endif // _WIN64