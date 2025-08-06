#include "pch.h"
export module ISuperWidget;

import zEvent;
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

		explicit ISuperWidget(std::shared_ptr<swSettings> _settings);
		virtual ~ISuperWidget() = 0;

		inline const zSize2D<>& GetWinSize() const noexcept { return winSize; };

		zEvent<zSize2D<>, e_TypeWinAppResize> onResize;

	protected:
		virtual zResult<> Initialize() = 0;
		friend class zzz::engine;

		std::shared_ptr<swSettings> settings;
		zSize2D<> winSize;

		virtual void OnUpdate() = 0;
	};

	ISuperWidget::ISuperWidget(std::shared_ptr<swSettings> _settings) :
		settings{ _settings },
		winSize{ 0, 0 }
	{
		ensure(settings);
	}

	ISuperWidget::~ISuperWidget()
	{
	}
}