#include "pch.h"
export module SuperWidgetSettings;

import result;

using namespace zzz;
using namespace zzz::result;

namespace zzz::platforms
{
	enum eSWState : zU32
	{
		eInitNot,	// Готов к инициализации
		eInitOK,	// Инициализированн
	};
}

export namespace zzz::platforms
{
	class SuperWidgetSettings
	{
	public:
		SuperWidgetSettings();
		~SuperWidgetSettings();

		zResult<> Initialize();
		void Reset();

	private:
		std::mutex initMutex;
		eSWState swState;

		zResult<> LoadSettings();
	};

	SuperWidgetSettings::SuperWidgetSettings() :
				swState{ eSWState::eInitNot }
	{
	}

	SuperWidgetSettings::~SuperWidgetSettings()
	{
		Reset();
	}

	void SuperWidgetSettings::Reset()
	{
		swState = eSWState::eInitNot; // Сброс состояния
	}

	zResult<> SuperWidgetSettings::Initialize()
	{
		std::lock_guard<std::mutex> lock(initMutex);
		if (swState != eSWState::eInitNot)
			return Unexpected(eResult::failure, L">>>>> [SuperWidgetSettings::Initialize()]. Re-initialization is not allowed.");

		swState = eSWState::eInitOK;

		return LoadSettings();
	}

	zResult<> SuperWidgetSettings::LoadSettings()
	{
		return {};
	}
}