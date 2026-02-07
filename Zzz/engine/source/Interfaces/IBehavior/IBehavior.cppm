
export module IBehavior;

import Input;
import Transform;

using namespace zzz::input;

namespace zzz
{
	export class SceneEntity;

	export class IBehavior
	{
	public:
		IBehavior(const std::shared_ptr<SceneEntity> entity) :
			m_Entity{ entity }
		{
		}
		virtual ~IBehavior() noexcept {}

		[[nodiscard]] std::shared_ptr<Input> GetInput() const noexcept;

		virtual void OnUpdate(float deltaTime) = 0;

	protected:
		const std::weak_ptr<SceneEntity> m_Entity;

		[[nodiscard]] Transform& GetTransform() noexcept;
	};
}