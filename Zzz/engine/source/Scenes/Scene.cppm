
export module Scene;

import Math;
import Input;
import Result;
import Camera;
import Transform;
import StrConvert;
//import SceneEntity;
import SceneEntityFactory;

using namespace zzz::math;
using namespace zzz::input;

namespace zzz
{
	export class SceneEntity;

	export class Scene final
	{
		Z_NO_COPY_MOVE(Scene);

	public:
		Scene() = delete;
		~Scene() = default;
		Scene(const std::shared_ptr<Input> input, const std::shared_ptr<SceneEntityFactory> sceneEntityFactory);

		Result<std::shared_ptr<SceneEntity>> AddColorBox(Transform& transform);

		inline [[nodiscard]] const Camera& GetPrimaryCamera() noexcept { return m_PrimaryCamera; }
		[[nodiscard]] std::shared_ptr<SceneEntity> GetEntity() const noexcept;
		inline [[nodiscard]] const std::shared_ptr<Input> GetInput() const noexcept { return m_Input; }

	private:
		const std::shared_ptr<Input> m_Input;
		std::shared_ptr<SceneEntityFactory> m_EntityFactory;

		Camera m_PrimaryCamera;
		std::shared_ptr<SceneEntity> m_Entity;
	};
//}

//module :private;
//
//import SceneEntity;
//
//namespace zzz
//{
	inline Scene::Scene(
		const std::shared_ptr<Input> input,
		const std::shared_ptr<SceneEntityFactory> sceneEntityFactory
	) :
		m_EntityFactory{ sceneEntityFactory }
	{
		ensure(input, "Scene input cannot be null.");
		ensure(m_EntityFactory, "Scene entity factory cannot be null.");

		m_PrimaryCamera.SetFovY(0.25f * PI);
		//m_PrimaryCamera.SetAspectRatio(eAspectType::Ratio_16x9);
		m_PrimaryCamera.SetAspectRatio(eAspectType::FullWindow, 1.0f);
		m_PrimaryCamera.SetNearPlane(0.1f);
		m_PrimaryCamera.SetFarPlane(1000.0f);
		float mTheta = 1.5f * PI;
		float mPhi = PI / 4.0f; // 45 градусов
		float mRadius = 5.0f;
		float x = mRadius * std::sin(mPhi) * std::cos(mTheta);
		float z = mRadius * std::sin(mPhi) * std::sin(mTheta);
		float y = mRadius * std::cos(mPhi);
		//float x = 0.0f;
		//float y = 0.0f;
		//float z = -5.0f;
		m_PrimaryCamera.SetPosition(Vector3(x, y, z));
		//m_PrimaryCamera.SetPosition(Vector4(0, 0, -5, 1.0f));
		m_PrimaryCamera.SetTarget(Vector3(0.0f, 0.0f, 0.0f));
		m_PrimaryCamera.SetUp(Vector3(0.0f, 1.0f, 0.0f));
	}

	[[nodiscard]] inline std::shared_ptr<SceneEntity> Scene::GetEntity() const noexcept
	{
		return m_Entity;
	}

	inline Result<std::shared_ptr<SceneEntity>> Scene::AddColorBox(Transform& transform)
	{
		std::shared_ptr<SceneEntity> entity;
		try
		{
			Result<std::shared_ptr<SceneEntity>> resEntity = m_EntityFactory->GetColorBox();
			if (!resEntity)
				return resEntity.error();

			entity = resEntity.value();
			entity->SetTransform(transform);
			m_Entity = entity;
		}
		catch (const std::exception& e)
		{
			return Unexpected(eResult::exception, string_to_wstring(e.what()).value_or(L"Unknown exception occurred."));
		}

		return entity;
	}
}