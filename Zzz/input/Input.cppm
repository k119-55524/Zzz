
export module Input;

import IAppWin;

using namespace zzz::core;

namespace zzz::input
{
	export class Input final
	{
		public:
			Input() = delete;
			~Input() = default;
			Input(const std::shared_ptr<IAppWin> appWindow)
			{
				ensure(appWindow, ">>>>> [Input::Input()]. Application window cannot be null.");
				Initialize(appWindow);
			}

	private:

		void Initialize(const std::shared_ptr<IAppWin> appWindow);
	};

	void Input::Initialize(const std::shared_ptr<IAppWin> appWindow)
	{
	}
}