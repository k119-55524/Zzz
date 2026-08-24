#include "SurfView_Metal.h"

#if defined(Z_METAL)

namespace zzz::engine
{
	SurfView_Metal::SurfView_Metal(std::shared_ptr<NativeWindow> window, std::shared_ptr<IGAPI> gapi) :
		ISurfView(window, gapi),
		m_MetalAPI(std::dynamic_pointer_cast<MetalAPI>(gapi))
	{
		ensure(m_Window, "Window cannot be null.");
		ensure(m_MetalAPI, "Failed to cast IGAPI to MetalAPI.");

		auto res = Initialize();
		if (!res)
		{
			THROW_RUNTIME("Failed to initialize SurfView_Metal: {}", res.error());
		}
	}

	std::expected<void, std::string> SurfView_Metal::Initialize()
	{
		return {};
	}

	void SurfView_Metal::PrepareFrame()
	{
	}

	void SurfView_Metal::RenderFrame()
	{
	}

	void SurfView_Metal::OnResize(const Size2D<>& size)
	{
		(void)size;
	}
}

#endif // Z_METAL
