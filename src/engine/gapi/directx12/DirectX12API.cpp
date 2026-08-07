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
	}

	void DirectX12API::EnableDebugLayer(UINT& dxgiFactoryFlags)
	{
#if defined(Z_DEBUG_BUILD)
		Microsoft::WRL::ComPtr<ID3D12Debug> debugController;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
		{
			debugController->EnableDebugLayer();
			dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;

			// Даёт дополнительную проверку корректности работы с GPU ресурсами
			// но работает медленнее
			//ComPtr<ID3D12Debug1> debugController1;
			//debugController->QueryInterface(IID_PPV_ARGS(&debugController1));
			//debugController1->SetEnableGPUBasedValidation(true);

			DOut("DirectX debug layer enabled.");
		}
		else
			THROW_RUNTIME("Failed to enable DirectX debug layer.");
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
