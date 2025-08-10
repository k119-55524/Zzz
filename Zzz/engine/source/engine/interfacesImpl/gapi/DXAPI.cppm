#include "pch.h"
export module DXAPI;

using namespace zzz::platforms::directx;

#if defined(_WIN64)
import IGAPI;
import result;
import swMSWin;
import strConver;
import RootSignature;

using namespace zzz;

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
		virtual result<> Initialize(const std::shared_ptr<ISuperWidget> appWin) override;

		void OnUpdate() override;
		void OnRender() override;
		void OnResize(const zSize2D<>& size) override;

	private:
		static const UINT FrameCount = 2;
		UINT m_swapChainFlags;
		float m_aspectRatio;

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
		ComPtr<ID3D12DescriptorHeap> m_dsvHeap;
		ComPtr<ID3D12Resource> m_renderTargets[FrameCount];
		ComPtr<ID3D12Resource> m_depthStencil;

		std::mutex commandMutex;
		std::unique_ptr<CommandWrapper> m_commandRender;
		std::unique_ptr<CommandWrapper> m_commandAccum;

		UINT m_RtvDescrSize;
		UINT m_DsvDescrSize;
		UINT m_CbvSrvDescrSize;

		UINT m_frameIndex;
		UINT64 m_fenceValue;
		unique_handle m_fenceEvent;
		ComPtr<ID3D12Fence> m_fence;

		result<> InitializePipeline(const std::shared_ptr<ISuperWidget> appWin);
		void CheckDirectX12UltimateSupport();
		result<> InitializeAssets();
		result<> GetAdapter(_In_ IDXGIFactory1* pFactory, _Outptr_result_maybenull_ IDXGIAdapter1** ppAdapter);
		result<> CreateRTVHeap();
		result<> CreateSRVHeap();
		result<> CreateDSVHeap();
		result<> CreateDS(const zSize2D<>& size);

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
		m_swapChainFlags{ 0 },
		m_fenceValue{ 0 },
		m_aspectRatio{ 0.0f },
		m_RtvDescrSize{ 0 },
		m_DsvDescrSize{ 0 },
		m_CbvSrvDescrSize{ 0 }
	{
	}

	DXAPI::~DXAPI()
	{
		WaitForPreviousFrame();
	}

	void DXAPI::WaitForPreviousFrame()
	{
		const UINT64 fence = m_fenceValue;
		ensure(S_OK == m_commandQueue->Signal(m_fence.Get(), fence));

		m_fenceValue++;

		// Ожидаем завершения предыдущего кадра.
		if (m_fence->GetCompletedValue() < fence)
		{
			ensure(S_OK == m_fence->SetEventOnCompletion(fence, m_fenceEvent.handle));

			WaitForSingleObject(m_fenceEvent.handle, INFINITE);
		}

		m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
	}

	result<> DXAPI::Initialize(const std::shared_ptr<ISuperWidget> appWin)
	{
		auto res = InitializePipeline(appWin)
			.and_then([&]() { return InitializeAssets(); });
		
		if(res)
			initState = eInitState::eInitOK;

		return res;
	}

	result<> DXAPI::InitializePipeline(const std::shared_ptr<ISuperWidget> appWin)
	{
		UINT dxgiFactoryFlags = 0;

		zSize2D winSize = appWin->GetWinSize();
		//m_aspectRatio = static_cast<float>(winSize->x) / static_cast<float>(winSize->y);
		m_viewport = CD3DX12_VIEWPORT{ 0.0f, 0.0f, static_cast<float>(winSize.width), static_cast<float>(winSize.height) };
		m_scissorRect = CD3DX12_RECT{ 0, 0, static_cast<LONG>(winSize.width), static_cast<LONG>(winSize.height) };
		m_aspectRatio = static_cast<float>(winSize.width) / static_cast<float>(winSize.height);

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

		ComPtr<IDXGIFactory7> factory;
		HRESULT hr = CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory));
		if (FAILED(hr))
		{
			// Fallback на более старую версию если новая недоступна
			ComPtr<IDXGIFactory4> factory4;
			hr = CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory4));
			if (FAILED(hr))
				return Unexpected(eResult::failure,
					std::format(L">>>>> [DXAPI::InitializePipeline()]. Failed to create DXGI Factory. HRESULT = 0x{:08X}", hr));

			hr = factory4.As(&factory);
			if (FAILED(hr))
				return Unexpected(eResult::failure,
					std::format(L">>>>> [DXAPI::InitializePipeline()]. Failed to query IDXGIFactory7. HRESULT = 0x{:08X}", hr));
		}

		result<> res = GetAdapter(factory.Get(), &m_adapter);
		if (!res)
			return Unexpected(eResult::failure, L">>>>> [DXAPI::InitializePipeline()]. Failed to get adapter. More specifically: " + res.error().getMessage());

		hr = m_adapter.As(&m_adapter3);
		if (FAILED(hr))
			return Unexpected(eResult::failure, std::format(L">>>>> [DXAPI::InitializePipeline()]. Failed to query IDXGIAdapter3. HRESULT = 0x{:08X}", hr));

		// Пытаемся создать устройство с максимальным Feature Level
		D3D_FEATURE_LEVEL featureLevels[] = {
			D3D_FEATURE_LEVEL_12_2,  // Поддержка DirectX 12 Ultimate
			D3D_FEATURE_LEVEL_12_1,  // Расширенные возможности D3D12
			D3D_FEATURE_LEVEL_12_0   // Базовый D3D12
		};

		// Пытаемся создать устройство с максимально возможным уровнем
		D3D_FEATURE_LEVEL achievedFeatureLevel = D3D_FEATURE_LEVEL_11_0;
		for (auto level : featureLevels)
		{
			hr = D3D12CreateDevice(
				m_adapter.Get(),
				level,
				IID_PPV_ARGS(&m_device)
			);

			if (SUCCEEDED(hr))
			{
				achievedFeatureLevel = level;
				break;
			}
		}
		if (FAILED(hr))
			return Unexpected(eResult::failure, std::format(L">>>>> [DXAPI::InitializePipeline()]. Failed to create D3D12 device. HRESULT = 0x{:08X}", hr));

		SET_RESOURCE_DEBUG_NAME(m_device, L"Main ID3D12Device");

#ifdef _DEBUG
		// Выводим достигнутый уровень функций
		std::wstring levelName;
		switch (achievedFeatureLevel)
		{
			case D3D_FEATURE_LEVEL_12_2: levelName = L"12.2 (DirectX 12 Ultimate)"; break;
			case D3D_FEATURE_LEVEL_12_1: levelName = L"12.1"; break;
			case D3D_FEATURE_LEVEL_12_0: levelName = L"12.0"; break;
			default: levelName = L"Unknown"; break;
		}
		DebugOutput(std::format(L">>>>> [DXAPI::InitializePipeline()]. Created D3D12 device with feature level: {}\n", levelName).c_str());
#endif

		CheckDirectX12UltimateSupport();

		m_RtvDescrSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		m_DsvDescrSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
		m_CbvSrvDescrSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		// Настройка и создание CommandQueue.
		D3D12_COMMAND_QUEUE_DESC queueDesc = {};
		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		ensure(S_OK == m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue)));

		// Настройка SwapChain
		m_swapChainFlags = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
		swapChainDesc.BufferCount = FrameCount;
		swapChainDesc.Width = static_cast<UINT>(winSize.width);
		swapChainDesc.Height = static_cast<UINT>(winSize.height);
		swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.Flags = m_swapChainFlags; // Поддержка Variable Refresh Rate

		ComPtr<IDXGISwapChain1> swapChain;
		hr = factory->CreateSwapChainForHwnd(
				m_commandQueue.Get(),	// SwapChain нужна CommandQueue, чтобы она могла принудительно очистить ее.
				dynamic_cast<swMSWin*>(appWin.get())->GetHWND(),
				&swapChainDesc,
				nullptr,
				nullptr,
				&swapChain);
		if (FAILED(hr))
			return Unexpected(eResult::failure, L">>>>> [DXAPI::InitializePipeline()]. Failed to create swap chain. More specifically: " + res.error().getMessage());

		// Этот пример не поддерживает полноэкранные переходы.
		hr = factory->MakeWindowAssociation(dynamic_cast<swMSWin*>(appWin.get())->GetHWND(), DXGI_MWA_NO_ALT_ENTER);
		if (FAILED(hr))
			return Unexpected(eResult::failure, std::format(L">>>>> [DXAPI::InitializePipeline()]. Failed to make window association. HRESULT = 0x{:08X}", hr));

		hr = swapChain.As(&m_swapChain);
		if (FAILED(hr))
			return Unexpected(eResult::failure, std::format(L">>>>> [DXAPI::InitializePipeline()]. Failed to query IDXGISwapChain3. HRESULT = 0x{:08X}", hr));

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
				HRESULT hr = m_swapChain->GetBuffer(n, IID_PPV_ARGS(&m_renderTargets[n]));
				if (FAILED(hr))
					return Unexpected(eResult::failure, std::format(L">>>>> [DXAPI::InitializePipeline()]. Failed to get back buffer. HRESULT = 0x{:08X}", hr));

				m_device->CreateRenderTargetView(m_renderTargets[n].Get(), nullptr, rtvHandle);
				rtvHandle.Offset(1, m_RtvDescrSize);
			}
		}

		res = CreateDS(winSize);
		if (!res)
			return Unexpected(eResult::failure, L">>>>> [DXAPI::InitializePipeline()]. -> " + res.error().getMessage());

		m_commandRender = safe_make_unique<CommandWrapper>(m_device);
		m_commandAccum = safe_make_unique<CommandWrapper>(m_device);

		return {};
	}

	// Метод для проверки поддержки DirectX 12 Ultimate функций
	void DXAPI::CheckDirectX12UltimateSupport()
	{
		if (!m_device)
			return;

		// Проверка поддержки Ray Tracing (DXR)
		D3D12_FEATURE_DATA_D3D12_OPTIONS5 options5 = {};
		if (SUCCEEDED(m_device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &options5, sizeof(options5))))
			m_supportsRayTracing = (options5.RaytracingTier >= D3D12_RAYTRACING_TIER_1_0);

		// Проверка поддержки Variable Rate Shading
		D3D12_FEATURE_DATA_D3D12_OPTIONS6 options6 = {};
		if (SUCCEEDED(m_device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS6, &options6, sizeof(options6))))
			m_supportsVariableRateShading = (options6.VariableShadingRateTier >= D3D12_VARIABLE_SHADING_RATE_TIER_1);

		// Проверка поддержки Mesh Shaders
		D3D12_FEATURE_DATA_D3D12_OPTIONS7 options7 = {};
		if (SUCCEEDED(m_device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS7, &options7, sizeof(options7))))
			m_supportsMeshShaders = (options7.MeshShaderTier >= D3D12_MESH_SHADER_TIER_1);

		// Проверка поддержки Sampler Feedback
		D3D12_FEATURE_DATA_D3D12_OPTIONS7 samplerFeedback = {};
		if (SUCCEEDED(m_device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS7, &samplerFeedback, sizeof(samplerFeedback))))
			m_supportsSamplerFeedback = (samplerFeedback.SamplerFeedbackTier >= D3D12_SAMPLER_FEEDBACK_TIER_0_9);

#ifdef _DEBUG
		DebugOutput(std::format(L">>>>> [DXAPI::CheckDirectX12UltimateSupport()]. DirectX 12 Ultimate Support:\n"
			L" +- Ray Tracing: {}\n"
			L" +- Variable Rate Shading: {}\n"
			L" +- Mesh Shaders: {}\n"
			L" +- Sampler Feedback: {}\n",
			m_supportsRayTracing ? L"Yes" : L"No",
			m_supportsVariableRateShading ? L"Yes" : L"No",
			m_supportsMeshShaders ? L"Yes" : L"No",
			m_supportsSamplerFeedback ? L"Yes" : L"No").c_str());
#endif
	}

	result<> DXAPI::CreateDS(const zSize2D<>& size)
	{
		m_depthStencil.Reset();

		// Описание вида глубины/трафарета
		D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = {};
		depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
		depthStencilDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		depthStencilDesc.Flags = D3D12_DSV_FLAG_NONE;

		// Значение по умолчанию для очистки глубины
		D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
		depthOptimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
		depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
		depthOptimizedClearValue.DepthStencil.Stencil = 0;

		// Свойства кучи для текстуры глубины
		const CD3DX12_HEAP_PROPERTIES depthStencilHeapProps(D3D12_HEAP_TYPE_DEFAULT);

		// Описание ресурса глубины
		const CD3DX12_RESOURCE_DESC depthStencilTextureDesc =
			CD3DX12_RESOURCE_DESC::Tex2D(
				DXGI_FORMAT_D32_FLOAT,
				static_cast<UINT>(size.width),
				static_cast<UINT>(size.height),
				1, // массив из одной текстуры
				0, // mip-уровни
				1, 0,
				D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL
			);

		// Создаём ресурс глубины
		if(S_OK != m_device->CreateCommittedResource(
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
			m_dsvHeap->GetCPUDescriptorHandleForHeapStart() );

		return {};
	}

	result<> DXAPI::CreateRTVHeap()
	{
		// Describe and create a render target view (RTV) descriptor heap.
		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
		rtvHeapDesc.NumDescriptors = FrameCount;
		rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		HRESULT hr = m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap));
		if (FAILED(hr))
			return Unexpected(eResult::failure, std::format(L"Failed to create RTV heap. HRESULT = 0x{:08X}", hr));

		return {};
	}

	result<> DXAPI::CreateSRVHeap()
	{
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

	result<> DXAPI::CreateDSVHeap()
	{
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

	result<> DXAPI::GetAdapter(
		_In_ IDXGIFactory1* pFactory,
		_Outptr_result_maybenull_ IDXGIAdapter1** ppAdapter)
	{
		if (!pFactory || !ppAdapter)
			return Unexpected(eResult::invalid_argument);

		*ppAdapter = nullptr;

		Microsoft::WRL::ComPtr<IDXGIAdapter1> adapter;
		Microsoft::WRL::ComPtr<IDXGIFactory6> factory6;

		auto trySelectAdapter = [&](IDXGIAdapter1* candidate) -> bool
			{
				DXGI_ADAPTER_DESC1 desc{};
				candidate->GetDesc1(&desc);

				if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
					return false;

				if (SUCCEEDED(D3D12CreateDevice(candidate,
					D3D_FEATURE_LEVEL_12_0,
					__uuidof(ID3D12Device),
					nullptr)))
				{
#ifdef _DEBUG
					DebugOutput(std::format(
						L">>>>> [DXAPI::GetAdapter()] Selected adapter: {}\n"
						L" VRAM: {} MB\n",
						desc.Description,
						desc.DedicatedVideoMemory / (1024 * 1024)).c_str());
#endif
					* ppAdapter = candidate;
					(*ppAdapter)->AddRef(); // так как мы не Detach'им
					return true;
				}
				return false;
			};

		// Через IDXGIFactory6 с приоритетом дискретных GPU
		if (SUCCEEDED(pFactory->QueryInterface(IID_PPV_ARGS(&factory6))))
		{
			for (UINT adapterIndex = 0;
				SUCCEEDED(factory6->EnumAdapterByGpuPreference(
					adapterIndex,
					DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
					IID_PPV_ARGS(&adapter)));
					++adapterIndex)
			{
				if (trySelectAdapter(adapter.Get()))
					return {};
			}
		}

		// Обычное перечисление (в случае если Factory6 нет или не найдено подходящих)
		for (UINT adapterIndex = 0;
			SUCCEEDED(pFactory->EnumAdapters1(adapterIndex, &adapter));
			++adapterIndex)
		{
			if (trySelectAdapter(adapter.Get()))
				return {};
		}

		return Unexpected(eResult::fail);
	}

	result<> DXAPI::InitializeAssets()
	{
		// Защита от повторной инициализации
		if (m_fence || m_fenceEvent)
			return Unexpected(eResult::failure, L">>>>> [DXAPI::InitializeAssets()]. Already initialized.");

		// Инициализация root signature
		if (auto res = m_rootSignature.Initialize(m_device); !res)
			return Unexpected(eResult::failure, L">>>>> [DXAPI::InitializeAssets()]. -> " + res.error().getMessage());

		// Создание объектов синхронизации
		{
			ComPtr<ID3D12Fence> fence;
			HRESULT hr = m_device->CreateFence( 0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence) );
			if (FAILED(hr))
				return Unexpected(eResult::failure, std::format(L">>>>> [DXAPI::InitializeAssets()]. CreateFence failed. HRESULT = 0x{:08X}", hr));

			m_fence = fence;
			m_fenceValue = 1;

			// RAII для события
			unique_handle fenceEvent{ CreateEvent(nullptr, FALSE, FALSE, nullptr) };
			if (!fenceEvent)
			{
				hr = HRESULT_FROM_WIN32(GetLastError());
				return Unexpected(eResult::failure,
					std::format(L">>>>> [DXAPI::InitializeAssets()]. CreateEvent failed. HRESULT = 0x{:08X}", hr));
			}

			m_fenceEvent = fenceEvent.release();
		}

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

		ensure(m_device);
		ensure(m_swapChain);

		WaitForPreviousFrame();

		for (UINT i = 0; i < FrameCount; i++)
			m_renderTargets[i].Reset();
		m_rtvHeap.Reset();

		HRESULT hr = m_swapChain->ResizeBuffers(
			FrameCount,
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

		for (UINT i = 0; i < FrameCount; i++)
		{
			Microsoft::WRL::ComPtr<ID3D12Resource> backBuffer;
			if (S_OK != m_swapChain->GetBuffer(i, IID_PPV_ARGS(&backBuffer)))
				throw_runtime_error(std::format(">>>>> [DXAPI::OnResize({}x{})]. Failed to get back buffer {}.", size.width, size.height, i));

			m_device->CreateRenderTargetView(
				backBuffer.Get(),
				nullptr,
				CD3DX12_CPU_DESCRIPTOR_HANDLE(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(),
				i, m_RtvDescrSize));
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
#endif // _WIN64