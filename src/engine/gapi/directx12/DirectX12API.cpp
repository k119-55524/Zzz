#include "engine/gapi/directx12/DirectX12API.h"

#if defined(Z_D3D12)

namespace zzz::engine
{
	DirectX12API::DirectX12API(std::shared_ptr<UserSettingsManager> userSettings)
		: IGAPI(std::move(userSettings), eGAPIType::DirectX12)
	{
	}

	DirectX12API::~DirectX12API()
	{
	}

	std::expected<void, std::string> DirectX12API::Init()
	{
		return {};
	}

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
