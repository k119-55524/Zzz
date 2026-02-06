
module IBehavior;

import Transform;
import IBehavior;
import SceneEntity;

namespace zzz
{
	[[nodiscard]] Transform& IBehavior::GetTransform() noexcept
	{
		return m_Entity->GetTransform();
	}

	void IBehavior::SetEntity(std::shared_ptr<SceneEntity> entity)
	{
		m_Entity = std::move(entity);
	}
}