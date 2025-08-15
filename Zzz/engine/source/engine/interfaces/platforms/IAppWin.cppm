#include "pch.h"
export module IAppWin;

import result;
import event;
import size2D;
import settings;

using namespace zzz::platforms;

namespace zzz
{
	class view;
}

export namespace zzz
{
	export class IAppWin abstract
	{
	public:
		IAppWin() = delete;
		IAppWin(const IAppWin&) = delete;
		IAppWin(IAppWin&&) = delete;
		IAppWin& operator=(const IAppWin&) = delete;
		IAppWin& operator=(IAppWin&&) = delete;

		explicit IAppWin(std::shared_ptr<const settings> settings);

		virtual ~IAppWin() = default;

		event<size2D<>, e_TypeWinResize> onResize;

		const size2D<> GetWinSize() const noexcept { return winSize; }

	protected:
		const std::shared_ptr<const settings> m_settings;
		size2D<> winSize;

		[[nodiscard]] virtual result<> Initialize() = 0;
		friend class zzz::view;
	};

	IAppWin::IAppWin(std::shared_ptr<const settings> settings)
		: m_settings{ settings },
		winSize{ 0, 0 }
	{
		ensure(settings, ">>>>> [IAppWin::IAppWin()]. Settings cannot be null.");
	}
}