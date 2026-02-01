
export module RenderQueue;

import Matrix4x4;

export import Scene;
export import Camera;
export import ViewSetup;
export import IRenderLayer;
export import PrimitiveTopology;

using namespace zzz::math;

export namespace zzz
{
	// Класс позволяет накапливать в себе объекты рендринга для последующей их отрисовки
	export class RenderQueue
	{
	public:
		RenderQueue(const std::shared_ptr<ViewSetup> viewSetup,
			std::vector<std::shared_ptr<IRenderLayer>>& m_RenderLayers) :
			m_ViewSetup{ viewSetup },
			m_RenderLayers{ m_RenderLayers }
		{
		}
		virtual ~RenderQueue() = default;

		inline void SetDeltaTime(float deltaTime) noexcept { m_DeltaTime = deltaTime; };

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
			RenderInexedMeshFunc&& renderInexedMeshFunc)
		{
			BuildQueue();

			PrimitiveTopology currTopo;

			// Очистка поверхности
			clearFunc(m_ViewSetup->GetSurfClearType(), m_ViewSetup->GetClearColor(), m_ViewSetup->IsClearDepth());

			std::shared_ptr<Scene> scene;
			for (auto& layer : m_RenderLayers)
			{
				if (!layer->IsActive())
					continue;

				// Установка viewport и scissor rect
				layerFunc(layer->GetViewport(), layer->GetScissor());

				scene = layer->GetScene();
				if (scene != nullptr)
				{
					// Установка глобальных констант шейдеров
					Camera& primaryCamera = scene->GetPrimaryCamera();
					Matrix4x4 camViewProj = primaryCamera.GetProjectionViewMatrix(surfSize);
					setGlobalConstFunc(camViewProj);

					auto entity = scene->GetEntity();
					if (entity == nullptr)
						continue;

					entity->OnUpdate(m_DeltaTime);

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

					const Matrix4x4& world = entity->GetTransform().GetWorldMatrix();
					Matrix4x4 worldViewProj = world * camViewProj;
					setMeshConstFunc(worldViewProj);

					// Рендринг меша
					auto mesh = entity->GetMesh();
					renderInexedMeshFunc(mesh, mesh->GetIndexCount());
				}
			}
		}

	private:
		std::shared_ptr<ViewSetup> m_ViewSetup;
		std::vector<std::shared_ptr<IRenderLayer>>& m_RenderLayers;

		float m_DeltaTime;

		void BuildQueue();
	};

	void RenderQueue::BuildQueue()
	{

	}
}