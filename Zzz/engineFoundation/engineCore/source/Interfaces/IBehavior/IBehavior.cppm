
export module IBehavior;

namespace zzz::core
{
	export class IBehavior
	{
	public:
		IBehavior() = default;
		virtual ~IBehavior() noexcept
		{
		}

		virtual void OnUpdate(float deltaTime) = 0;
	};
}