
export module UserScene;

import Scene;
import Result;
import Transform;
import StrConvert;
import SceneEntity;
import UserSceneEntity;

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

		Result<std::shared_ptr<UserSceneEntity>> AddColorBox(Transform& transform)
		{
			std::shared_ptr<UserSceneEntity> userEntity;

			try
			{
				std::shared_ptr<SceneEntity> entity;
				Result<std::shared_ptr<SceneEntity>> resEntity = m_Scene->AddColorBox(transform);
				if (!resEntity)
					return resEntity.error();

				entity = resEntity.value();
				entity->SetTransform(transform);
				userEntity = safe_make_shared<UserSceneEntity>(entity);
			}
			catch (const std::exception& e)
			{
				return Unexpected(eResult::exception, string_to_wstring(e.what()).value_or(L"Unknown exception occurred."));
			}

			return userEntity;
		}

	private:
		std::shared_ptr<Scene> m_Scene;
	};
}