
export module Transform;

export import Math;

using namespace zzz::math;

namespace zzz
{
	export class Transform
	{
	public:

	protected:
		Vector3 position{ 0,0,0 };
		Vector3 scale{ 1,1,1 };
	};
}