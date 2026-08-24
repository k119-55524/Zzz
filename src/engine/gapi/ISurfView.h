#pragma once

#include "engine/gapi/GAPI.h"
#include "engine/gapi/Swapchain.h"
#include "engine/gapi/DepthBuffer.h"
#include "engine/platforms/window/NativeWindow.h"

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
			m_GAPI{ std::move(gapi) }
		{
			ensure(m_Window != nullptr, "NativeWindow не должен быть null.");
			ensure(m_GAPI != nullptr, "GAPI не должен быть null.");
		}

		virtual ~ISurfView() = default;

		[[nodiscard]] std::shared_ptr<NativeWindow> GetWindow() const noexcept { return m_Window; }
		[[nodiscard]] std::shared_ptr<GAPI> GetGAPI() const noexcept { return m_GAPI; }
		[[nodiscard]] Swapchain* GetSwapchain() const noexcept { return m_Swapchain.get(); }
		[[nodiscard]] DepthBuffer* GetDepthBuffer() const noexcept { return m_DepthBuffer.get(); }

		virtual void PreRender() {}
		virtual void PrepareFrame() = 0;
		virtual void RenderFrame() = 0;
		virtual void PostRender() {}

		virtual void OnResize(const Size2D<>& size) = 0;
		virtual void OnUpdateVSyncState() {}

	protected:
		virtual void Initialize() = 0;

		std::shared_ptr<NativeWindow> m_Window;
		std::shared_ptr<GAPI> m_GAPI;

		std::unique_ptr<Swapchain> m_Swapchain;
		std::unique_ptr<DepthBuffer> m_DepthBuffer;

		Size2D<> m_OldSize{};

		uint32_t m_IndexRender{ 1 };
		uint32_t m_IndexPrepare{ 0 };
	};
}
