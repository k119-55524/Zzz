
export module DXAPI;

#if defined(ZRENDER_API_D3D12)

import IGAPI;
import Result;
import StrConvert;
import GPUUploadDX;
import AppWin_MSWin;
import RootSignature;
import CommandWrapperDX;
import GPUUploadCallbacks;
import DXDeviceCapabilities;
import ThreadSafeSwapBuffer;

using namespace zzz;
using namespace zzz::core;
using namespace zzz::templates;

export namespace zzz::dx
{
	// Создана только для обхода ограничений VisualStudio при передаче параметров
	template <class T>
	constexpr inline auto& keep(T&& x) noexcept
	{
		return x;
	}

	export class DXAPI final : public IGAPI
	{
		Z_NO_COPY_MOVE(DXAPI);

	public:
		explicit DXAPI(const std::shared_ptr<GAPIConfig> config);
		virtual ~DXAPI() override;

		const ComPtr<ID3D12Device>& GetDevice() const noexcept { return m_device; };
		const ComPtr<ID3D12CommandQueue>& GetCommandQueue() const noexcept { return m_commandQueue; };
		const ComPtr<IDXGIFactory7>& GetFactory() const noexcept { return m_factory; };
		const ComPtr<ID3D12GraphicsCommandList>& GetCommandListUpdate() const noexcept { return m_commandWrapper[m_frameIndexUpdate]->GetCommandList(); };
		const ComPtr<ID3D12GraphicsCommandList>& GetCommandListRender() const noexcept { return m_commandWrapper[m_frameIndexRender]->GetCommandList(); };
		ComPtr<ID3D12RootSignature> GetRootSignature() const noexcept {  return m_rootSignature.Get(); }

		void CommandRenderReset() noexcept;
		[[nodiscard]] Result<> CommandRenderReinitialize();
		void EndPreparedTransfers();

		void SubmitCommandLists() override;
		void BeginRender() override;
		void EndRender() override;

	protected:
		[[nodiscard]] Result<> Init() override;
		void WaitForGpu() override;

	private:
		ThreadSafeSwapBuffer<std::shared_ptr<GPUUploadCB>> m_PreparedTransfers;

		UINT64 m_fenceValue;
		unique_handle m_fenceEvent;
		ComPtr<ID3D12Fence> m_fence;
		UINT m_swapChainFlags;

		RootSignature m_rootSignature;
		D3D_FEATURE_LEVEL m_featureLevel;

		ComPtr<IDXGIFactory7> m_factory;
		ComPtr<IDXGIAdapter3> m_adapter3;
		ComPtr<ID3D12Device> m_device;
		ComPtr<ID3D12CommandQueue> m_commandQueue;
		std::shared_ptr<CommandWrapperDX> m_commandWrapper[BACK_BUFFER_COUNT];

		[[nodiscard]] Result<> InitializeDevice();
		[[nodiscard]] Result<> InitializeFence();
		void EnableDebugLayer(UINT& dxgiFactoryFlags);
		[[nodiscard]] Result<> CreateFactory(UINT dxgiFactoryFlags, ComPtr<IDXGIFactory7>& outFactory);
		[[nodiscard]] Result<> CreateDevice(ComPtr<IDXGIAdapter1> adapter, ComPtr<ID3D12Device>& outDevice, D3D_FEATURE_LEVEL& outFeatureLevel);
		[[nodiscard]] Result<> CreateCommandQueue(ComPtr<ID3D12Device> device, ComPtr<ID3D12CommandQueue>& outQueue);
		[[nodiscard]] Result<> GetAdapter(_In_ IDXGIFactory1* pFactory, _Outptr_result_maybenull_ IDXGIAdapter1** ppAdapter);

		void BeginPreparedTransfers();
	};

	DXAPI::DXAPI(const std::shared_ptr<GAPIConfig> config) :
		IGAPI(config, eGAPIType::DirectX),
		m_fenceValue{ 0 },
		m_swapChainFlags{ 0 },
		m_featureLevel{ D3D_FEATURE_LEVEL_12_0 },
		m_PreparedTransfers{ 100 }
	{
	}

	DXAPI::~DXAPI()
	{
		WaitForGpu();
	}

	void DXAPI::CommandRenderReset() noexcept
	{
		for (int i = 0; i < BACK_BUFFER_COUNT; i++)
			m_commandWrapper[i]->Reset();
	}

	Result<> DXAPI::CommandRenderReinitialize()
	{
		Result<> res;
		for (int i = 0; i < BACK_BUFFER_COUNT; i++)
		{
			res = m_commandWrapper[i]->Reinitialize(m_device);
			if (!res)
				break;
		}

		return res;
	}

#pragma region Initialize
	[[nodiscard]] Result<> DXAPI::Init()
	{
		Result<> res = InitializeDevice()
			.and_then([&]() { m_CPUtoGPUDataTransfer = safe_make_unique<GPUUploadDX>(m_device, m_PreparedTransfers); })
			.and_then([&]() { return m_rootSignature.Initialize(m_device); })
			.and_then([&]() { return InitializeFence(); });

		return res;
	}

	[[nodiscard]] Result<> DXAPI::InitializeDevice()
	{
		UINT dxgiFactoryFlags = 0;
		EnableDebugLayer(dxgiFactoryFlags);

		ComPtr<IDXGIFactory7> factory;
		ComPtr<IDXGIAdapter1> adapter;
		auto res = CreateFactory(dxgiFactoryFlags, factory)
			.and_then([&]() { m_factory = factory; })
			.and_then([&]() { return GetAdapter(factory.Get(), &adapter); });
		if (!res)
			return Unexpected(eResult::failure, std::format(L" -> {}", res.error().getMessage()));

		HRESULT hr = adapter.As(&m_adapter3);
		if (FAILED(hr))
			return Unexpected(eResult::failure, std::format(L"Failed to query IDXGIAdapter3. HRESULT = 0x{:08X}", hr));

		res = CreateDevice(adapter, m_device, m_featureLevel)
			.and_then([&]() { return CreateCommandQueue(m_device, m_commandQueue); });
		if (!res)
			return Unexpected(eResult::failure, std::format(L" -> {}", res.error().getMessage()));

		for (int index = 0; index < BACK_BUFFER_COUNT; index++)
			m_commandWrapper[index] = safe_make_shared<CommandWrapperDX>(m_device, D3D12_COMMAND_LIST_TYPE_DIRECT);

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

			// Даёт дополнительную проверку корректности работы с GPU ресурсами
			// но работает медленнее
			//ComPtr<ID3D12Debug1> debugController1;
			//debugController->QueryInterface(IID_PPV_ARGS(&debugController1));
			//debugController1->SetEnableGPUBasedValidation(true);

			DebugOutput(L"DirectX debug layer enabled.");
		}
#endif
	}

	[[nodiscard]] Result<> DXAPI::CreateFactory(UINT dxgiFactoryFlags, ComPtr<IDXGIFactory7>& outFactory)
	{
		HRESULT hr = CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&outFactory));
		if (FAILED(hr))
		{
			ComPtr<IDXGIFactory4> factory4;
			hr = CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory4));
			if (FAILED(hr))
				return Unexpected(eResult::failure, L"Failed to create DXGI Factory");

			hr = factory4.As(&outFactory);
			if (FAILED(hr))
				return Unexpected(eResult::failure, L"Failed to query IDXGIFactory7");
		}

		return {};
	}

	[[nodiscard]] Result<> DXAPI::CreateDevice(ComPtr<IDXGIAdapter1> adapter, ComPtr<ID3D12Device>& outDevice, D3D_FEATURE_LEVEL& outFeatureLevel)
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
				std::format(L"Failed to create D3D12 device. HRESULT = 0x{:08X}", hr));

		SET_RESOURCE_DEBUG_NAME(outDevice, L"Main ID3D12Device");

#ifdef _DEBUG
		std::wstring levelName = (outFeatureLevel == D3D_FEATURE_LEVEL_12_2) ? L"12.2 (DirectX 12 Ultimate)" :
			(outFeatureLevel == D3D_FEATURE_LEVEL_12_1) ? L"12.1" :
			(outFeatureLevel == D3D_FEATURE_LEVEL_12_0) ? L"12.0" : L"Unknown";
		DebugOutput(std::format(
			L"Created D3D12 device with feature level: {}",
			levelName).c_str());
#endif

		m_CheckGapiSupport = safe_make_unique<DXDeviceCapabilities>(outDevice, m_adapter3);

		return {};
	}

	[[nodiscard]] Result<> DXAPI::CreateCommandQueue(ComPtr<ID3D12Device> device, ComPtr<ID3D12CommandQueue>& outQueue)
	{
		D3D12_COMMAND_QUEUE_DESC queueDesc{};
		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

		HRESULT hr = device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&outQueue));
		if (FAILED(hr))
			return Unexpected(eResult::failure, std::format(L"Failed to create Command Queue. HRESULT = 0x{:08X}", hr));

		return {};
	}

	[[nodiscard]] Result<> DXAPI::GetAdapter(_In_ IDXGIFactory1* pFactory, _Outptr_result_maybenull_ IDXGIAdapter1** ppAdapter)
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
					DebugOutput(std::format( L"Selected adapter: {} VRAM: {} MB", desc.Description, desc.DedicatedVideoMemory / (1024 * 1024)).c_str());

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

		return Unexpected(eResult::failure);
	}

	[[nodiscard]] Result<> DXAPI::InitializeFence()
	{
		// Защита от повторной инициализации
		if (m_fence || m_fenceEvent)
			return Unexpected(eResult::failure, L"Already initialized.");

		// Создание объектов синхронизации
		{
			ComPtr<ID3D12Fence> fence;
			HRESULT hr = m_device->CreateFence( 0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence) );
			if (FAILED(hr))
				return Unexpected(eResult::failure, std::format(L"CreateFence failed. HRESULT = 0x{:08X}", hr));

			m_fence = fence;

			// RAII для события
			unique_handle fenceEvent{ CreateEvent(nullptr, FALSE, FALSE, nullptr) };
			if (!fenceEvent)
			{
				hr = HRESULT_FROM_WIN32(GetLastError());
				return Unexpected(eResult::failure, std::format(L"CreateEvent failed. HRESULT = 0x{:08X}", hr));
			}

			m_fenceEvent = fenceEvent.release();
		}

		return {};
	}
#pragma endregion Initialize

#pragma region Rendring
	void DXAPI::BeginRender()
	{
		auto commandAllocator = m_commandWrapper[m_frameIndexUpdate]->GetCommandAllocator();
		auto commandList = m_commandWrapper[m_frameIndexUpdate]->GetCommandList();

		// Сбрасываем аллокатор и командный список
		ensure(S_OK == commandAllocator->Reset());
		ensure(S_OK == commandList->Reset(commandAllocator.Get(), nullptr));

		// Устанавливаем состояние ресурсов как готовых к рендрингу после копирования в память GPU
		BeginPreparedTransfers();
	}

	void DXAPI::EndRender()
	{
		// Закрываем командный список
		auto commandListUpdate = m_commandWrapper[m_frameIndexUpdate]->GetCommandList();
		ensure(S_OK == commandListUpdate->Close());

		m_frameIndexRender = (m_frameIndexRender + 1) % BACK_BUFFER_COUNT;
		m_frameIndexUpdate = (m_frameIndexRender + 1) % BACK_BUFFER_COUNT;

		// Устанавливаем состояние ресурсов как готовых к рендрингу после копирования в память GPU
		EndPreparedTransfers();

		WaitForGpu();
	}

	// Устанавливаем состояние ресурса готовое к рендрингу после копирования в память GPU
	void DXAPI::BeginPreparedTransfers()
	{
		auto commandList = m_commandWrapper[m_frameIndexUpdate]->GetCommandList();

		m_PreparedTransfers.ForEach([&](std::shared_ptr<GPUUploadCB> callback)
			{
				if (callback->Success)
					callback->OnPrepared(commandList);
			});
	}

	void DXAPI::EndPreparedTransfers()
	{
		m_PreparedTransfers.ForEach([&](std::shared_ptr<GPUUploadCB> callback)
			{
				if (callback->Success)
					callback->OnComplete(callback->Success);
			});

		m_PreparedTransfers.SwapAndReset();
	}

	void DXAPI::SubmitCommandLists()
	{
		//DebugOutput(std::format(L"m_frameIndexRender: {}.", m_frameIndexRender));

		ID3D12CommandList* ppCommandLists[] = { m_commandWrapper[m_frameIndexRender]->GetCommandList().Get()};
		m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
	}

	void DXAPI::WaitForGpu()
	{
		const UINT64 fence = m_fenceValue;
		ensure(S_OK == m_commandQueue->Signal(m_fence.Get(), fence));

		m_fenceValue++;
		if (m_fence->GetCompletedValue() < fence)
		{
			ensure(S_OK == m_fence->SetEventOnCompletion(fence, m_fenceEvent.handle));

			WaitForSingleObject(m_fenceEvent.handle, INFINITE);
		}
	}
#pragma endregion Rendring
}
#endif // defined(ZRENDER_API_D3D12)