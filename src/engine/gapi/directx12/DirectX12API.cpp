
#if defined(Z_D3D12)
#include "engine/utils/MonitorUtils.h"
#include "engine/gapi/directx12/DirectX12API.h"
#include "engine/gapi/selectors/monitor/MonitorSelector.h"
#include "engine/gapi/selectors/gpu/directx12/DirectX12GpuSelector.h"
#include "core/utils/ToStringHelpers.h"


namespace zzz::engine
{
	DirectX12API::~DirectX12API()
	{
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
#if defined(Z_DEBUG_BUILD)
		Microsoft::WRL::ComPtr<ID3D12Debug> debugController;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
		{
			debugController->EnableDebugLayer();
			dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;

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

		D3D12_COMMAND_QUEUE_DESC queueDesc{};
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

		hr = m_Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(m_CommandQueue.ReleaseAndGetAddressOf()));
		if (FAILED(hr))
			THROW_RUNTIME("Failed to create D3D12 Direct Command Queue. HRESULT = 0x{:08X}", static_cast<unsigned int>(hr));

		hr = m_Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_Fence.ReleaseAndGetAddressOf()));
		if (FAILED(hr))
			THROW_RUNTIME("Failed to create D3D12 Fence. HRESULT = 0x{:08X}", static_cast<unsigned int>(hr));

		m_FenceValue = 0;
		m_FenceEvent = CreateEventEx(nullptr, FALSE, FALSE, EVENT_ALL_ACCESS);
		if (!m_FenceEvent)
			THROW_RUNTIME("Failed to create D3D12 Fence Event.");

#if defined(Z_DEBUG_BUILD)
		DOut("[DirectX12API::CreateDevice] - Created D3D12 device with feature level: {}", m_FeatureLevel);
#endif
	}
#pragma endregion

	void DirectX12API::SubmitCommandLists()
	{
	}

	void DirectX12API::BeginRender()
	{
	}

	void DirectX12API::EndRender()
	{
	}

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
}

#endif // Z_D3D12
