#include "pch.h"
export module ISuperWidget;

import zEvent;
import result;
import zSize2D;
import strConver;
import IMainLoop;
import zViewSettings;

using namespace zzz;

namespace zzz
{
	class engine;
}

export namespace zzz::platforms
{
	class ISuperWidget
	{
	public:
		ISuperWidget() = delete;
		ISuperWidget(ISuperWidget&) = delete;
		ISuperWidget(ISuperWidget&&) = delete;

		explicit ISuperWidget(std::shared_ptr<zViewSettings> _settings);
		virtual ~ISuperWidget() = 0;

		inline const zSize2D<>& GetWinSize() const noexcept { return winSize; };

		zEvent<zSize2D<>, e_TypeWinResize> onResize;

	protected:
		virtual result<> Initialize() = 0;
		friend class zzz::engine;

		std::shared_ptr<zViewSettings> settings;
		zSize2D<> winSize;

		virtual void OnUpdate() = 0;
	};

	ISuperWidget::ISuperWidget(std::shared_ptr<zViewSettings> _settings) :
		settings{ _settings },
		winSize{ 0, 0 }
	{
		ensure(settings);
	}

	ISuperWidget::~ISuperWidget()
	{
	}
}