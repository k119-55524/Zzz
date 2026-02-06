
export module IBehavior;

import Transform;
//import SceneEntity;

namespace zzz
{
	export class SceneEntity;

	export class IBehavior
	{
	public:
		IBehavior(){};
		virtual ~IBehavior() noexcept {}

		virtual void OnUpdate(float deltaTime) = 0;

	protected:
		std::shared_ptr<SceneEntity> m_Entity;

		[[nodiscard]] Transform& GetTransform() noexcept;

	private:
		void SetEntity(const std::shared_ptr<SceneEntity> entity);
		friend class SceneEntity;
	};
}