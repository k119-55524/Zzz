
export module MouseNull;

import IMouse;
import IAppWin;

using namespace zzz::core;

namespace zzz::input
{
	export class MouseNull : public IMouse
	{
	public:
		MouseNull() = default;
		~MouseNull() override {};
	};
}