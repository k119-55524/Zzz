#pragma once

#include "engine/gapi/ISurfView.h"
#include "engine/gapi/metal/MetalAPI.h"
#include "engine/platforms/window/NativeWindow.h"

#if defined(Z_METAL)
namespace zzz::engine
{
	class SurfView_Metal final : public ISurfView
	{
		Z_NO_COPY_MOVE(SurfView_Metal);

	public:
		SurfView_Metal(std::shared_ptr<NativeWindow> window, std::shared_ptr<MetalAPI> gapi);
		~SurfView_Metal() override = default;

		void PrepareFrame(const ClearConfig& clearConfig) override;
		void SubmitRenderTree(const SceneRenderTree& renderTree) override;
		void RenderFrame() override;
		void OnResize(const Size2D<>& size) override;

		void OnSurfaceCreated(void* handle) override;
		void OnSurfaceDestroyed() override;

	protected:
		void Initialize() override;
	};
}
#endif // Z_METAL
