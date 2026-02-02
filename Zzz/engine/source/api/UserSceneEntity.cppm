
export module UserSceneEntity;

import Math;
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
		explicit UserSceneEntity(const std::shared_ptr<SceneEntity> entity) :
			m_Entity{ entity }
		{
			ensure(m_Entity, ">>>>> [UserSceneEntity::UserSceneEntity(...)]. SceneEntity cannot be null.");
		}

		inline Transform& GetTransform() noexcept { return m_Entity->GetTransform(); }

		template<typename T, typename... Args>
		Result<> SetScript(Args&&... args)
		{
			static_assert(std::is_base_of_v<IBehavior, T>, "T must derive from IBehavior");
			return m_Entity->SetScript<T>(std::forward<Args>(args)...);
		}

	private:
		std::shared_ptr<SceneEntity> m_Entity;
	};
}