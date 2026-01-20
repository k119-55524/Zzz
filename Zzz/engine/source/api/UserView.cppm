
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

		inline Result<> AddLayer_3D()
		{
			if (!m_MainView)
				return Unexpected(eResult::failure, L">>>>> [UserViewþAddLayer_3D()]. m_MainView is not initialized.");

			return {};
		}

	private:
		std::shared_ptr<View> m_MainView;
	};
}