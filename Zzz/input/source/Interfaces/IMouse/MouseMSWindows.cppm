
export module MouseMSWindows;

#if defined(ZPLATFORM_MSWINDOWS)

import IMouse;
import IAppWin;
import AppWin_MSWin;

using namespace zzz::core;

namespace zzz::input
{
	export class MouseMSWindows final : public IMouse
	{
	public:
		explicit MouseMSWindows(const std::shared_ptr<AppWin_MSWin> window) :
			m_Window{ window }
		{
			ensure(window, ">>>>> [Input::Input()]. Application window cannot be null.");
			Initialize();
		}
		~MouseMSWindows() override {};

	protected:
		void Initialize();

		std::shared_ptr<AppWin_MSWin> m_Window;
	};

	void MouseMSWindows::Initialize()
	{
		m_Window->OnMouseEnter += std::bind(&MouseMSWindows::OnMouseEnter, this, std::placeholders::_1);
	}
}
#endif // ZPLATFORM_MSWINDOWS