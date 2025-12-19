
export module RenderQueue;

import Scene;
import Colors;
import RenderArea;
import ViewportDesc;
import PrimitiveTopology;

using namespace zzz;
using namespace zzz::colors;

export namespace zzz::core
{
	// Класс позволяет накапливать в себе объекты рендринга для последующей их отрисовки
	export class RenderQueue
	{
	public:
		RenderQueue() :
			m_ClearColor{ colors::DarkMidnightBlue },
			b_IsClearDepth{ true }
		{
			m_SurfClearType = eSurfClearType::Color;
		}
		virtual ~RenderQueue() = default;

		// Очищает очередь рендринга перед началом нового кадра
		void ClearQueue(const std::shared_ptr<RenderArea> renderArea, const std::shared_ptr<Scene> scene);

		template<typename ClearFunc, typename LayerFunc, typename SetMeshTopologyFunc, typename SetPSOFunc, typename RenderInexedMeshFunc>
		void PrepareQueue(ClearFunc&& clearFunc, LayerFunc&& layerFunc, SetMeshTopologyFunc&& topoFunc, SetPSOFunc&& setPSOFunc, RenderInexedMeshFunc&& renderInexedMeshFunc) const
		{
			PrimitiveTopology currTopo;

			// Очистка поверхности
			clearFunc(m_SurfClearType, m_ClearColor, b_IsClearDepth);

			// Установка viewport и scissor rect
			layerFunc(m_RenderArea->GetViewport(), m_RenderArea->GetScissor());

			auto entity = m_Scene->GetEntity();
			auto material = entity->GetMaterial();
			auto pso = material->GetPSO();

			// Установка топологии примитивов
			if (pso->GetPrimitiveTopology() != currTopo)
			{
				currTopo = pso->GetPrimitiveTopology();
				topoFunc(currTopo);
			}

			// Установка PSO(set material)
			setPSOFunc(pso);

			// Рендринг меша
			auto mesh = entity->GetMesh();
			renderInexedMeshFunc(mesh, 36);
		}

	protected:
		std::shared_ptr<RenderArea> m_RenderArea;
		std::shared_ptr<Scene> m_Scene;

		eSurfClearType m_SurfClearType;
		Color m_ClearColor;
		bool b_IsClearDepth;
	};

	void RenderQueue::ClearQueue(const std::shared_ptr<RenderArea> renderArea, const std::shared_ptr<Scene> scene)
	{
		m_RenderArea = renderArea;
		m_Scene = scene;
	}
}