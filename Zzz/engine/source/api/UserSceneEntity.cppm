
export module UserSceneEntity;

import Math;
import Transform;
import SceneEntity;

using namespace zzz::math;

namespace zzz
{
	export class UserSceneEntity final
	{
	public:
		UserSceneEntity() = delete;
		~UserSceneEntity() = default;
		explicit UserSceneEntity(const std::shared_ptr<SceneEntity> entity) :
			m_Entity{ entity }
		{
			ensure(m_Entity, ">>>>> [UserSceneEntity::UserSceneEntity(...)]. SceneEntity cannot be null.");
		}

		inline Transform& GetTransform() noexcept { return m_Entity->GetTransform(); }

	private:
		std::shared_ptr<SceneEntity> m_Entity;
	};
}