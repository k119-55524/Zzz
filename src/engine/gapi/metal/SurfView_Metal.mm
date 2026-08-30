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

	void SurfView_Metal::PrepareFrame(const ClearConfig& clearConfig)
	{
		(void)clearConfig;
		// [Logic] В Metal здесь должен быть вызов:
		// id<CAMetalDrawable> drawable = [layer nextDrawable];
		// m_PhysicalIndices[prepIdx] = ... (сохраняем объект или его индекс)
		// Цвет/глубину очистки брать из clearConfig.surface / clearConfig.depthBuffer.

		// nextDrawable блокирует поток, если все буферы заняты GPU.
	}

	void SurfView_Metal::SubmitRenderTree(const SceneRenderTree& renderTree)
	{
		// SceneRenderTree - пока заглушка (пустое дерево), трансляция бакетов Материалы -> Меши
		// в нативные encode-вызовы появится вместе с ResourceManager/MeshRenderer.
		(void)renderTree;
	}

	void SurfView_Metal::RenderFrame()
	{
		// [Logic] В конце рендеринга:
		// [commandBuffer presentDrawable:drawable];
		// [commandBuffer commit];
	}

	void SurfView_Metal::OnResize(const Size2D<>& size)
	{
		(void)size;
	}

	void SurfView_Metal::OnSurfaceCreated(void* handle)
	{
		(void)handle;
	}

	void SurfView_Metal::OnSurfaceDestroyed()
	{
	}
}

#endif // Z_METAL
