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

		zResult Initialize(const DataEngineInitialization& data) override;

	protected:
		void OnUpdate() override;
		void OnRender() override;
		void OnResize(const zSize& size) override;
	};

#endif // _WINDOWS
}
