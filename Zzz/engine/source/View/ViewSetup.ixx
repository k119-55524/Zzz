
export module ViewSetup;

export import RenderArea;

using namespace zzz::core;

namespace zzz
{
	export class ViewSetup
	{
	public:
		ViewSetup() = delete;
		ViewSetup(eAspectType aspect, Size2D<float>& size, float minDepth, float maxDepth) :
			m_RenderArea(aspect, size, minDepth, maxDepth)
		{
		}

	protected:
		RenderArea m_RenderArea;
	};
}