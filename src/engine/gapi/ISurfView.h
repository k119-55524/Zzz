#pragma once

#include "engine/EngineIncludes.h"
#include "engine/gapi/IGAPI.h"
#include "engine/platforms/window/NativeWindow.h"

namespace zzz::engine
{
	using namespace zzz::core;

	class ISurfView
	{
		Z_NO_COPY_MOVE(ISurfView);

	public:
		ISurfView() = delete;
		explicit ISurfView(std::shared_ptr<NativeWindow> window, std::shared_ptr<IGAPI> gapi) :
			m_Window{ std::move(window) },
			m_GAPI{ std::move(gapi) }
		{
			ensure(m_Window != nullptr, "NativeWindow не должен быть null.");
			ensure(m_GAPI != nullptr, "IGAPI не должен быть null.");
		}

		virtual ~ISurfView() = default;

		[[nodiscard]] std::shared_ptr<NativeWindow> GetWindow() const noexcept { return m_Window; }
		[[nodiscard]] Size2D<> GetSize() const noexcept { return m_Window ? m_Window->GetClientRect().GetSize() : Size2D<>{}; }

		virtual void PreRender() {}
		virtual void PrepareFrame() = 0;
		virtual void RenderFrame() = 0;
		virtual void PostRender() {}

		virtual void OnResize(const Size2D<>& size) = 0;
		virtual void OnUpdateVSyncState() {}

	protected:
		virtual std::expected<void, std::string> Initialize() = 0;

		std::shared_ptr<NativeWindow> m_Window;
		std::shared_ptr<IGAPI> m_GAPI;

		uint32_t m_IndexRender{ 1 };
		uint32_t m_IndexPrepare{ 0 };
	};
}
