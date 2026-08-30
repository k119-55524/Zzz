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

		void PreRender() override;
		void PrepareFrame(const ClearConfig& clearConfig) override;
		void SubmitRenderTree(const SceneRenderTree& renderTree) override;
		void RenderFrame() override;
		void OnResize(const Size2D<>& size) override;

#pragma region Surface Lifecycle
		void OnSurfaceCreated(void* handle) override;
		void OnSurfaceDestroyed() override;
#pragma endregion

	protected:
		void Initialize() override;

	private:
		std::array<ComPtr<ID3D12CommandAllocator>, c_FramesInFlight> m_CommandAllocators;
		std::array<ComPtr<ID3D12GraphicsCommandList>, c_FramesInFlight> m_CommandLists;
		std::array<bool, c_FramesInFlight> m_IsRecording{};
		std::array<bool, c_FramesInFlight> m_FrameReady{};
		std::array<ID3D12Resource*, c_FramesInFlight> m_FrameBackBuffer{};

		std::array<uint64_t, c_FramesInFlight> m_FrameFenceValues{};

		std::mutex m_SubmitMutex;
	};
}

#endif // Z_D3D12
