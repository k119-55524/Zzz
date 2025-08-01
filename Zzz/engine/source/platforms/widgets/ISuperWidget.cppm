#include "pch.h"
export module ISuperWidget;

import result;
import zSize2D;
import IMainLoop;
import swSettings;
import StringConverters;

using namespace zzz;
using namespace zzz::result;

namespace zzz
{
	class engine;
}

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

		ISuperWidget(std::shared_ptr<swSettings> _settings, const std::function<void(const zSize2D<>& size, e_TypeWinAppResize resType)> _callbackResizeSW);
		virtual ~ISuperWidget() = 0;

		inline const zSize2D<>& GetWinSize() const noexcept { return winSize; };

	protected:
		virtual zResult<> _Initialize() = 0;
		std::shared_ptr<swSettings> settings;
		zSize2D<> winSize;

		std::shared_ptr<IMainLoop> mainLoop;
		std::function<void(const zSize2D<>& size, e_TypeWinAppResize resType)> callbackResizeSW;

	private:
		zResult<> Initialize();
		friend class zzz::engine;
	};

	ISuperWidget::ISuperWidget(std::shared_ptr<swSettings> _settings, const std::function<void(const zSize2D<>& size, e_TypeWinAppResize resType)> _callbackResizeSW) :
		settings{ _settings },
		callbackResizeSW{ _callbackResizeSW },
		winSize{ 0, 0 }
	{
		ensure(settings);
		ensure(callbackResizeSW);
	}

	ISuperWidget::~ISuperWidget()
	{
	}

	zResult<> ISuperWidget::Initialize()
	{
		return _Initialize();
	}
}