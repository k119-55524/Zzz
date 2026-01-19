
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

					// Пример изменения трансформации объекта
					{
						//Transform& transform = entity->GetTransform();

						//Vector3 position = transform.GetPosition();
						//position.set_x(-1.2f);
						//position.set_y(-1.2f);
						//position.set_z(3.0f);
						//transform.SetPosition(position);

						//Vector3 scale = transform.GetScale();
						//scale.set_x(0.5f);
						//scale.set_y(0.5f);
						//scale.set_z(0.5f);
						//transform.SetScale(scale);

						//static int c = 0;
						//static float f = 0.0f;
						//c++;
						//if (c > 50)
						//{
						//	c = 0;
						//	f += 0.01f;
						//}

						//Quaternion rotationQuat = Quaternion::fromEulerXYZ(f, f / 2.0f, 0.0f);
						//transform.SetRotation(rotationQuat);
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

		void BuildQueue();
	};

	void RenderQueue::BuildQueue()
	{

	}
}