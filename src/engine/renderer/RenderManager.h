#pragma once

#include "engine/EngineIncludes.h"
#include "engine/renderer/SceneRenderTree.h"

#include <memory>

namespace zzz::engine
{
	class ISurfView;
	class Scene;

	class RenderManager final
	{
		Z_NO_COPY_MOVE(RenderManager);

	public:
		RenderManager() = delete;
		explicit RenderManager(std::shared_ptr<ISurfView> surfView);

		void PrepareFrame(const std::shared_ptr<Scene>& scene);
		void RenderFrame();

	private:
		void BuildRenderTree(const Scene& scene);

		std::shared_ptr<ISurfView> m_SurfView;
		SceneRenderTree m_RenderTree;
	};
}
