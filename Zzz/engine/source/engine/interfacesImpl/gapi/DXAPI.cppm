#include "pch.h"
export module DXAPI;

using namespace zzz::platforms::directx;

#if defined(_WIN64)
import IGAPI;
import result;
import winMSWin;
import strConvert;
import RootSignature;
import CheckDirectXSupport;

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

export namespace zzz::platforms::directx
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

		void ExecuteCommandList();

	protected:
		result<> Initialize();
		void WaitForPreviousFrame() override;

	private:
		static constexpr UINT FRAME_COUNT = 2;
		static constexpr DXGI_FORMAT BACK_BUFFER_FORMAT = DXGI_FORMAT_R8G8B8A8_UNORM;
		static constexpr DXGI_FORMAT DEPTH_FORMAT = DXGI_FORMAT_D32_FLOAT;

		UINT m_swapChainFlags;

		RootSignature m_rootSignature;
		D3D_FEATURE_LEVEL m_featureLevel;

		ComPtr<IDXGIFactory7> m_factory;
		ComPtr<IDXGIAdapter3> m_adapter3;
		ComPtr<ID3D12Device> m_device;
		ComPtr<ID3D12CommandQueue> m_commandQueue;

		std::shared_ptr<CommandWrapper> m_commandRender;
		std::shared_ptr<CommandWrapper> m_commandAccum;

		UINT64 m_fenceValue;
		unique_handle m_fenceEvent;
		ComPtr<ID3D12Fence> m_fence;

		result<> InitializeDevice();
		result<> InitializeFence();
		void EnableDebugLayer(UINT& dxgiFactoryFlags);
		result<> CreateFactory(UINT dxgiFactoryFlags, ComPtr<IDXGIFactory7>& outFactory);
		result<> CreateDevice(ComPtr<IDXGIAdapter1> adapter, ComPtr<ID3D12Device>& outDevice, D3D_FEATURE_LEVEL& outFeatureLevel);
		result<> CreateCommandQueue(ComPtr<ID3D12Device> device, ComPtr<ID3D12CommandQueue>& outQueue);
		result<> GetAdapter(_In_ IDXGIFactory1* pFactory, _Outptr_result_maybenull_ IDXGIAdapter1** ppAdapter);
	};

	DXAPI::DXAPI() :
		IGAPI(eGAPIType::DirectX),
		m_swapChainFlags{ 0 },
		m_fenceValue{ 0 },
		m_featureLevel{ D3D_FEATURE_LEVEL_12_0 }
	{
	}

	DXAPI::~DXAPI()
	{
		WaitForPreviousFrame();
	}

#pragma region Rendring
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

	void DXAPI::ExecuteCommandList()
	{
		ID3D12CommandList* ppCommandLists[] = { m_commandRender->CommandList().Get() };
		m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
	}
#pragma endregion Rendring

#pragma region Initialize
	result<> DXAPI::Initialize()
	{
		result<> res = InitializeDevice()
			.and_then([&]() { return InitializeFence(); })
			.and_then([&]() { initState = eInitState::eInitOK; });

		return res;
	}

	result<> DXAPI::InitializeDevice()
	{
		UINT dxgiFactoryFlags = 0;
		EnableDebugLayer(dxgiFactoryFlags);

		ComPtr<IDXGIFactory7> factory;
		ComPtr<IDXGIAdapter1> adapter;
		auto res = CreateFactory(dxgiFactoryFlags, factory)
			.and_then([&]() { m_factory = factory; })
			.and_then([&]() { return GetAdapter(factory.Get(), &adapter); });
		if (!res)
			return Unexpected(eResult::failure, std::format(L">>>>> [DXAPI::InitializeDevice()]. -> {}", res.error().getMessage()));

		HRESULT hr = adapter.As(&m_adapter3);
		if (FAILED(hr))
			return Unexpected(eResult::failure, std::format(L">>>>> [DXAPI::InitializeDevice()]. Failed to query IDXGIAdapter3. HRESULT = 0x{:08X}", hr));

		res = CreateDevice(adapter, m_device, m_featureLevel)
			.and_then([&]() { return CreateCommandQueue(m_device, m_commandQueue); });
		if (!res)
			return Unexpected(eResult::failure, std::format(L">>>>> [DXAPI::InitializeDevice()]. -> {}", res.error().getMessage()));

		// Создаём обёртки для командных списков
		m_commandRender = safe_make_shared<CommandWrapper>(m_device);
		m_commandAccum = safe_make_shared<CommandWrapper>(m_device);

		return {};
	}

	void DXAPI::EnableDebugLayer(UINT& dxgiFactoryFlags)
	{
#if defined(_DEBUG)
		ComPtr<ID3D12Debug> debugController;
		if (S_OK == D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))
		{
			debugController->EnableDebugLayer();
			dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;

			DebugOutput(L">>>>> [DXAPI::EnableDebugLayer()]. DirectX debug layer enabled.\n");
		}
#endif
	}

	result<> DXAPI::CreateFactory(UINT dxgiFactoryFlags, ComPtr<IDXGIFactory7>& outFactory)
	{
		HRESULT hr = CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&outFactory));
		if (FAILED(hr))
		{
			ComPtr<IDXGIFactory4> factory4;
			hr = CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory4));
			if (FAILED(hr))
				return Unexpected(eResult::failure, L">>>>> [DXAPI::InitializeDevice()]. Failed to create DXGI Factory");

			hr = factory4.As(&outFactory);
			if (FAILED(hr))
				return Unexpected(eResult::failure, L">>>>> [DXAPI::InitializeDevice()]. Failed to query IDXGIFactory7");
		}

		return {};
	}

	result<> DXAPI::CreateDevice(ComPtr<IDXGIAdapter1> adapter, ComPtr<ID3D12Device>& outDevice, D3D_FEATURE_LEVEL& outFeatureLevel)
	{
		static constexpr D3D_FEATURE_LEVEL levels[] =
		{
			D3D_FEATURE_LEVEL_12_2,
			D3D_FEATURE_LEVEL_12_1,
			D3D_FEATURE_LEVEL_12_0
		};

		HRESULT hr = E_FAIL;
		for (auto level : levels)
		{
			hr = D3D12CreateDevice(adapter.Get(), level, IID_PPV_ARGS(&outDevice));
			if (SUCCEEDED(hr))
			{
				outFeatureLevel = level;
				break;
			}
		}

		if (FAILED(hr))
			return Unexpected(eResult::failure,
				std::format(L">>>>> [DXAPI::InitializeDevice()]. Failed to create D3D12 device. HRESULT = 0x{:08X}", hr));

		SET_RESOURCE_DEBUG_NAME(outDevice, L"Main ID3D12Device");

#ifdef _DEBUG
		std::wstring levelName = (outFeatureLevel == D3D_FEATURE_LEVEL_12_2) ? L"12.2 (DirectX 12 Ultimate)" :
			(outFeatureLevel == D3D_FEATURE_LEVEL_12_1) ? L"12.1" :
			(outFeatureLevel == D3D_FEATURE_LEVEL_12_0) ? L"12.0" : L"Unknown";
		DebugOutput(std::format(
			L">>>>> [DXAPI::InitializePipeline()]. Created D3D12 device with feature level: {}\n",
			levelName).c_str());
#endif

		checkGapiSupport = safe_make_unique<CheckDirectXSupport>(outDevice);

		return {};
	}

	result<> DXAPI::CreateCommandQueue(ComPtr<ID3D12Device> device, ComPtr<ID3D12CommandQueue>& outQueue)
	{
		D3D12_COMMAND_QUEUE_DESC queueDesc{};
		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

		HRESULT hr = device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&outQueue));
		if (FAILED(hr))
			return Unexpected(eResult::failure, std::format(L">>>>> [DXAPI::InitializeDevice()]. Failed to create Command Queue. HRESULT = 0x{:08X}", hr));

		return {};
	}

	result<> DXAPI::GetAdapter(_In_ IDXGIFactory1* pFactory, _Outptr_result_maybenull_ IDXGIAdapter1** ppAdapter)
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
#pragma endregion Initialize
}
#endif // _WIN64