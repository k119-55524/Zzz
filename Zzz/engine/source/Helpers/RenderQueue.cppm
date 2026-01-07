
export module RenderQueue;

import Scene;
import Camera;
import Matrix4x4;
export import ViewSetup;
import PrimitiveTopology;

using namespace zzz::core;
using namespace zzz::math;

export namespace zzz
{
	// Класс позволяет накапливать в себе объекты рендринга для последующей их отрисовки
	export class RenderQueue
	{
	public:
		RenderQueue(const std::shared_ptr<ViewSetup> viewSetup) :
			m_ViewSetup{ viewSetup }
		{
		}
		virtual ~RenderQueue() = default;

		// Очищает очередь рендринга перед началом нового кадра
		void ClearQueue(const std::shared_ptr<Scene> scene) noexcept { m_Scene = scene; }

		template<
			typename ClearFunc,
			typename LayerFunc,
			typename SetGlobalConstFunc,
			typename SetMeshTopologyFunc,
			typename SetPSOFunc,
			typename SetMeshConstFunc,
			typename RenderInexedMeshFunc>
		void PrepareQueue(
			Size2D<>& surfSize,
			ClearFunc&& clearFunc,
			LayerFunc&& layerFunc,
			SetGlobalConstFunc&& setGlobalConstFunc,
			SetPSOFunc&& setPSOFunc,
			SetMeshTopologyFunc&& topoFunc,
			SetMeshConstFunc&& setMeshConstFunc,
			RenderInexedMeshFunc&& renderInexedMeshFunc) const
		{
			PrimitiveTopology currTopo;

			// Очистка поверхности
			clearFunc(m_ViewSetup->GetSurfClearType(), m_ViewSetup->GetClearColor(), m_ViewSetup->IsClearDepth());

			// Установка viewport и scissor rect
			layerFunc(m_ViewSetup->GetViewport(), m_ViewSetup->GetScissor());

			// Установка глобальных констант шейдеров
			Camera& primaryCamera = m_Scene->GetPrimaryCamera();
			Matrix4x4 camViewProj = primaryCamera.GetProjectionViewMatrix(surfSize);
			setGlobalConstFunc(camViewProj);

			auto entity = m_Scene->GetEntity();
			auto material = entity->GetMaterial();
			auto pso = material->GetPSO();

			// Установка PSO(set material)
			setPSOFunc(pso);

			// Установка топологии примитивов
			if (pso->GetPrimitiveTopology() != currTopo)
			{
				currTopo = pso->GetPrimitiveTopology();
				topoFunc(currTopo);
			}

			Matrix4x4 world;
			world = world.translation(0.0f, 0.0f, 3.0f);
			Matrix4x4 worldViewProj = world * camViewProj;
			setMeshConstFunc(worldViewProj);

			// Рендринг меша
			auto mesh = entity->GetMesh();
			renderInexedMeshFunc(mesh, 36);
		}

	protected:
		std::shared_ptr<ViewSetup> m_ViewSetup;
		std::shared_ptr<Scene> m_Scene;
	};
}