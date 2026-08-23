#pragma once

#include "engine/EngineIncludes.h"
#include "engine/gapi/IGAPI.h"

namespace zzz::engine
{
	class ISurfView
	{
		Z_NO_COPY_MOVE(ISurfView);

	public:
		explicit ISurfView(std::shared_ptr<IGAPI> gapi);
		virtual ~ISurfView() = default;

		virtual void PrepareFrame() = 0;
		virtual void RenderFrame() = 0;
		virtual void OnResize(const Size2D<>& size) = 0;
		virtual std::expected<void, std::string> OnUpdateVSyncState() { return {}; }

		virtual void PreRender() {}
		virtual void PostRender() {}

	protected:
		virtual std::expected<void, std::string> Initialize() = 0;

		std::shared_ptr<IGAPI> m_GAPI;

		uint32_t m_IndexRender{ 1 };
		uint32_t m_IndexPrepare{ 0 };
	};

	inline ISurfView::ISurfView(std::shared_ptr<IGAPI> gapi) :
		m_GAPI(gapi),
		m_IndexRender(1),
		m_IndexPrepare(0)
	{
		ensure(m_GAPI, "GAPI cannot be null.");
	}
}
