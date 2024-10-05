#pragma once

#include "IGAPI.h"

namespace Zzz::Platforms
{
#ifdef _GAPI_DX12

	class DirectX12API : public IGAPI
	{
	public:
		DirectX12API() = delete;
		DirectX12API(DirectX12API&) = delete;
		DirectX12API(DirectX12API&&) = delete;
		virtual ~DirectX12API();

		DirectX12API(const shared_ptr<IWinApp> appWin);

		zResult Initialize(const DataEngineInitialization& data) override;

	protected:
		void OnUpdate() override;
		void OnRender() override;
		void OnResize(const zSize& size) override;

	private:
		//void GetAdapter(_In_ IDXGIFactory1* pFactory, _Outptr_result_maybenull_ IDXGIAdapter1** ppAdapter);

		//ComPtr<ID3D12Device> m_device;
	};

#endif // _GAPI_DX12
}
