#include "RenderManager.h"

#include "engine/gapi/ISurfView.h"
#include "engine/gapi/clear_config/ClearConfig.h"
#include "engine/scene/Scene.h"

Z_SET_LOG_CATEGORY(::zzz::core::GAPI);

using namespace zzz::core;

namespace zzz::engine
{
	static constexpr ClearConfig c_FallbackClearConfig{
		SurfaceClearConfig{ eSurfaceClearMode::Color, zzz::math::Palette4::Black },
		DepthBufferClearConfig{ eClearDepthMode::Depth, 1.0f }
	};

	RenderManager::RenderManager(std::shared_ptr<ISurfView> surfView) :
		m_SurfView(std::move(surfView))
	{
		ensure(m_SurfView != nullptr, "ISurfView не должен быть null в RenderManager.");
	}

	void RenderManager::PreRender()
	{
		m_SurfView->PreRender();
	}

	void RenderManager::PrepareFrame(const std::shared_ptr<Scene>& scene)
	{
		const ClearConfig& clearConfig = scene ? scene->GetClearConfig() : c_FallbackClearConfig;
		m_SurfView->PrepareFrame(clearConfig);

		if (!scene)
			return;

		// 3. Формирование SceneRenderTree
		BuildRenderTree(*scene);

		// 4. Передача дерева в GAPI
		m_SurfView->SubmitRenderTree(m_RenderTree);
	}

	void RenderManager::BuildRenderTree(const Scene& scene)
	{
		(void)scene;
		m_RenderTree.Clear(); // Сброс данных прошлого кадра
	}

	void RenderManager::RenderFrame()
	{
		m_SurfView->RenderFrame();
	}

	void RenderManager::PostRender()
	{
		m_SurfView->PostRender();
	}
}
