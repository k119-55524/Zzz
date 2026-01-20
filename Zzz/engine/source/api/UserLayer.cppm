
export module UserLayer;

export import Result;
export import Layer3D;

namespace zzz
{
	export class UserLayer
	{
	public:
		UserLayer() = delete;
		~UserLayer() = default;
		explicit UserLayer(const std::shared_ptr<Layer3D> layer) :
			m_RenderLayer3D{ layer }
		{
		}

	private:
		std::shared_ptr<Layer3D> m_RenderLayer3D;
	};
}