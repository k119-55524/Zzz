#include "pch.h"
export module DXAPI;

using namespace zzz::platforms::directx;

#if defined(_WIN64)
import IGAPI;
import result;
import winMSWin;
import strConver;
import RootSignature;

using namespace zzz;

export namespace zzz::platforms::directx
{
	// Создана только для обхода ограничений VisualStudio при передаче параметров
	template <class T>
	constexpr inline auto& keep(T&& x) noexcept
	{
		return x;
	}
}

export namespace zzz::platforms
{
	export class DXAPI final : public IGAPI
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
		DXAPI& operator=(DXAPI&&) = delete;

		virtual ~DXAPI() override;

		inline const ComPtr<ID3D12Device> GetDevice() const noexcept { return m_device; };
		inline const ComPtr<ID3D12CommandQueue> GetCommandQueue() const noexcept { return m_commandQueue; };
		inline const ComPtr<IDXGIFactory7> GetFactory() const noexcept { return m_factory; };
		inline const std::shared_ptr<CommandWrapper> GetCommandRender() const noexcept { return m_commandRender; };
		ComPtr<ID3D12RootSignature> GetRootSignature() const noexcept { return m_rootSignature.Get(); }

		void WaitForPreviousFrame() override;
		result<> Initialize(const std::shared_ptr<IAppWin> appWin) override;
		void SetViewport(const D3D12_VIEWPORT& viewport, const D3D12_RECT& scissor);
		void TransitionToRenderTarget(ComPtr<ID3D12Resource> target, ComPtr<ID3D12DescriptorHeap> rtvHeap, UINT rtvDescSize, UINT frameIndex);
		void TransitionToPresent(ComPtr<ID3D12Resource> target);
		void ExecuteCommandList();

	private:
		static constexpr UINT FRAME_COUNT = 2;
		static constexpr DXGI_FORMAT BACK_BUFFER_FORMAT = DXGI_FORMAT_R8G8B8A8_UNORM;
		static constexpr DXGI_FORMAT DEPTH_FORMAT = DXGI_FORMAT_D32_FLOAT;

		UINT m_swapChainFlags;
		float m_aspectRatio;

		RootSignature m_rootSignature;
		CD3DX12_VIEWPORT m_viewport;
		CD3DX12_RECT m_scissorRect;
		D3D_FEATURE_LEVEL m_featureLevel;

		ComPtr<IDXGIFactory7> m_factory;
		ComPtr<IDXGIAdapter1> m_adapter;
		ComPtr<IDXGIAdapter3> m_adapter3;
		ComPtr<ID3D12Device> m_device;
		ComPtr<ID3D12CommandQueue> m_commandQueue;
		ComPtr<IDXGISwapChain3> m_swapChain;
		ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
		ComPtr<ID3D12DescriptorHeap> m_srvHeap;
		ComPtr<ID3D12DescriptorHeap> m_dsvHeap;
		ComPtr<ID3D12Resource> m_renderTargets[FRAME_COUNT];
		ComPtr<ID3D12Resource> m_depthStencil;

		std::mutex commandMutex;
		std::shared_ptr<CommandWrapper> m_commandRender;
		std::shared_ptr<CommandWrapper> m_commandAccum;

		UINT64 m_fenceValue;
		unique_handle m_fenceEvent;
		ComPtr<ID3D12Fence> m_fence;

		result<> InitializeDevice();
		void CheckDirectX12UltimateSupport();
		result<> InitializeFence();
		result<> GetAdapter(_In_ IDXGIFactory1* pFactory, _Outptr_result_maybenull_ IDXGIAdapter1** ppAdapter);
	};

	DXAPI::DXAPI() :
		IGAPI(eGAPIType::DirectX12),
		//m_frameIndex{ 0 },
		m_swapChainFlags{ 0 },
		m_fenceValue{ 0 },
		m_aspectRatio{ 0.0f },
		m_featureLevel{ D3D_FEATURE_LEVEL_12_0 }
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
	}

	result<> DXAPI::Initialize(const std::shared_ptr<IAppWin> appWin)
	{
		std::shared_ptr<winMSWin> appMSWin = std::dynamic_pointer_cast<winMSWin>(appWin);
		if (!appMSWin)
			return Unexpected(eResult::failure, L">>>>> [DXAPI::Initialize()]. Failed to cast IAppWin to winMSWin.");

		result<> res = InitializeDevice()
			.and_then([&]() { return InitializeFence(); });
		if(res)
			initState = eInitState::eInitOK;

		return res;
	}

	result<> DXAPI::InitializeDevice()
	{
		UINT dxgiFactoryFlags = 0;

#if defined(_DEBUG)
		{
			ComPtr<ID3D12Debug> debugController;
			if (S_OK == D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))
			{
				debugController->EnableDebugLayer();
				dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
			}
		}
#endif

		// DXGI Factory
		ComPtr<IDXGIFactory7> factory;
		HRESULT hr = CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory));
		if (FAILED(hr))
		{
			ComPtr<IDXGIFactory4> factory4;
			hr = CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory4));
			if (FAILED(hr))
				return Unexpected(eResult::failure, L">>>>> [DXAPI::InitializeDevice()]. Failed to create DXGI Factory");

			hr = factory4.As(&factory);
			if (FAILED(hr))
				return Unexpected(eResult::failure, L">>>>> [DXAPI::InitializeDevice()]. Failed to query IDXGIFactory7");
		}
		m_factory = factory; // сохраняем для IWinSurface

		// Адаптер
		result<> res = GetAdapter(factory.Get(), &m_adapter);
		if (!res)
			return res;

		hr = m_adapter.As(&m_adapter3);
		if (FAILED(hr))
			return Unexpected(eResult::failure, std::format(L">>>>> [DXAPI::InitializeDevice()]. Failed to query IDXGIAdapter3. HRESULT = 0x{:08X}", hr));

		// Пытаемся создать устройство с максимальным Feature Level
		D3D_FEATURE_LEVEL featureLevels[] =
		{
			D3D_FEATURE_LEVEL_12_2,  // Поддержка DirectX 12 Ultimate
			D3D_FEATURE_LEVEL_12_1,  // Расширенные возможности D3D12
			D3D_FEATURE_LEVEL_12_0   // Базовый D3D12
		};

		for (auto level : featureLevels)
		{
			hr = D3D12CreateDevice(
				m_adapter.Get(),
				level,
				IID_PPV_ARGS(&m_device)
			);

			if (SUCCEEDED(hr))
			{
				m_featureLevel = level;
				break;
			}
		}
		if (FAILED(hr))
			return Unexpected(eResult::failure, std::format(L">>>>> [DXAPI::InitializeDevice()]. Failed to create D3D12 device. HRESULT = 0x{:08X}", hr));

		SET_RESOURCE_DEBUG_NAME(m_device, L"Main ID3D12Device");

#ifdef _DEBUG
		// Выводим достигнутый уровень функций
		std::wstring levelName;
		switch (m_featureLevel)
		{
			case D3D_FEATURE_LEVEL_12_2: levelName = L"12.2 (DirectX 12 Ultimate)"; break;
			case D3D_FEATURE_LEVEL_12_1: levelName = L"12.1"; break;
			case D3D_FEATURE_LEVEL_12_0: levelName = L"12.0"; break;
			default: levelName = L"Unknown"; break;
		}
		DebugOutput(std::format(L">>>>> [DXAPI::InitializePipeline()]. Created D3D12 device with feature level: {}\n", levelName).c_str());
#endif

		CheckDirectX12UltimateSupport();

		// Command Queue
		D3D12_COMMAND_QUEUE_DESC queueDesc = {};
		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		ensure(S_OK == m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue)));

		m_commandRender = safe_make_shared<CommandWrapper>(m_device);
		m_commandAccum = safe_make_shared<CommandWrapper>(m_device);

		return {};
	}

//	result<> DXAPI::InitializePipeline(const std::shared_ptr<winMSWin> appWin)
//	{
//		UINT dxgiFactoryFlags = 0;
//		zSize2D winSize = appWin->GetWinSize();
//		//m_aspectRatio = static_cast<float>(winSize->x) / static_cast<float>(winSize->y);
//		m_viewport = CD3DX12_VIEWPORT{ 0.0f, 0.0f, static_cast<float>(winSize.width), static_cast<float>(winSize.height) };
//		m_scissorRect = CD3DX12_RECT{ 0, 0, static_cast<LONG>(winSize.width), static_cast<LONG>(winSize.height) };
//		m_aspectRatio = static_cast<float>(winSize.width) / static_cast<float>(winSize.height);
//
//#if defined(_DEBUG)
//		// Включаем уровень отладки.
//		{
//			ComPtr<ID3D12Debug> debugController;
//			if (S_OK == D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))
//			{
//				debugController->EnableDebugLayer();
//
//				// Включить дополнительные уровни отладки.
//				dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
//			}
//		}
//#endif
//
//		ComPtr<IDXGIFactory7> factory;
//		HRESULT hr = CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory));
//		if (FAILED(hr))
//		{
//			// Fallback на более старую версию если новая недоступна
//			ComPtr<IDXGIFactory4> factory4;
//			hr = CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory4));
//			if (FAILED(hr))
//				return Unexpected(eResult::failure,
//					std::format(L">>>>> [DXAPI::InitializePipeline()]. Failed to create DXGI Factory. HRESULT = 0x{:08X}", hr));
//
//			hr = factory4.As(&factory);
//			if (FAILED(hr))
//				return Unexpected(eResult::failure,
//					std::format(L">>>>> [DXAPI::InitializePipeline()]. Failed to query IDXGIFactory7. HRESULT = 0x{:08X}", hr));
//		}
//
//		result<> res = GetAdapter(factory.Get(), &m_adapter);
//		if (!res)
//			return Unexpected(eResult::failure, L">>>>> [DXAPI::InitializePipeline()]. Failed to get adapter. More specifically: " + res.error().getMessage());
//
//		hr = m_adapter.As(&m_adapter3);
//		if (FAILED(hr))
//			return Unexpected(eResult::failure, std::format(L">>>>> [DXAPI::InitializePipeline()]. Failed to query IDXGIAdapter3. HRESULT = 0x{:08X}", hr));
//
//		// Пытаемся создать устройство с максимальным Feature Level
//		D3D_FEATURE_LEVEL featureLevels[] = {
//			D3D_FEATURE_LEVEL_12_2,  // Поддержка DirectX 12 Ultimate
//			D3D_FEATURE_LEVEL_12_1,  // Расширенные возможности D3D12
//			D3D_FEATURE_LEVEL_12_0   // Базовый D3D12
//		};
//
//		// Пытаемся создать устройство с максимально возможным уровнем
//		D3D_FEATURE_LEVEL achievedFeatureLevel = D3D_FEATURE_LEVEL_11_0;
//		for (auto level : featureLevels)
//		{
//			hr = D3D12CreateDevice(
//				m_adapter.Get(),
//				level,
//				IID_PPV_ARGS(&m_device)
//			);
//
//			if (SUCCEEDED(hr))
//			{
//				achievedFeatureLevel = level;
//				break;
//			}
//		}
//		if (FAILED(hr))
//			return Unexpected(eResult::failure, std::format(L">>>>> [DXAPI::InitializePipeline()]. Failed to create D3D12 device. HRESULT = 0x{:08X}", hr));
//
//		SET_RESOURCE_DEBUG_NAME(m_device, L"Main ID3D12Device");
//
//#ifdef _DEBUG
//		// Выводим достигнутый уровень функций
//		std::wstring levelName;
//		switch (achievedFeatureLevel)
//		{
//			case D3D_FEATURE_LEVEL_12_2: levelName = L"12.2 (DirectX 12 Ultimate)"; break;
//			case D3D_FEATURE_LEVEL_12_1: levelName = L"12.1"; break;
//			case D3D_FEATURE_LEVEL_12_0: levelName = L"12.0"; break;
//			default: levelName = L"Unknown"; break;
//		}
//		DebugOutput(std::format(L">>>>> [DXAPI::InitializePipeline()]. Created D3D12 device with feature level: {}\n", levelName).c_str());
//#endif
//
//		CheckDirectX12UltimateSupport();
//
//		m_RtvDescrSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
//		m_DsvDescrSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
//		m_CbvSrvDescrSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
//
//		// Настройка и создание CommandQueue.
//		D3D12_COMMAND_QUEUE_DESC queueDesc = {};
//		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
//		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
//		ensure(S_OK == m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue)));
//
//		// Настройка SwapChain
//		m_swapChainFlags = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
//		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
//		swapChainDesc.BufferCount = FRAME_COUNT;
//		swapChainDesc.Width = static_cast<UINT>(winSize.width);
//		swapChainDesc.Height = static_cast<UINT>(winSize.height);
//		swapChainDesc.Format = BACK_BUFFER_FORMAT;
//		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
//		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
//		swapChainDesc.SampleDesc.Count = 1;
//		swapChainDesc.Flags = m_swapChainFlags; // Поддержка Variable Refresh Rate
//
//		ComPtr<IDXGISwapChain1> swapChain;
//		hr = factory->CreateSwapChainForHwnd(
//				m_commandQueue.Get(),	// SwapChain нужна CommandQueue, чтобы она могла принудительно очистить ее.
//				dynamic_cast<winMSWin*>(appWin.get())->GetHWND(),
//				&swapChainDesc,
//				nullptr,
//				nullptr,
//				&swapChain);
//		if (FAILED(hr))
//			return Unexpected(eResult::failure, L">>>>> [DXAPI::InitializePipeline()]. Failed to create swap chain. More specifically: " + res.error().getMessage());
//
//		// Этот пример не поддерживает полноэкранные переходы.
//		hr = factory->MakeWindowAssociation(dynamic_cast<winMSWin*>(appWin.get())->GetHWND(), DXGI_MWA_NO_ALT_ENTER);
//		if (FAILED(hr))
//			return Unexpected(eResult::failure, std::format(L">>>>> [DXAPI::InitializePipeline()]. Failed to make window association. HRESULT = 0x{:08X}", hr));
//
//		hr = swapChain.As(&m_swapChain);
//		if (FAILED(hr))
//			return Unexpected(eResult::failure, std::format(L">>>>> [DXAPI::InitializePipeline()]. Failed to query IDXGISwapChain3. HRESULT = 0x{:08X}", hr));
//
//		m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
//
//		res = CreateRTVHeap()
//			.and_then([&]() { return CreateSRVHeap(); })
//			.and_then([&]() { return CreateDSVHeap(); });
//		if (!res)
//			return Unexpected(eResult::failure, L">>>>> [DXAPI::InitializePipeline()]. -> " + res.error().getMessage());
//
//		// Create frame resources.
//		{
//			CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());
//
//			// Create a RTV for each frame.
//			for (UINT n = 0; n < FRAME_COUNT; n++)
//			{
//				HRESULT hr = m_swapChain->GetBuffer(n, IID_PPV_ARGS(&m_renderTargets[n]));
//				if (FAILED(hr))
//					return Unexpected(eResult::failure, std::format(L">>>>> [DXAPI::InitializePipeline()]. Failed to get back buffer. HRESULT = 0x{:08X}", hr));
//
//				m_device->CreateRenderTargetView(m_renderTargets[n].Get(), nullptr, rtvHandle);
//				rtvHandle.Offset(1, m_RtvDescrSize);
//			}
//		}
//
//		res = CreateDS(winSize);
//		if (!res)
//			return Unexpected(eResult::failure, L">>>>> [DXAPI::InitializePipeline()]. -> " + res.error().getMessage());
//
//		m_commandRender = safe_make_unique<CommandWrapper>(m_device);
//		m_commandAccum = safe_make_unique<CommandWrapper>(m_device);
//
//		return {};
//	}

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

	result<> DXAPI::InitializeFence()
	{
		// Защита от повторной инициализации
		if (m_fence || m_fenceEvent)
			return Unexpected(eResult::failure, L">>>>> [DXAPI::InitializeAssets()]. Already initialized.");

		// Инициализация root signature
		//if (auto res = m_rootSignature.Initialize(m_device); !res)
		//	return Unexpected(eResult::failure, L">>>>> [DXAPI::InitializeAssets()]. -> " + res.error().getMessage());

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

	void DXAPI::SetViewport(const D3D12_VIEWPORT& viewport, const D3D12_RECT& scissor)
	{
		m_commandRender->CommandList()->RSSetViewports(1, &viewport);
		m_commandRender->CommandList()->RSSetScissorRects(1, &scissor);
	}

	void DXAPI::TransitionToRenderTarget(ComPtr<ID3D12Resource> target, ComPtr<ID3D12DescriptorHeap> rtvHeap, UINT rtvDescSize, UINT frameIndex)
	{
		m_commandRender->CommandList()->ResourceBarrier(
			1,
			&keep(
				CD3DX12_RESOURCE_BARRIER::Transition(
					m_renderTargets[frameIndex].Get(),
					D3D12_RESOURCE_STATE_PRESENT,
					D3D12_RESOURCE_STATE_RENDER_TARGET)
			)
		);

		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvHeap->GetCPUDescriptorHandleForHeapStart(), frameIndex, rtvDescSize);
		m_commandRender->CommandList()->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);
	}

	void DXAPI::TransitionToPresent(ComPtr<ID3D12Resource> target)
	{
		m_commandRender->CommandList()->ResourceBarrier(
			1,
			&keep(
				CD3DX12_RESOURCE_BARRIER::Transition(
					target.Get(),
					D3D12_RESOURCE_STATE_RENDER_TARGET,
					D3D12_RESOURCE_STATE_PRESENT)
			)
		);
	}

	void DXAPI::ExecuteCommandList()
	{
		ID3D12CommandList* ppCommandLists[] = { m_commandRender->CommandList().Get() };
		m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
	}
}
#endif // _WIN64