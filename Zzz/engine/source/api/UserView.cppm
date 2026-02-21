
export module UserView;

import View;
import Result;
import Ensure;
import UserLayer3D;

export namespace zzz
{
	class UserView
	{
	public:
		UserView() = delete;
		~UserView() = default;

		explicit UserView(const std::shared_ptr<View> view) :
			m_View{ view }
		{
			ensure(m_View, "Main view cannot be null.");
		}

		inline Result<std::shared_ptr<UserLayer3D>> AddLayer_3D() { return m_View->AddLayer_3D(); }

	private:
		std::shared_ptr<View> m_View;
	};
}