
export module Transform;

export import Math;

using namespace zzz::math;

namespace zzz
{
	export class Transform
	{
	public:
		Transform() :
			m_Position{ 0, 0, 0 },
			m_Scale{ 1, 1, 1 }
		{
		}

	protected:
		Vector3 m_Position;
		Quaternion m_Rotation;
		Vector3 m_Scale;
	};
}