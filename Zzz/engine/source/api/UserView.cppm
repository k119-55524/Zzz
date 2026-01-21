
export module UserView;

import View;
import Result;
import UserLayer3D;

export namespace zzz
{
	class UserView
	{
	public:
		UserView() = delete;
		~UserView() = default;

		explicit UserView(const std::shared_ptr<View> mainView) :
			m_MainView{ mainView }
		{
			ensure(m_MainView, ">>>>> [UserView::UserView(...)]. Main view cannot be null.");
		}

		inline Result<std::shared_ptr<UserLayer3D>> AddLayer_3D()
		{
			if (!m_MainView)
				return Unexpected(eResult::failure, L">>>>> [UserViewþAddLayer_3D()]. m_MainView is not initialized.");

			return m_MainView->AddLayer_3D();
		}

	private:
		std::shared_ptr<View> m_MainView;
	};
}