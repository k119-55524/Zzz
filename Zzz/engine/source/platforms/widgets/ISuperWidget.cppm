#include "pch.h"
export module ISuperWidget;

import result;
import zSize2D;
import ISuperWidgetSettings;

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

		ISuperWidget(std::function<void(const zSize2D<>& size, e_TypeWinAppResize resType)> _resizeWindows);
		virtual ~ISuperWidget() = 0;

		virtual zResult<> Initialize(const std::shared_ptr<ISuperWidgetSettings> settings) = 0;
		inline const zSize2D<>& GetWinSize() const noexcept { return winSize; };

	protected:
		zSize2D<> winSize;
		std::function<void(const zSize2D<>& size, e_TypeWinAppResize resType)> resizeWindows;
	};

	ISuperWidget::ISuperWidget(std::function<void(const zSize2D<>& size, e_TypeWinAppResize resType)> _resizeWindows) :
		resizeWindows{ _resizeWindows },
		winSize{ 0, 0 }
	{
		ZASSERT_NULLPTR(resizeWindows);
	}

	ISuperWidget::~ISuperWidget()
	{
	}
}