#include "pch.h"
export module ISuperWidget;

import result;
import zSize2D;
import SWSettings;

using namespace zzz;
using namespace zzz::result;

namespace zzz::platforms
{
	export enum eSuperWidgetState : zU32
	{
		eInitNot = 0,
		eInitOK = 1,
	};
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

		ISuperWidget(SWSettings& _settings, const std::function<void(const zSize2D<>& size, e_TypeWinAppResize resType)> _resizeWindows);
		virtual ~ISuperWidget() = 0;

		virtual zResult<> Initialize() final;
		inline const zSize2D<>& GetWinSize() const noexcept { return winSize; };

	protected:
		SWSettings& settings;
		zSize2D<> winSize;
		std::function<void(const zSize2D<>& size, e_TypeWinAppResize resType)> resizeWindows;

		virtual zResult<> Init() = 0;

	private:
		eSuperWidgetState initState;
		std::mutex stateMutex;
	};

	ISuperWidget::ISuperWidget(SWSettings& _settings, const std::function<void(const zSize2D<>& size, e_TypeWinAppResize resType)> _resizeWindows) :
		settings{ _settings },
		resizeWindows{ _resizeWindows },
		winSize{ 0, 0 },
		initState{ eSuperWidgetState::eInitNot }
	{
		ensure(resizeWindows);
	}

	ISuperWidget::~ISuperWidget()
	{
	}

	zResult<> ISuperWidget::Initialize()
	{
		std::lock_guard<std::mutex> lock(stateMutex);
		if (initState != eSuperWidgetState::eInitNot)
			return Unexpected(eResult::failure, L">>>>> [ISuperWidget.Initialize()]. Re-initialization is not allowed.");

		return Init().and_then([&]() 
			{
				initState = eSuperWidgetState::eInitOK;
			});
	}
}