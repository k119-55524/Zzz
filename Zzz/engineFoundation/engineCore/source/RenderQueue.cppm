
export module RenderQueue;

import Scene;
import Camera;
import Colors;
import Matrix4x4;
import RenderArea;
import ViewportDesc;
import PrimitiveTopology;

using namespace zzz;
using namespace zzz::math;
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

		template<
			typename ClearFunc,
			typename LayerFunc,
			typename SetGlobalConstFunc,
			typename SetMeshTopologyFunc,
			typename SetPSOFunc,
			typename SetMeshConstFunc,
			typename RenderInexedMeshFunc>
		void PrepareQueue(
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
			clearFunc(m_SurfClearType, m_ClearColor, b_IsClearDepth);

			// Установка viewport и scissor rect
			layerFunc(m_RenderArea->GetViewport(), m_RenderArea->GetScissor());

			// Установка глобальных констант шейдеров
			Camera& primaryCamera = m_Scene->GetPrimaryCamera();
			Matrix4x4 camViewProj = primaryCamera.GetProjectionViewMatrix();
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

//{
	//Matrix4x4 mView;
	//Matrix4x4 mProj;
	//
	//mProj = Matrix4x4::perspective(
	//	0.25f * Pi,		// FoV 45 градусов
	//	16.0f / 9.0f,	// Aspect ratio
	//	1.0f,			// Near plane
	//	1000.0f);		// Far plane
	//
	//float mTheta = 1.5f * Pi;
	//float mPhi = Pi / 4.0f;  // 45 градусов
	//float mRadius = 5.0f;
	//
	//float x = mRadius * std::sin(mPhi) * std::cos(mTheta);
	//float z = mRadius * std::sin(mPhi) * std::sin(mTheta);
	//float y = mRadius * std::cos(mPhi);
	//
	//Vector4 pos(x, y, z, 1.0f);
	//Vector4 target(0.0f, 0.0f, 0.0f, 1.0f);
	//Vector4 up(0.0f, 1.0f, 0.0f, 0.0f);
	//mView = Matrix4x4::lookAt(pos, target, up);
	//Matrix4x4 worldViewProj = mProj * mView * mWorld;
//}