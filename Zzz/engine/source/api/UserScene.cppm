
export module UserScene;

import Scene;
import Result;

namespace zzz
{
	export class UserScene
	{
	public:
		UserScene() = default;
		explicit UserScene(const std::shared_ptr<Scene> scene) :
			m_Scene{ scene }
		{
			ensure(m_Scene, ">>>>> [UserScene::UserScene(...)]. Scene cannot be null.");
		}

		~UserScene() = default;

	private:
		std::shared_ptr<Scene> m_Scene;
	};
}