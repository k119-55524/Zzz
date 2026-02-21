
export module Mouse_MSWin;

#if defined(ZPLATFORM_MSWINDOWS)

import Ensure;
import IMouse;
import IAppWin;
import AppWin_MSWin;

using namespace zzz::core;

namespace zzz::input
{
	export class Mouse_MSWin final : public IMouse
	{
	public:
		explicit Mouse_MSWin(const std::shared_ptr<AppWin_MSWin> window) :
			m_Window{ window }
		{
			ensure(window, "Application window cannot be null.");
			Initialize();
		}
		~Mouse_MSWin() override {};

	protected:
		void Initialize();

		std::shared_ptr<AppWin_MSWin> m_Window;
	};

	void Mouse_MSWin::Initialize()
	{
		m_Window->OnMouseEnter += std::bind(&Mouse_MSWin::OnMouseEnter, this, std::placeholders::_1);
		m_Window->OnMouseDelta += std::bind(&Mouse_MSWin::OnMouseDelta, this, std::placeholders::_1, std::placeholders::_2);
		m_Window->OnMouseButtonsChanged += std::bind(&Mouse_MSWin::OnMouseButtonsChanged, this, std::placeholders::_1, std::placeholders::_2);
		m_Window->OnMouseWheelVertical += std::bind(&Mouse_MSWin::OnMouseWheelVertical, this, std::placeholders::_1);
		m_Window->OnMouseWheelHorizontal += std::bind(&Mouse_MSWin::OnMouseWheelHorizontal, this, std::placeholders::_1);
	}
}
#endif // ZPLATFORM_MSWINDOWS