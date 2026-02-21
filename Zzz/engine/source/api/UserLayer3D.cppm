
export module UserLayer3D;

import Result;
import Ensure;
import Layer3D;
import UserScene;

namespace zzz
{
	export class UserLayer3D
	{
	public:
		UserLayer3D() = delete;
		~UserLayer3D() = default;
		explicit UserLayer3D(const std::shared_ptr<Layer3D> layer) :
			m_Layer{ layer }
		{
			ensure(m_Layer, "Layer3D cannot be null.");
		}

		Result<std::shared_ptr<UserScene>> AddScene();

	private:
		std::shared_ptr<Layer3D> m_Layer;
	};

	Result<std::shared_ptr<UserScene>> UserLayer3D::AddScene()
	{
		auto resScene = m_Layer->AddScene();
		if (!resScene)
			return Unexpected(eResult::failure, resScene.error().getMessage());

		std::shared_ptr<Scene> scene = resScene.value();
		std::shared_ptr<UserScene> userScene = safe_make_shared<UserScene>(scene);

		return userScene;
	}
}