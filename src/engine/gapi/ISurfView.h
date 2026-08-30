#pragma once

#include "engine/gapi/GAPI.h"
#include "engine/gapi/Swapchain.h"
#include "engine/gapi/DepthBuffer.h"
#include "engine/platforms/window/NativeWindow.h"
#include "engine/gapi/clear_config/ClearConfig.h"
#include "engine/renderer/SceneRenderTree.h"

using namespace zzz::core;

namespace zzz::engine
{
	class ISurfView
	{
		Z_NO_COPY_MOVE(ISurfView);

	public:
		ISurfView() = delete;
		explicit ISurfView(std::shared_ptr<NativeWindow> window, std::shared_ptr<GAPI> gapi) :
			m_Window{ std::move(window) },
			m_GAPI{ std::move(gapi) },
			m_IndexPrepare{ 0 },
			m_IndexRender{ 1 }
		{
			ensure(m_Window != nullptr, "NativeWindow не должен быть null.");
			ensure(m_GAPI != nullptr, "GAPI не должен быть null.");
		}

		virtual ~ISurfView() = default;

		[[nodiscard]] std::shared_ptr<NativeWindow> GetWindow() const noexcept { return m_Window; }
		[[nodiscard]] std::shared_ptr<GAPI> GetGAPI() const noexcept { return m_GAPI; }
		[[nodiscard]] Swapchain* GetSwapchain() const noexcept { return m_Swapchain.get(); }
		[[nodiscard]] DepthBuffer* GetDepthBuffer() const noexcept { return m_DepthBuffer.get(); }

		[[nodiscard]] uint32_t GetPrepareIndex() const noexcept { return m_IndexPrepare; }
		[[nodiscard]] uint32_t GetRenderIndex() const noexcept { return m_IndexRender; }

		[[nodiscard]] uint32_t GetPhysicalIndex(uint32_t logicalIndex) const noexcept { return m_PhysicalIndices[logicalIndex]; }

		virtual void PreRender() {}
		virtual void PrepareFrame(const ClearConfig& clearConfig) = 0;
		virtual void SubmitRenderTree(const SceneRenderTree& renderTree) = 0;
		virtual void RenderFrame() = 0;
		inline void PostRender() noexcept
		{
			m_IndexPrepare = (m_IndexPrepare + 1) % c_FramesInFlight;
			m_IndexRender  = (m_IndexRender + 1) % c_FramesInFlight;
		}

		virtual void OnResize(const Size2D<>& size) = 0;
		virtual void OnUpdateVSyncState() {}

		virtual void OnSurfaceCreated(void* handle) = 0;
		virtual void OnSurfaceDestroyed() = 0;

	protected:
		virtual void Initialize() = 0;

		std::shared_ptr<NativeWindow> m_Window;
		std::shared_ptr<GAPI> m_GAPI;
		std::unique_ptr<Swapchain> m_Swapchain;
		std::unique_ptr<DepthBuffer> m_DepthBuffer;

		Size2D<> m_OldSize{};
		std::array<uint32_t, c_FramesInFlight> m_PhysicalIndices{};

	private:
		uint32_t m_IndexRender;
		uint32_t m_IndexPrepare;
	};
}
