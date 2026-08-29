
#if defined(Z_D3D12)
#include "engine/utils/MonitorUtils.h"
#include "core/utils/ToStringHelpers.h"
#include "engine/gapi/directx12/DirectX12API.h"
#include "engine/gapi/selectors/monitor/MonitorSelector.h"
#include "engine/gapi/selectors/gpu/directx12/DirectX12GpuSelector.h"

namespace zzz::engine
{
	namespace
	{
		// eLogMessageType::Message покрывает и Info, и Message(Verbose) DX12 - GAPIDebugLogger::Report сам
		// разводит их по категориям (LogGAPI/LogGAPIVerbose, см. GAPIDebugLogger.cpp). Отдельная категория
		// Validation/General (D3D12_MESSAGE_CATEGORY) больше не нужна - LogGAPIPerformance убрана из
		// финального плана категорий, см. core/utils/LogCategory.h.
		eLogMessageType MapDX12Severity(D3D12_MESSAGE_SEVERITY severity)
		{
			switch (severity)
			{
			case D3D12_MESSAGE_SEVERITY_CORRUPTION:
			case D3D12_MESSAGE_SEVERITY_ERROR:
				return eLogMessageType::Error;
			case D3D12_MESSAGE_SEVERITY_WARNING:
				return eLogMessageType::Warning;
			case D3D12_MESSAGE_SEVERITY_INFO:
			case D3D12_MESSAGE_SEVERITY_MESSAGE:
			default:
				return eLogMessageType::Message;
			}
		}

		// Колбэк ID3D12InfoQueue1::RegisterMessageCallback - сюда попадают все сообщения DX12 debug layer
		// (аналог pfnUserCallback у Vulkan), без фильтрации по severity/категории. Решение "показывать ли
		// Verbose-уровень" здесь НЕ принимается - это делает GAPIDebugLogger::Report централизованно,
		// одинаково для всех бэкендов (см. её комментарий в GAPIDebugLogger.h).
		void __stdcall DX12DebugMessageCallback(D3D12_MESSAGE_CATEGORY category, D3D12_MESSAGE_SEVERITY severity, D3D12_MESSAGE_ID id, LPCSTR description, void* context)
		{
			(void)category;
			(void)id;
			(void)context;

			GAPIDebugLogger::Report(eGAPIType::DirectX12, MapDX12Severity(severity), description ? description : "");
		}

		// У DX12 нет аналога VK_EXT_layer_settings/report_flags (у Vulkan это позволяет слою самому
		// решать, репортить ли сообщение, если оно не error/warn/perf - см. VulkanAPI::BuildVerboseValidationLayerSettings).
		// Ближайший DX12-аналог - ID3D12InfoQueue::AddStorageFilterEntries с D3D12_INFO_QUEUE_FILTER:
		// он глушит сообщения по категории/severity/id ещё до попадания в очередь InfoQueue. Сейчас не
		// используется - мы и так получаем ВСЕ сообщения через RegisterMessageCallback (push, не через
		// очередь GetMessage), а фильтрация по Verbose-уровню теперь единая для всех бэкендов и живёт в
		// GAPIDebugLogger::Report, а не здесь. Полный пример, если понадобится более тонкая
		// фильтрация по категориям (например заглушить болтливые STATE_SETTING/STATE_GETTING):
		//
		// void ApplyStorageFilter(ID3D12InfoQueue1* infoQueue)
		// {
		//     D3D12_MESSAGE_SEVERITY denySeverities[] = { D3D12_MESSAGE_SEVERITY_MESSAGE };
		//     D3D12_MESSAGE_CATEGORY denyCategories[] =
		//     {
		//         D3D12_MESSAGE_CATEGORY_STATE_CREATION,
		//         D3D12_MESSAGE_CATEGORY_STATE_SETTING,
		//         D3D12_MESSAGE_CATEGORY_STATE_GETTING,
		//     };
		//
		//     D3D12_INFO_QUEUE_FILTER filter{};
		//     filter.DenyList.NumSeverities = _countof(denySeverities);
		//     filter.DenyList.pSeverityList = denySeverities;
		//     filter.DenyList.NumCategories = _countof(denyCategories);
		//     filter.DenyList.pCategoryList = denyCategories;
		//
		//     infoQueue->AddStorageFilterEntries(&filter);
		// }
		//
		// AddStorageFilterEntries относится к очереди сообщений InfoQueue (GetMessage/GetNumStoredMessages) -
		// на RegisterMessageCallback он не влияет, поэтому нам он и не нужен при текущем push-подходе.
	}

	DirectX12API::~DirectX12API()
	{
#if Z_DEBUG_BUILD || Z_DEVELOPMENT_BUILD
		if (m_InfoQueue && m_InfoQueueCookie != 0)
		{
			m_InfoQueue->UnregisterMessageCallback(m_InfoQueueCookie);
			m_InfoQueueCookie = 0;
		}
#endif

		if (m_FenceEvent)
		{
			CloseHandle(m_FenceEvent);
			m_FenceEvent = nullptr;
		}
	}

#pragma region Initialize
	void DirectX12API::Initialize(std::shared_ptr<UserSettingsManager> userSettings)
	{
		UINT dxgiFactoryFlags = 0;
		EnableDebugLayer(dxgiFactoryFlags);
		InitializeDevice(userSettings, dxgiFactoryFlags);
		SelectMonitor(m_Adapter1.Get(), userSettings);
	}

	void DirectX12API::EnableDebugLayer(UINT& dxgiFactoryFlags)
	{
#if Z_DEBUG_BUILD || Z_DEVELOPMENT_BUILD
		Microsoft::WRL::ComPtr<ID3D12Debug> debugController;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
		{
			debugController->EnableDebugLayer();
			dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;

#if Z_GAPI_VERBOSE_DEBUG_LAYER
			Microsoft::WRL::ComPtr<ID3D12Debug1> debugController1;
			if (SUCCEEDED(debugController.As(&debugController1)))
			{
				debugController1->SetEnableGPUBasedValidation(TRUE);
				debugController1->SetEnableSynchronizedCommandQueueValidation(TRUE);
			}
#endif

			DOut("[DirectX12API::EnableDebugLayer] - DirectX debug layer enabled.");
		}
		else
			THROW_RUNTIME("Failed to enable DirectX debug layer.");
#endif
	}

	void DirectX12API::InitializeDevice(std::shared_ptr<UserSettingsManager> userSettings, UINT dxgiFactoryFlags)
	{
		m_Factory = CreateFactory(dxgiFactoryFlags);

		m_Adapter1 = GetAdapter(m_Factory.Get(), userSettings);

		HRESULT hr = m_Adapter1.As(&m_Adapter3);
		if (FAILED(hr))
			THROW_RUNTIME("Failed to query IDXGIAdapter3. HRESULT = 0x{:08X}", static_cast<unsigned int>(hr));

		CreateDevice(m_Adapter1.Get());

		// Проверка поддержки отмены VSYNC (Allow Tearing)
		BOOL allowTearing = FALSE;
		hr = m_Factory->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof(allowTearing));
		if (SUCCEEDED(hr))
			m_IsCanDisableVSync = allowTearing;
	}

	Microsoft::WRL::ComPtr<IDXGIFactory7> DirectX12API::CreateFactory(UINT dxgiFactoryFlags)
	{
		Microsoft::WRL::ComPtr<IDXGIFactory7> outFactory;
		HRESULT hr = CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&outFactory));
		if (FAILED(hr))
		{
			Microsoft::WRL::ComPtr<IDXGIFactory4> factory4;
			hr = CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory4));
			if (FAILED(hr))
				THROW_RUNTIME("Failed to create DXGI Factory");

			hr = factory4.As(&outFactory);
			if (FAILED(hr))
				THROW_RUNTIME("Failed to query IDXGIFactory7");
		}
		return outFactory;
	}

	Microsoft::WRL::ComPtr<IDXGIAdapter1> DirectX12API::GetAdapter(IDXGIFactory1* pFactory, const std::shared_ptr<UserSettingsManager>& userSettings)
	{
		if (!pFactory)
			THROW_RUNTIME("Invalid DXGI Factory argument");

		DirectX12GpuSelector selector(userSettings);

		Microsoft::WRL::ComPtr<IDXGIAdapter1> adapter;
		Microsoft::WRL::ComPtr<IDXGIFactory6> factory6;

		// 1. Сначала опрашиваем через IDXGIFactory6 (с приоритетом дискретных GPU)
		if (SUCCEEDED(pFactory->QueryInterface(IID_PPV_ARGS(&factory6))))
		{
			for (UINT adapterIndex = 0; SUCCEEDED(factory6->EnumAdapterByGpuPreference(adapterIndex, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&adapter))); ++adapterIndex)
			{
				selector.AddCandidate(adapter);
			}
		}

		// 2. Дозаполняем оставшиеся графические адаптеры через обычное перечисление (встройка и т.д.)
		for (UINT adapterIndex = 0; SUCCEEDED(pFactory->EnumAdapters1(adapterIndex, &adapter)); ++adapterIndex)
		{
			selector.AddCandidate(adapter);
		}

		// Выбор лучшего или ранее сохраненного адаптера через селектор
		return selector.SelectAdapter();
	}

	void DirectX12API::CreateDevice(IDXGIAdapter1* adapter)
	{
		if (!adapter)
			THROW_RUNTIME("Invalid DXGI Adapter argument");

		static constexpr D3D_FEATURE_LEVEL levels[] =
		{
			D3D_FEATURE_LEVEL_12_2,
			D3D_FEATURE_LEVEL_12_1,
			D3D_FEATURE_LEVEL_12_0
		};

		HRESULT hr = E_FAIL;
		for (auto level : levels)
		{
			hr = D3D12CreateDevice(adapter, level, IID_PPV_ARGS(m_Device.ReleaseAndGetAddressOf()));
			if (SUCCEEDED(hr))
			{
				m_FeatureLevel = level;
				break;
			}
		}

		if (FAILED(hr))
			THROW_RUNTIME("Failed to create D3D12 device. HRESULT = 0x{:08X}", static_cast<unsigned int>(hr));

		SetDebugName(m_Device, "MainDevice");
		RegisterDebugMessageCallback();

		D3D12_COMMAND_QUEUE_DESC queueDesc{};
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

		hr = m_Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(m_CommandQueue.ReleaseAndGetAddressOf()));
		if (FAILED(hr))
			THROW_RUNTIME("Failed to create D3D12 Direct Command Queue. HRESULT = 0x{:08X}", static_cast<unsigned int>(hr));

		SetDebugName(m_CommandQueue, "MainCommandQueue");

		hr = m_Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_Fence.ReleaseAndGetAddressOf()));
		if (FAILED(hr))
			THROW_RUNTIME("Failed to create D3D12 Fence. HRESULT = 0x{:08X}", static_cast<unsigned int>(hr));

		SetDebugName(m_Fence, "MainFence");

		m_FenceValue = 0;
		m_FenceEvent = CreateEventEx(nullptr, FALSE, FALSE, EVENT_ALL_ACCESS);
		if (!m_FenceEvent)
			THROW_RUNTIME("Failed to create D3D12 Fence Event.");

#if defined(Z_DEBUG_BUILD)
		DOut("[DirectX12API::CreateDevice] - Created D3D12 device with feature level: {}", m_FeatureLevel);
#endif
	}

	void DirectX12API::RegisterDebugMessageCallback()
	{
#if Z_DEBUG_BUILD || Z_DEVELOPMENT_BUILD
		if (!m_Device)
			return;

		if (FAILED(m_Device.As(&m_InfoQueue)))
		{
			DOutWarning("[DirectX12API::RegisterDebugMessageCallback] - ID3D12InfoQueue1 is not available, DX12 debug layer messages will not be routed to the engine logger.");
			return;
		}

		HRESULT hr = m_InfoQueue->RegisterMessageCallback(&DX12DebugMessageCallback, D3D12_MESSAGE_CALLBACK_FLAG_NONE, nullptr, &m_InfoQueueCookie);
		if (FAILED(hr))
		{
			DOutWarning("[DirectX12API::RegisterDebugMessageCallback] - Failed to register DX12 debug message callback. HRESULT = 0x{:08X}", static_cast<unsigned int>(hr));
			m_InfoQueue.Reset();
		}
#endif
	}

	void DirectX12API::SelectMonitor(IDXGIAdapter1* adapter, const std::shared_ptr<UserSettingsManager>& userSettings)
	{
		if (!adapter || !userSettings)
			return;

		MonitorSelector selector(userSettings);

		Microsoft::WRL::ComPtr<IDXGIOutput> output;
		for (UINT i = 0; SUCCEEDED(adapter->EnumOutputs(i, &output)); ++i)
		{
			DXGI_OUTPUT_DESC desc{};
			if (SUCCEEDED(output->GetDesc(&desc)))
			{
				int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, desc.DeviceName, -1, NULL, 0, NULL, NULL);
				std::string systemId(sizeNeeded > 1 ? sizeNeeded - 1 : 0, 0);
				if (sizeNeeded > 1)
				{
					WideCharToMultiByte(CP_UTF8, 0, desc.DeviceName, -1, systemId.data(), sizeNeeded, NULL, NULL);
				}
				std::string platformMonitorId = MonitorUtils::MakeId(systemId);

				Size2D<zU32> resolution{
					static_cast<zU32>(desc.DesktopCoordinates.right - desc.DesktopCoordinates.left),
					static_cast<zU32>(desc.DesktopCoordinates.bottom - desc.DesktopCoordinates.top)
				};

				zI32 posX = desc.DesktopCoordinates.left;
				zI32 posY = desc.DesktopCoordinates.top;
				bool isPrimary = (posX == 0 && posY == 0);

				selector.AddMonitor(MonitorInfo(platformMonitorId, systemId, resolution, posX, posY, isPrimary));
			}
		}

		selector.SelectMonitor();
	}
#pragma endregion

	void DirectX12API::WaitForGpu()
	{
		if (!m_CommandQueue || !m_Fence || !m_FenceEvent)
			return;

		const uint64_t fenceValue = ++m_FenceValue;

		if (FAILED(m_CommandQueue->Signal(m_Fence.Get(), fenceValue)))
			return;

		if (m_Fence->GetCompletedValue() < fenceValue)
		{
			if (FAILED(m_Fence->SetEventOnCompletion(fenceValue, m_FenceEvent)))
				return;

			WaitForSingleObject(m_FenceEvent, INFINITE);
		}
	}

	uint64_t DirectX12API::SignalFence()
	{
		if (!m_CommandQueue || !m_Fence)
			return 0;

		const uint64_t fenceValue = ++m_FenceValue;
		m_CommandQueue->Signal(m_Fence.Get(), fenceValue);
		return fenceValue;
	}

	void DirectX12API::WaitForFenceValue(uint64_t fenceValue)
	{
		if (!m_Fence || !m_FenceEvent || fenceValue == 0)
			return;

		if (m_Fence->GetCompletedValue() < fenceValue)
		{
			m_Fence->SetEventOnCompletion(fenceValue, m_FenceEvent);
			WaitForSingleObject(m_FenceEvent, INFINITE);
		}
	}
}
#endif // Z_D3D12
