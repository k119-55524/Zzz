#include "pch.h"
export module ISuperWidget;

import zSize2D;
import IWidgetSettings;

using namespace zzz;

namespace zzz::platforms
{
	class ISuperWidget
	{
	public:
		ISuperWidget() = delete;
		ISuperWidget(ISuperWidget&) = delete;
		ISuperWidget(ISuperWidget&&) = delete;

		ISuperWidget(std::function<void(const zSize2D<>& size)> _resizeWindows);
		virtual ~ISuperWidget() = 0;

		virtual void Initialize(const std::shared_ptr<IWidgetSettings> userGS) = 0;
		inline const zSize2D<>& GetWinSize() const noexcept { return winSize; };

	protected:
		zSize2D<> winSize;
		std::function<void(const zSize2D<>& size)> resizeWindows;
	};

	ISuperWidget::ISuperWidget(std::function<void(const zSize2D<>& size)> _resizeWindows) :
		resizeWindows{ _resizeWindows },
		winSize{ 0, 0 }
	{
	}

	ISuperWidget::~ISuperWidget()
	{
	}
}