#pragma once

#include "engine/gapi/ISurfView.h"
#include "engine/gapi/directx12/DirectX12API.h"
#include "engine/platforms/window/NativeWindow.h"

#if defined(Z_D3D12)

namespace zzz::engine
{
	class SurfView_DX final : public ISurfView
	{
		Z_NO_COPY_MOVE(SurfView_DX);

	public:
		SurfView_DX(std::shared_ptr<NativeWindow> window, std::shared_ptr<IGAPI> gapi);
		~SurfView_DX() override = default;

		void PrepareFrame() override;
		void RenderFrame() override;
		void OnResize(const Size2D<>& size) override;

	protected:
		std::expected<void, std::string> Initialize() override;

	private:
		std::shared_ptr<NativeWindow> m_Window;
		std::shared_ptr<DirectX12API> m_DirectX12API;
	};
}

#endif // Z_D3D12
