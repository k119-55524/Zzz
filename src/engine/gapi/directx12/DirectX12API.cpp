#include "engine/gapi/directx12/DirectX12API.h"

#if defined(Z_D3D12)

namespace zzz::engine
{
	DirectX12API::~DirectX12API()
	{
	}

#pragma region Initialize
	void DirectX12API::Initialize(std::shared_ptr<UserSettingsManager> userSettings)
	{
		UINT dxgiFactoryFlags = 0;
		EnableDebugLayer(dxgiFactoryFlags);
		InitializeDevice(std::move(userSettings), dxgiFactoryFlags);
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

		Microsoft::WRL::ComPtr<IDXGIAdapter1> adapter = GetAdapter(m_Factory.Get(), userSettings);

		HRESULT hr = adapter.As(&m_Adapter3);
		if (FAILED(hr))
			THROW_RUNTIME("Failed to query IDXGIAdapter3. HRESULT = 0x{:08X}", static_cast<unsigned int>(hr));

		CreateDevice(adapter.Get());

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
			hr = D3D12CreateDevice(adapter, level, IID_PPV_ARGS(&m_Device));
			if (SUCCEEDED(hr))
			{
				m_FeatureLevel = level;
				break;
			}
		}

		if (FAILED(hr))
			THROW_RUNTIME("Failed to create D3D12 device. HRESULT = 0x{:08X}", static_cast<unsigned int>(hr));

#if defined(Z_DEBUG_BUILD)
		std::string levelName = (m_FeatureLevel == D3D_FEATURE_LEVEL_12_2) ? "12.2 (DirectX 12 Ultimate)" :
			(m_FeatureLevel == D3D_FEATURE_LEVEL_12_1) ? "12.1" :
			(m_FeatureLevel == D3D_FEATURE_LEVEL_12_0) ? "12.0" : "Unknown";
		DOut("[DirectX12API::CreateDevice] - Created D3D12 device with feature level: {}", levelName);
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
	}
}

#endif // Z_D3D12
