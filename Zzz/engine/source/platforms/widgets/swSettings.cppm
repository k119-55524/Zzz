#include "pch.h"
export module swSettings;

import result;
import iozaml;
import StringConverters;


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
	class swSettings
	{
	public:
		swSettings();
		swSettings(const swSettings&) = delete;
		swSettings& operator=(const swSettings&) = delete;

		~swSettings();

		zResult<> Get(std::wstring _filePath);
		void Reset();

		template<typename T, typename... Path>
		zResult<T> GetParam(Path&&... pathAndParamName) const
		{
			if (!settings)
				return Unexpected(eResult::not_initialized, L">>>>> [swSettings.GetParam(...)] Settings not loaded");

			constexpr size_t argCount = sizeof...(Path);
			static_assert(argCount >= 1, "At least one argument (paramName) is required");

			const zamlNode* node = settings.get();

			// Помещаем аргументы в массив
			std::array<std::wstring, argCount> args = { std::wstring(std::forward<Path>(pathAndParamName))... };

			// Последний аргумент — это имя параметра
			const std::wstring& paramName = args.back();

			// Остальные — путь до нужного узла
			for (size_t i = 0; i < args.size() - 1; ++i)
			{
				const std::wstring& tag = args[i];
				bool found = false;

				for (const auto& child : node->children)
				{
					if (child.name == tag)
					{
						node = &child;
						found = true;
						break;
					}
				}

				if (!found)
				{
					return Unexpected(eResult::not_found, L">>>>> [swSettings.GetParam(...)] Tag '" + tag + L"' not found");
				}
			}

			// Ищем атрибут
			auto attr = node->GetAttribute(paramName);
			if (!attr)
			{
				return Unexpected(eResult::not_found, L">>>>> [swSettings.GetParam(...)] Parameter '" + paramName + L"' not found");
			}

			// Преобразуем значение
			return ConvertValue<T>(*attr.value());
		}


	private:
		std::mutex initMutex;
		eSWState swState;
		std::wstring filePath;
		std::unique_ptr<zamlNode> settings;

		zResult<> LoadSettings(std::wstring _filePath);
	};

	swSettings::swSettings() :
		settings{},
		swState{ eSWState::eInitNot }
	{
	}

	swSettings::~swSettings()
	{
		Reset();
	}

	void swSettings::Reset()
	{
		settings.reset();
		swState = eSWState::eInitNot; // Сброс состояния
	}

	zResult<> swSettings::Get(std::wstring _filePath)
	{
		std::lock_guard<std::mutex> lock(initMutex);
		if (swState != eSWState::eInitNot)
			return Unexpected(eResult::failure, L">>>>> [SuperWidgetSettings::Get()]. Re-initialization is not allowed.");

		auto res = LoadSettings(_filePath)
			.and_then([this](){ swState = eSWState::eInitOK; });

		return res;
	}

	zResult<> swSettings::LoadSettings(std::wstring _filePath)
	{
		ioZaml loader;
		auto res = loader.LoadFromFile(_filePath)
			.and_then([this](const zamlNode& node)
			{
				settings = std::make_unique<zamlNode>(node);
				return zResult<>();
			})
			.or_else([](auto error) { return error; });

		return res;
	}
}