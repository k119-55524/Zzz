
module IBehavior;

import Input;
import Transform;
import IBehavior;
import SceneEntity;

using namespace zzz::input;

namespace zzz
{
	[[nodiscard]] Transform& IBehavior::GetTransform() noexcept
	{
		auto entity = m_Entity.lock();
		ensure(entity, "SceneEntity is no longer valid.");
		return entity->GetTransform();
	}

	std::shared_ptr<Input> IBehavior::GetInput() const noexcept
	{
		auto entity = m_Entity.lock();
		ensure(entity, "SceneEntity is no longer valid.");
		return entity->GetInput();
	}
}