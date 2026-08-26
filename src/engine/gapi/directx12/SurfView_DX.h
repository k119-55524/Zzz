#pragma once

#include <mutex>
#include <array>
#include "engine/gapi/ISurfView.h"
#include "engine/gapi/directx12/DirectX12API.h"
#include "engine/platforms/window/NativeWindow.h"

#if defined(Z_D3D12)

using namespace zzz::core;
using namespace Microsoft::WRL;

namespace zzz::engine
{
	class SurfView_DX final : public ISurfView
	{
		Z_NO_COPY_MOVE(SurfView_DX);

	public:
		SurfView_DX(std::shared_ptr<NativeWindow> window, std::shared_ptr<DirectX12API> gapi);
		~SurfView_DX() override;

		void PrepareFrame() override;
		void RenderFrame() override;
		void OnResize(const Size2D<>& size) override;

		void SetClearConfig(const ViewClearConfig& config) override;

	protected:
		void Initialize() override;

	private:
		std::array<ComPtr<ID3D12CommandAllocator>, c_FramesInFlight> m_CommandAllocators;
		std::array<ComPtr<ID3D12GraphicsCommandList>, c_FramesInFlight> m_CommandLists;
		std::array<bool, c_FramesInFlight> m_IsRecording{};

		uint32_t m_IndexPrepare;
		uint32_t m_IndexRender;

		std::mutex m_FrameMutex;
	};
}

#endif // Z_D3D12
