
export module IBehavior;

import Transform;

namespace zzz
{
	export class IBehavior
	{
	public:
		IBehavior() : m_Transform{ nullptr } {};
		virtual ~IBehavior() noexcept {}

		virtual void OnUpdate(float deltaTime) = 0;

	protected:
		Transform* m_Transform = nullptr;

	private:
		void SetTransform(Transform& transform) { m_Transform = &transform; }
		friend class SceneEntity;
	};
}