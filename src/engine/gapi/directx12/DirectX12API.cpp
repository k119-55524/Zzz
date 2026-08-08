#include "engine/gapi/directx12/DirectX12API.h"

#if defined(Z_D3D12)

namespace zzz::engine
{
	DirectX12API::DirectX12API(std::shared_ptr<UserSettingsManager> userSettings)
		: IGAPI(std::move(userSettings))
	{
	}

	DirectX12API::~DirectX12API()
	{
	}

#pragma region Initialize
	void DirectX12API::Initialize()
	{
		UINT dxgiFactoryFlags = 0;
		EnableDebugLayer(dxgiFactoryFlags);
		InitializeDevice(dxgiFactoryFlags);
	}

	void DirectX12API::EnableDebugLayer(UINT& dxgiFactoryFlags)
	{
#if defined(Z_DEBUG_BUILD)
		Microsoft::WRL::ComPtr<ID3D12Debug> debugController;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
		{
			debugController->EnableDebugLayer();
			dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;

			DOut("DirectX debug layer enabled.");
		}
		else
			THROW_RUNTIME("Failed to enable DirectX debug layer.");
#endif
	}

	void DirectX12API::InitializeDevice(UINT dxgiFactoryFlags)
	{
		CreateFactory(dxgiFactoryFlags, m_Factory);

		Microsoft::WRL::ComPtr<IDXGIAdapter1> adapter;
		GetAdapter(m_Factory.Get(), adapter);

		HRESULT hr = adapter.As(&m_Adapter3);
		if (FAILED(hr))
			THROW_RUNTIME("Failed to query IDXGIAdapter3. HRESULT = 0x{:08X}", static_cast<unsigned int>(hr));

		CreateDevice(adapter, m_Device, m_FeatureLevel);

		// Проверка поддержки отмены VSYNC (Allow Tearing)
		BOOL allowTearing = FALSE;
		hr = m_Factory->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof(allowTearing));
		if (SUCCEEDED(hr))
			m_IsCanDisableVSync = allowTearing;
	}

	void DirectX12API::CreateFactory(UINT dxgiFactoryFlags, Microsoft::WRL::ComPtr<IDXGIFactory7>& outFactory)
	{
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
	}

	void DirectX12API::GetAdapter(IDXGIFactory1* pFactory, Microsoft::WRL::ComPtr<IDXGIAdapter1>& outAdapter)
	{
		if (!pFactory)
			THROW_RUNTIME("Invalid DXGI Factory argument");

		Microsoft::WRL::ComPtr<IDXGIAdapter1> adapter;
		Microsoft::WRL::ComPtr<IDXGIFactory6> factory6;

		auto trySelectAdapter = [&](IDXGIAdapter1* candidate) -> bool
		{
			DXGI_ADAPTER_DESC1 desc{};
			candidate->GetDesc1(&desc);

			if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
				return false;

			if (SUCCEEDED(D3D12CreateDevice(candidate, D3D_FEATURE_LEVEL_12_0, __uuidof(ID3D12Device), nullptr)))
			{
				DOut("Selected adapter: {} VRAM: {} MB", std::string(desc.Description, desc.Description + wcslen(desc.Description)), desc.DedicatedVideoMemory / (1024 * 1024));
				outAdapter = candidate;
				return true;
			}
			return false;
		};

		if (SUCCEEDED(pFactory->QueryInterface(IID_PPV_ARGS(&factory6))))
		{
			for (UINT adapterIndex = 0; SUCCEEDED(factory6->EnumAdapterByGpuPreference(adapterIndex, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&adapter))); ++adapterIndex)
			{
				if (trySelectAdapter(adapter.Get()))
					return;
			}
		}

		for (UINT adapterIndex = 0; SUCCEEDED(pFactory->EnumAdapters1(adapterIndex, &adapter)); ++adapterIndex)
		{
			if (trySelectAdapter(adapter.Get()))
				return;
		}

		THROW_RUNTIME("Failed to find suitable D3D12 GPU adapter");
	}

	void DirectX12API::CreateDevice(Microsoft::WRL::ComPtr<IDXGIAdapter1> adapter, Microsoft::WRL::ComPtr<ID3D12Device>& outDevice, D3D_FEATURE_LEVEL& outFeatureLevel)
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
			THROW_RUNTIME("Failed to create D3D12 device. HRESULT = 0x{:08X}", static_cast<unsigned int>(hr));

#if defined(Z_DEBUG_BUILD)
		std::string levelName = (outFeatureLevel == D3D_FEATURE_LEVEL_12_2) ? "12.2 (DirectX 12 Ultimate)" :
			(outFeatureLevel == D3D_FEATURE_LEVEL_12_1) ? "12.1" :
			(outFeatureLevel == D3D_FEATURE_LEVEL_12_0) ? "12.0" : "Unknown";
		DOut("Created D3D12 device with feature level: {}", levelName);
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
