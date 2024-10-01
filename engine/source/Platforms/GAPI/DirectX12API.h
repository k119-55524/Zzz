#pragma once

#include "IGAPI.h"

namespace Zzz::Platforms
{
#ifdef _WINDOWS

	class DirectX12API : public IGAPI
	{
	public:
		DirectX12API();
		DirectX12API(DirectX12API&) = delete;
		DirectX12API(DirectX12API&&) = delete;

		virtual ~DirectX12API();

		zResult Initialize(const s_zEngineInit* const data) override;

		//const ComPtr<ID3D12Device>& GetDevice() const noexcept { return m_device; };

	protected:
		void OnUpdate() override;
		void OnRender() override;
	};

#endif // _WINDOWS
}
