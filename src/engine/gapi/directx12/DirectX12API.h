#pragma once

#include "engine/gapi/IGAPI.h"

#if defined(Z_D3D12)

namespace zzz::engine
{
	class DirectX12API final : public IGAPI
	{
	public:
		explicit DirectX12API(std::shared_ptr<UserSettingsManager> userSettings);
		~DirectX12API() override;

		void SubmitCommandLists() override;
		void BeginRender() override;
		void EndRender() override;

	protected:
		void WaitForGpu() override;

	private:
		friend class Engine;
		void Initialize() override;

		void EnableDebugLayer(UINT& dxgiFactoryFlags);
	};
}

#endif // Z_D3D12
