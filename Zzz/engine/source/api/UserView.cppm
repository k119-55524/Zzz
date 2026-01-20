
export module UserView;

export import View;
export import Result;

namespace zzz
{
	export class UserView
	{
	public:
		UserView() = delete;
		~UserView() = default;

		explicit UserView(const std::shared_ptr<View> mainView) :
			m_MainView{ mainView }
		{
		}

		inline Result<Result<std::shared_ptr<UserLayer>>> AddLayer_3D()
		{
			if (!m_MainView)
				return Unexpected(eResult::failure, L">>>>> [UserViewþAddLayer_3D()]. m_MainView is not initialized.");

			return m_MainView->AddLayer_3D();
		}

	private:
		std::shared_ptr<View> m_MainView;
	};
}