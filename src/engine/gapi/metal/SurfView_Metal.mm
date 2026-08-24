#include "SurfView_Metal.h"

#if defined(Z_METAL)

namespace zzz::engine
{
	SurfView_Metal::SurfView_Metal(std::shared_ptr<NativeWindow> window, std::shared_ptr<MetalAPI> gapi) :
		ISurfView(std::move(window), std::move(gapi))
	{
		Initialize();
	}

	void SurfView_Metal::Initialize()
	{
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
