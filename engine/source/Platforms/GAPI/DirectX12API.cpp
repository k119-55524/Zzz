#include "pch.h"
#include "DirectX12API.h"
#include "../WinApp/WinAppMSWindows.h"

using namespace Zzz;
using namespace Zzz::Platforms;

#ifdef _GAPI_DX12

DirectX12API::DirectX12API(const shared_ptr<IWinApp> appWin) :
	IGAPI(appWin, eGAPIType::DirectX12),
	m_frameIndex{ 0 },
	m_fenceEvent{ 0 },
	m_fenceValue{ 0 },
	//m_aspectRatio{ 0 },
	m_RtvDescrSize{ 0 },
	m_DsvDescrSize{ 0 },
	m_CbvSrvDescrSize{ 0 }
{
}

DirectX12API::~DirectX12API()
{
	// Ожидаем окончания обработки ресурсов на GPU
	WaitForPreviousFrame();

	CloseHandle(m_fenceEvent);
}

void DirectX12API::Initialize()
{
	InitializePipeline();
	InitializeAssets();

	initState = e_InitState::eInitOK;
}

void DirectX12API::InitializePipeline()
{
	UINT dxgiFactoryFlags = 0;

	zSize winSize = appWin->GetWinSize();
	//m_aspectRatio = static_cast<float>(winSize->x) / static_cast<float>(winSize->y);
	m_viewport = CD3DX12_VIEWPORT{ 0.0f, 0.0f, static_cast<float>(winSize.width), static_cast<float>(winSize.height) };
	m_scissorRect = CD3DX12_RECT{ 0, 0, static_cast<LONG>(winSize.width), static_cast<LONG>(winSize.height) };

#if _DEBUG
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
	if (S_OK != CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory)))
		THROW_RUNTIME_ERROR("CreateDXGIFactory2( ... )");

	ComPtr<IDXGIAdapter1> adapter;
	GetAdapter(factory.Get(), &adapter);

	if (S_OK != D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&m_device)))
		THROW_RUNTIME_ERROR("D3D12CreateDevice( ... )");

	m_RtvDescrSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	m_DsvDescrSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	m_CbvSrvDescrSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	// Проверка уровня биндинга
	//{
	//	D3D12_FEATURE_DATA_D3D12_OPTIONS d3d12Options;
	//	ThrowIfFailed(m_device->CheckFeatureSupport(
	//		D3D12_FEATURE_D3D12_OPTIONS,
	//		&d3d12Options,
	//		sizeof(d3d12Options)
	//	));

	//	switch (d3d12Options.ResourceBindingTier)
	//	{
	//	case D3D12_RESOURCE_BINDING_TIER_3:
	//		// 3 уровень накладывает меньше ограничений на размеры таблиц ресурсов(их хэндлов)
	//		break;
	//	}
	//}

		// Настройка и создание CommandQueue.
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	if(S_OK != m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue)))
		THROW_RUNTIME_ERROR("m_device->CreateCommandQueue( ... )");

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
	if (S_OK !=
		factory->CreateSwapChainForHwnd(
			m_commandQueue.Get(),	// SwapChain нужна CommandQueue, чтобы она могла принудительно очистить ее.
			dynamic_cast<WinAppMSWindows*>(appWin.get())->GetHWND(),
			&swapChainDesc,
			nullptr,
			nullptr,
			&swapChain)
		)
		THROW_RUNTIME_ERROR("CreateSwapChainForHwnd( ... )");

	// This sample does not support fullscreen transitions.
	if (S_OK != factory->MakeWindowAssociation(dynamic_cast<WinAppMSWindows*>(appWin.get())->GetHWND(), DXGI_MWA_NO_ALT_ENTER))
		THROW_RUNTIME_ERROR("MakeWindowAssociation( ... )");

	if (S_OK != swapChain.As(&m_swapChain))
		THROW_RUNTIME_ERROR("swapChain.As( ... )");
	m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

	// Create descriptor heaps.
	{
		// Describe and create a render target view (RTV) descriptor heap.
		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
		rtvHeapDesc.NumDescriptors = FrameCount;
		rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		if (S_OK != m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap)))
			THROW_RUNTIME_ERROR("CreateDescriptorHeap( ... )");

		//D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
		//dsvHeapDesc.NumDescriptors = 1;
		//dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		//dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		//ThrowIfFailed(m_device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_dsvHeap)));
	}

	// Create frame resources.
	{
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());

		// Create a RTV for each frame.
		for (UINT n = 0; n < FrameCount; n++)
		{
			if (S_OK != m_swapChain->GetBuffer(n, IID_PPV_ARGS(&m_renderTargets[n])))
				THROW_RUNTIME_ERROR("GetBuffer( ... )");

			m_device->CreateRenderTargetView(m_renderTargets[n].Get(), nullptr, rtvHandle);
			rtvHandle.Offset(1, m_RtvDescrSize);
		}
	}

	// Create the depth stencil view.
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
	}

	if (S_OK != m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator)))
		THROW_RUNTIME_ERROR("CreateCommandAllocator( ... )");
}

void DirectX12API::InitializeAssets()
{
	if (S_OK != m_rootSignature.Initialize(m_device))
		THROW_RUNTIME_ERROR("m_rootSignature.Initialize( ... )");

	// Create the command list.
	if (S_OK != m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator.Get(), nullptr, IID_PPV_ARGS(&m_commandList)))
		THROW_RUNTIME_ERROR("CreateCommandList( ... )");

	// К одному ID3D12CommandAllocator(m_commandAllocator) можно привязать несколько ID3D12GraphicsCommandList(m_commandList) но одновременно записывать можно только в один.
	// А остальные ID3D12GraphicsCommandList, привязанные к ID3D12CommandAllocator, должны быть закрыты.
	// Так как ID3D12GraphicsCommandList при создании всегда открыт, сразу закрываем его.
	if (S_OK != m_commandList->Close())
		THROW_RUNTIME_ERROR("m_commandList->Close()");

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
		if (S_OK != m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)))
			THROW_RUNTIME_ERROR("m_device->CreateFence( ... )");

		m_fenceValue = 1;

		// Create an event handle to use for frame synchronization.
		m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		if (m_fenceEvent == nullptr)
		{
			if (S_OK != HRESULT_FROM_WIN32(GetLastError()))
				THROW_RUNTIME_ERROR("HRESULT_FROM_WIN32(GetLastError())");
		}
	}

	//gapiInterfaces = make_shared<IGAPI>(m_device, m_rootSignature, m_commandList);
}

void DirectX12API::GetAdapter(_In_ IDXGIFactory1* pFactory, _Outptr_result_maybenull_ IDXGIAdapter1** ppAdapter)
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

#ifdef _DEBUG
			OutputDebugString(L">>>>> [DirectX12API::GetAdapter()]. EnumAdapterByGpuPreference: ");
			OutputDebugString(desc.Description);
			OutputDebugString(L"\n");
#endif

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

			OutputDebugString(_T(">>>>> [DirectX12API::GetAdapter()]. EnumAdapters1: "));
			OutputDebugString(desc.Description);
			OutputDebugString(_T("\n"));

			// Пропускаем Basic Render Driver адаптер.
			if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
				continue;

			// Проверяем, поддерживает ли адаптер Direct3D 12(без фактического создания адаптера)
			if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
				break;
		}
	}

	{
		IDXGIAdapter3* pDXGIAdapter3;
		if (S_OK != adapter->QueryInterface(IID_PPV_ARGS(&pDXGIAdapter3)))
			THROW_RUNTIME_ERROR("adapter->QueryInterface(IID_PPV_ARGS(&pDXGIAdapter3))");

		// Now I can query video/system memory usage.
		DXGI_QUERY_VIDEO_MEMORY_INFO vm_info;
		if (S_OK != pDXGIAdapter3->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &vm_info))
			THROW_RUNTIME_ERROR("pDXGIAdapter3->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &vm_info)");

		if (S_OK != pDXGIAdapter3->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_NON_LOCAL, &vm_info))
			THROW_RUNTIME_ERROR("pDXGIAdapter3->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_NON_LOCAL, &vm_info)");
	}

	*ppAdapter = adapter.Detach();
}

void DirectX12API::WaitForPreviousFrame()
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
	if (S_OK != m_commandQueue->Signal(m_fence.Get(), fence))
		THROW_RUNTIME_ERROR("m_commandQueue->Signal(m_fence.Get(), fence)");

	m_fenceValue++;

	// Ожидаем завершения предыдущего кадра.
	if (m_fence->GetCompletedValue() < fence)
	{
		if (S_OK != m_fence->SetEventOnCompletion(fence, m_fenceEvent))
			THROW_RUNTIME_ERROR("m_fence->SetEventOnCompletion(fence, m_fenceEvent)");

		WaitForSingleObject(m_fenceEvent, INFINITE);
	}

	m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
}

void DirectX12API::OnUpdate()
{
}

void DirectX12API::OnRender()
{
	// Record all the commands we need to render the scene into the command list.
	PopulateCommandList();

	// Execute the command list.
	ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	// Present the frame.
	if (S_OK != m_swapChain->Present(1, 0))
		THROW_RUNTIME_ERROR("m_swapChain->Present(1, 0)");

	WaitForPreviousFrame();
}

void DirectX12API::PopulateCommandList()
{
	// Command list allocators can only be reset when the associated 
	// command lists have finished execution on the GPU; apps should use 
	// fences to determine GPU execution progress.
	if (S_OK != m_commandAllocator->Reset())
		THROW_RUNTIME_ERROR("m_commandAllocator->Reset()");

	// However, when ExecuteCommandList() is called on a particular command 
	// list, that command list can then be reset at any time and must be before 
	// re-recording.
	//ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), m_pipelineState.Get()));
	if (S_OK != m_commandList->Reset(m_commandAllocator.Get(), nullptr))
		THROW_RUNTIME_ERROR("m_commandList->Reset( ... )");

	// Set necessary state.
	m_commandList->SetGraphicsRootSignature(m_rootSignature.Get().Get());
	m_commandList->RSSetViewports(1, &m_viewport);
	m_commandList->RSSetScissorRects(1, &m_scissorRect);

	// Indicate that the back buffer will be used as a render target.
	m_commandList->ResourceBarrier(
		1,
		&keep(
			CD3DX12_RESOURCE_BARRIER::Transition(
				m_renderTargets[m_frameIndex].Get(),
				D3D12_RESOURCE_STATE_PRESENT,
				D3D12_RESOURCE_STATE_RENDER_TARGET)
		)
	);

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_RtvDescrSize);
	m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

	// Record commands.
	const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
	m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
	//m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//m_commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
	//m_commandList->DrawInstanced(3, 1, 0, 0);

	// Indicate that the back buffer will now be used to present.
	m_commandList->ResourceBarrier(
		1,
		&keep(
			CD3DX12_RESOURCE_BARRIER::Transition(
				m_renderTargets[m_frameIndex].Get(),
				D3D12_RESOURCE_STATE_RENDER_TARGET,
				D3D12_RESOURCE_STATE_PRESENT
			)
		)
	);

	if(S_OK != m_commandList->Close())
		THROW_RUNTIME_ERROR("m_commandList->Close()");
}

void DirectX12API::OnResize(const zSize& size)
{
}

#endif // _GAPI_DX12