
export module DXAPI;

#if defined(ZRENDER_API_D3D12)

import IGAPI;
import Result;
import Ensure;
import StrConvert;
import GPUUploadDX;
import AppWin_MSWin;
import RootSignature;
import EngineConstants;
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
		~DXAPI() override;

		const ComPtr<ID3D12Device>& GetDevice() const noexcept { return m_device; };
		const ComPtr<ID3D12CommandQueue>& GetCommandQueue() const noexcept { return m_commandQueue; };
		const ComPtr<IDXGIFactory7>& GetFactory() const noexcept { return m_factory; };
		const ComPtr<ID3D12GraphicsCommandList>& GetCommandListUpdate() const noexcept { return m_commandWrapper[m_IndexFrameUpdate]->GetCommandList(); };
		const ComPtr<ID3D12GraphicsCommandList>& GetCommandListRender() const noexcept { return m_commandWrapper[m_IndexFrameRender]->GetCommandList(); };
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

		[[nodiscard]] Result<> EnableDebugLayer(UINT& dxgiFactoryFlags);
		[[nodiscard]] Result<> InitializeDevice(UINT dxgiFactoryFlags);
		[[nodiscard]] Result<> InitializeFence();
		[[nodiscard]] Result<> CreateFactory(UINT dxgiFactoryFlags, ComPtr<IDXGIFactory7>& outFactory);
		[[nodiscard]] Result<> CreateDevice(ComPtr<IDXGIAdapter1> adapter, ComPtr<ID3D12Device>& outDevice, D3D_FEATURE_LEVEL& outFeatureLevel);
		[[nodiscard]] Result<> CreateCommandQueue(ComPtr<ID3D12Device> device, ComPtr<ID3D12CommandQueue>& outQueue);
		[[nodiscard]] Result<> GetAdapter(_In_ IDXGIFactory1* pFactory, _Outptr_result_maybenull_ IDXGIAdapter1** ppAdapter);

		void BeginPreparedTransfers();

		[[nodiscard]] Result<> CreateDebugMessenger();
		void ReportGPUDebugMessages();
#if defined(_DEBUG)
		ComPtr<ID3D12InfoQueue> m_infoQueue;
#endif

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

#pragma region Helpers
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
#pragma endregion

#pragma region Initialize
	[[nodiscard]] Result<> DXAPI::Init()
	{
		UINT dxgiFactoryFlags = 0;
		Result<> res = EnableDebugLayer(dxgiFactoryFlags)
			.and_then([&]() { return InitializeDevice(dxgiFactoryFlags); })
			.and_then([&]() { return CreateCommandQueue(m_device, m_commandQueue); })
			.and_then([&]() { return CreateDebugMessenger(); })
			.and_then([&]() { m_CPUtoGPUDataTransfer = safe_make_unique<GPUUploadDX>(m_device, m_PreparedTransfers); })
			.and_then([&]() { return m_rootSignature.Initialize(m_device); })
			.and_then([&]() { return InitializeFence(); });

		return res;
	}

	[[nodiscard]] Result<> DXAPI::InitializeDevice(UINT dxgiFactoryFlags)
	{
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

		// Проверка возможности отключения VSYNC
		BOOL allowTearing = FALSE;
		hr = factory->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof(allowTearing));
		if (SUCCEEDED(hr))
			b_IsCanDisableVsync = allowTearing;

		return {};
	}

	[[nodiscard]] Result<> DXAPI::CreateDebugMessenger()
	{
#if defined(_DEBUG)
		if (SUCCEEDED(m_device.As(&m_infoQueue)))
		{
			// Включаем break-on-severity, если нужно
			m_infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
			m_infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
			m_infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, FALSE);

			// Фильтр для ненужных сообщений (по желанию)
			D3D12_MESSAGE_ID hide[] =
			{
				D3D12_MESSAGE_ID_SETPRIVATEDATA_CHANGINGPARAMS
			};
			D3D12_INFO_QUEUE_FILTER filter{};
			filter.DenyList.NumIDs = _countof(hide);
			filter.DenyList.pIDList = hide;
			m_infoQueue->AddStorageFilterEntries(&filter);

			DebugOutput(L"D3D12 InfoQueue enabled.");
		}
		else
			return Unexpected(eResult::failure, L"Failed to get ID3D12InfoQueue for debug messaging.");
#endif

		return {};
	}

	void DXAPI::ReportGPUDebugMessages()
	{
#if defined(_DEBUG)
		if (!m_infoQueue)
			return;

		const UINT64 numMessages = m_infoQueue->GetNumStoredMessages();
		for (UINT64 i = 0; i < numMessages; ++i)
		{
			SIZE_T messageLength = 0;
			m_infoQueue->GetMessage(i, nullptr, &messageLength);

			std::vector<char> buffer(messageLength);
			D3D12_MESSAGE* msg = reinterpret_cast<D3D12_MESSAGE*>(buffer.data());

			m_infoQueue->GetMessage(i, msg, &messageLength);

			// severity
			std::wstring severityStr;
			switch (msg->Severity)
			{
			case D3D12_MESSAGE_SEVERITY_CORRUPTION:	severityStr = L"CORRUPTION"; break;
			case D3D12_MESSAGE_SEVERITY_ERROR:		severityStr = L"ERROR"; break;
			case D3D12_MESSAGE_SEVERITY_WARNING:	severityStr = L"WARNING"; break;
			case D3D12_MESSAGE_SEVERITY_INFO:		severityStr = L"INFO"; break;
			case D3D12_MESSAGE_SEVERITY_MESSAGE:	severityStr = L"MESSAGE"; break;
			default:								severityStr = L"UNKNOWN"; break;
			}

			// category
			std::wstring categoryStr;
			switch (msg->Category)
			{
			case D3D12_MESSAGE_CATEGORY_APPLICATION_DEFINED:	categoryStr = L"APPLICATION_DEFINED"; break;
			case D3D12_MESSAGE_CATEGORY_MISCELLANEOUS:			categoryStr = L"MISCELLANEOUS"; break;
			case D3D12_MESSAGE_CATEGORY_INITIALIZATION:			categoryStr = L"INITIALIZATION"; break;
			case D3D12_MESSAGE_CATEGORY_CLEANUP:				categoryStr = L"CLEANUP"; break;
			case D3D12_MESSAGE_CATEGORY_COMPILATION:			categoryStr = L"COMPILATION"; break;
			case D3D12_MESSAGE_CATEGORY_STATE_CREATION:			categoryStr = L"STATE_CREATION"; break;
			case D3D12_MESSAGE_CATEGORY_STATE_SETTING:			categoryStr = L"STATE_SETTING"; break;
			case D3D12_MESSAGE_CATEGORY_RESOURCE_MANIPULATION:	categoryStr = L"RESOURCE_MANIPULATION"; break;
			case D3D12_MESSAGE_CATEGORY_EXECUTION:				categoryStr = L"EXECUTION"; break;
			case D3D12_MESSAGE_CATEGORY_SHADER:					categoryStr = L"SHADER"; break;
			default:											categoryStr = L"UNKNOWN"; break;
			}

			std::wstring msgStr(msg->pDescription, msg->pDescription + strlen(msg->pDescription));
			LogGPUDebugMessage(std::format(L"[DX12 Debug] {} | {}: {}", severityStr, categoryStr, msgStr));
		}

		m_infoQueue->ClearStoredMessages();
#endif
	}

	[[nodiscard]] Result<> DXAPI::EnableDebugLayer(UINT& dxgiFactoryFlags)
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
		else
			return Unexpected(eResult::failure, L"Failed to enable DirectX debug layer.");
#endif

		return {};
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

#if  defined(_DEBUG)
		std::wstring levelName = (outFeatureLevel == D3D_FEATURE_LEVEL_12_2) ? L"12.2 (DirectX 12 Ultimate)" :
			(outFeatureLevel == D3D_FEATURE_LEVEL_12_1) ? L"12.1" :
			(outFeatureLevel == D3D_FEATURE_LEVEL_12_0) ? L"12.0" : L"Unknown";
		DebugOutput(std::format(L"Created D3D12 device with feature level: {}", levelName).c_str());
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

		for (int index = 0; index < BACK_BUFFER_COUNT; index++)
			m_commandWrapper[index] = safe_make_shared<CommandWrapperDX>(m_device, D3D12_COMMAND_LIST_TYPE_DIRECT);

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
		auto commandAllocator = m_commandWrapper[m_IndexFrameUpdate]->GetCommandAllocator();
		auto commandList = m_commandWrapper[m_IndexFrameUpdate]->GetCommandList();

		// Сбрасываем аллокатор и командный список
		ensure(S_OK == commandAllocator->Reset());
		ensure(S_OK == commandList->Reset(commandAllocator.Get(), nullptr));

		// Устанавливаем состояние ресурсов как готовых к рендрингу после копирования в память GPU
		BeginPreparedTransfers();
	}

	void DXAPI::EndRender()
	{
		ReportGPUDebugMessages();

		// Закрываем командный список
		auto commandListUpdate = m_commandWrapper[m_IndexFrameUpdate]->GetCommandList();
		ensure(S_OK == commandListUpdate->Close());

		// Устанавливаем состояние ресурсов как готовых к рендрингу после копирования в память GPU
		EndPreparedTransfers();

		WaitForGpu();
		IGAPI::EndRender();
	}

	// Устанавливаем состояние ресурса готовое к рендрингу после копирования в память GPU
	void DXAPI::BeginPreparedTransfers()
	{
		auto commandList = m_commandWrapper[m_IndexFrameUpdate]->GetCommandList();

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
		//DebugOutput(std::format(L"m_IndexFrameRender: {}.", m_IndexFrameRender));

		ID3D12CommandList* ppCommandLists[] = { m_commandWrapper[m_IndexFrameRender]->GetCommandList().Get()};
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