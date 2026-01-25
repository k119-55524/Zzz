
export module MouseWin;

#if defined(ZPLATFORM_MSWINDOWS)

import IMouse;
import IAppWin;
import AppWin_MSWin;

using namespace zzz::core;

namespace zzz::input
{
	export class MouseWin final : public IMouse
	{
	public:
		explicit MouseWin(const std::shared_ptr<AppWin_MSWin> window) :
			m_Window{ window }
		{
			ensure(window, ">>>>> [Input::Input()]. Application window cannot be null.");
			Initialize();
		}
		~MouseWin() override {};

	protected:
		void Initialize();

		std::shared_ptr<AppWin_MSWin> m_Window;
	};

	void MouseWin::Initialize()
	{
		m_Window->OnMouseEnter += std::bind(&MouseWin::OnMouseEnter, this);
		m_Window->OnMouseLeave += std::bind(&MouseWin::OnMouseLeave, this);
	}
}
#endif // ZPLATFORM_MSWINDOWS