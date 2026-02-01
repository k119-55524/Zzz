
export module Keyboard_MSWin;

#if defined(ZPLATFORM_MSWINDOWS)

import IAppWin;
import IKeyboard;
import AppWin_MSWin;

using namespace zzz::core;

namespace zzz::input
{
	export class Keyboard_MSWin final : public IKeyboard
	{
	public:
		explicit Keyboard_MSWin(const std::shared_ptr<AppWin_MSWin> window) :
			m_Window{ window }
		{
			ensure(window, "Application window cannot be null.");
			Initialize();
		}
		~Keyboard_MSWin() override {};

	protected:
		void Initialize();

		std::shared_ptr<AppWin_MSWin> m_Window;
	};

	void Keyboard_MSWin::Initialize()
	{
		m_Window->OnKeyStateChanged += std::bind(&Keyboard_MSWin::OnKeyStateChanged, this, std::placeholders::_1, std::placeholders::_2);
	}
}
#endif // ZPLATFORM_MSWINDOWS