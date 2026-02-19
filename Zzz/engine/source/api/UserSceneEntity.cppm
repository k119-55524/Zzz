export module UserSceneEntity;

import std;
import Math;
import Ensure;
import Result;
import Transform;
import IBehavior;
import SceneEntity;

using namespace zzz::math;

namespace zzz
{
	export class UserSceneEntity final
	{
	public:
		UserSceneEntity() = delete;
		~UserSceneEntity() = default;
		UserSceneEntity(std::shared_ptr<SceneEntity> entity);

		template<typename T, typename... Args>
		Result<> SetScript(Args&&... args)
		{
			static_assert(std::is_base_of_v<IBehavior, T>, "T must derive from IBehavior");
			return m_Entity->SetScript<T>(std::forward<Args>(args)...);
		}

	private:
		std::shared_ptr<SceneEntity> m_Entity;
	};

	inline UserSceneEntity::UserSceneEntity(std::shared_ptr<SceneEntity> entity) :
		m_Entity{ std::move(entity) }
	{
		ensure(m_Entity, "SceneEntity cannot be null.");
	}
}