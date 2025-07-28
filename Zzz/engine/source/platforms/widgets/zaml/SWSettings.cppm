#include "pch.h"
export module SWSettings;

import result;
import iozaml;


using namespace zzz;
using namespace zzz::zaml;
using namespace zzz::result;

namespace zzz::platforms
{
	enum eSWState : zU32
	{
		eInitNot = 0,	// Готов к инициализации
		eInitOK = 1,	// Инициализированн
	};
}

export namespace zzz::platforms
{
	class SWSettings
	{
	public:
		SWSettings();
		SWSettings(const SWSettings&) = delete;
		SWSettings& operator=(const SWSettings&) = delete;

		~SWSettings();

		zResult<> Initialize(std::wstring _filePath);
		void Reset();

	private:
		std::mutex initMutex;
		eSWState swState;
		std::wstring filePath;
		zamlNode* result = nullptr;

		zResult<> LoadSettings(std::wstring _filePath);
	};

	SWSettings::SWSettings() :
				swState{ eSWState::eInitNot }
	{
	}

	SWSettings::~SWSettings()
	{
		Reset();
	}

	void SWSettings::Reset()
	{
		if (result)
			delete result;

		swState = eSWState::eInitNot; // Сброс состояния
	}

	zResult<> SWSettings::Initialize(std::wstring _filePath)
	{
		std::lock_guard<std::mutex> lock(initMutex);
		if (swState != eSWState::eInitNot)
			return Unexpected(eResult::failure, L">>>>> [SuperWidgetSettings::Initialize()]. Re-initialization is not allowed.");

		if (LoadSettings(_filePath))
			swState = eSWState::eInitOK;

		return {};
	}

	zResult<> SWSettings::LoadSettings(std::wstring _filePath)
	{
		ioZaml loader;
		auto res = loader.LoadFromFile(_filePath)
			.and_then([this](const zamlNode& node)
			{
				if (result)
					delete result; // Освобождаем предыдущий результат

				result = new zamlNode(node); // Сохраняем новый результат

				return zResult<>();
			})
			.or_else([](auto error) { return error; });

		return res;
	}
}