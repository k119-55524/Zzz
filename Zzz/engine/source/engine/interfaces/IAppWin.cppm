#include "pch.h"
export module IAppWin;

import result;
import zViewSettings;

using namespace zzz::platforms;

namespace zzz
{
	class zView;
}

namespace zzz
{
	export class IAppWin abstract
	{
	public:
		IAppWin() = delete;
		IAppWin(const IAppWin&) = delete;
		IAppWin(IAppWin&&) = delete;
		IAppWin& operator=(const IAppWin&) = delete;
		IAppWin& operator=(IAppWin&&) = delete;

		explicit IAppWin(std::shared_ptr<const zViewSettings> settings);

		virtual ~IAppWin() = default;

	protected:
		const std::shared_ptr<const zViewSettings> settings;
		[[nodiscard]] virtual result<> Initialize() = 0;
		friend class zzz::zView;
	};

	IAppWin::IAppWin(std::shared_ptr<const zViewSettings> settings)
		: settings{ settings }
	{
		ensure(settings, ">>>>> [IAppWin::IAppWin()]. Settings cannot be null.");
	}
}