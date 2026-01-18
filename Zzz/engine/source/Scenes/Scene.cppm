
export module Scene;

export import Math;
export import Camera;
export import SceneEntity;

using namespace zzz::math;

export namespace zzz
{
	export class Scene final
	{
		Z_NO_COPY_MOVE(Scene);

	public:
		Scene();
		~Scene();

		void Add(std::shared_ptr<SceneEntity> entity);

		inline Camera& GetPrimaryCamera() noexcept { return m_PrimaryCamera; }
		inline std::shared_ptr<SceneEntity> GetEntity() const noexcept { return m_Entity; }

	private:
		Camera m_PrimaryCamera;
		std::shared_ptr<SceneEntity> m_Entity;
	};

	export Scene::Scene()
	{
		m_PrimaryCamera.SetFovY(0.25f * Pi);
		//m_PrimaryCamera.SetAspectRatio(eAspectType::Ratio_16x9);
		m_PrimaryCamera.SetAspectRatio(eAspectType::FullWindow, 1.0f);
		m_PrimaryCamera.SetNearPlane(0.1f);
		m_PrimaryCamera.SetFarPlane(1000.0f);

		float mTheta = 1.5f * Pi;
		float mPhi = Pi / 4.0f; // 45 градусов
		float mRadius = 5.0f;

		float x = mRadius * std::sin(mPhi) * std::cos(mTheta);
		float z = mRadius * std::sin(mPhi) * std::sin(mTheta);
		float y = mRadius * std::cos(mPhi);
		//float x = 0.0f;
		//float y = 0.0f;
		//float z = -5.0f;

		m_PrimaryCamera.SetPosition(Vector4(x, y, z, 1.0f));
		//m_PrimaryCamera.SetPosition(Vector4(0, 0, -5, 1.0f));
		m_PrimaryCamera.SetTarget(Vector4(0.0f, 0.0f, 0.0f, 0.0f));
		m_PrimaryCamera.SetUp(Vector4(0.0f, 1.0f, 0.0f, 0.0f));
	}

	export Scene::~Scene()
	{
	}

	void Scene::Add(std::shared_ptr<SceneEntity> entity)
	{
		m_Entity = entity;
	}
}