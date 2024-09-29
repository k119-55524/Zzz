#pragma once

#include "GAPIBase.h"

namespace Zzz::Platforms
{
	class DirectX12API : public GAPIBase
	{
	public:
		DirectX12API() = delete;
		DirectX12API(DirectX12API&) = delete;
		DirectX12API(DirectX12API&&) = delete;

		DirectX12API(unique_ptr<WinAppBase> _win);
		~DirectX12API();

		zResult Initialize(const s_zEngineInit* const data) override;

		//const ComPtr<ID3D12Device>& GetDevice() const noexcept { return m_device; };

	protected:
		void OnUpdate() override;
		void OnRender() override;
	};
}
