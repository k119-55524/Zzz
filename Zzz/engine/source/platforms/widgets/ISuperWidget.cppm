#include "pch.h"
export module ISuperWidget;

import result;
import zSize2D;
import SWSettings;

using namespace zzz;
using namespace zzz::result;

export namespace zzz::platforms
{
	export enum e_TypeWinAppResize : zU32
	{
		eShow,
		eHide,
		eResize
	};

	class ISuperWidget
	{
	public:
		ISuperWidget() = delete;
		ISuperWidget(ISuperWidget&) = delete;
		ISuperWidget(ISuperWidget&&) = delete;

		ISuperWidget(const std::function<void(const zSize2D<>& size, e_TypeWinAppResize resType)> _resizeWindows);
		virtual ~ISuperWidget() = 0;

		virtual zResult<> Initialize(std::shared_ptr<SWSettings> _setting) final;
		inline const zSize2D<>& GetWinSize() const noexcept { return winSize; };

	protected:
		std::shared_ptr<SWSettings> settings;
		zSize2D<> winSize;
		std::function<void(const zSize2D<>& size, e_TypeWinAppResize resType)> resizeWindows;

		virtual zResult<> Init() = 0;
	};

	ISuperWidget::ISuperWidget(const std::function<void(const zSize2D<>& size, e_TypeWinAppResize resType)> _resizeWindows) :
		resizeWindows{ _resizeWindows },
		winSize{ 0, 0 }
	{
		ensure(resizeWindows);
	}

	ISuperWidget::~ISuperWidget()
	{
	}

	zResult<> ISuperWidget::Initialize(std::shared_ptr<SWSettings> _setting)
	{
		settings = _setting;
		return Init();
	}
}