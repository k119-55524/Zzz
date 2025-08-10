#include "pch.h"
export module zViewSettings;

import result;
import iozaml;
import strConver;

using namespace zzz::zaml;

export namespace zzz
{
	class zViewSettings
	{
	public:
		zViewSettings() = delete;
		zViewSettings(std::wstring _filePath);
		zViewSettings(const zViewSettings&) = delete;
		zViewSettings(zViewSettings&&) = delete;

		zViewSettings& operator=(const zViewSettings&) = delete;

		template<typename T, typename... Path>
		result<T> GetParam(Path&&... pathAndParamName) const
		{
			if (!settings)
				return Unexpected(eResult::not_initialized, L">>>>> [zViewSettings.GetParam(...)] Settings not loaded");

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
					return Unexpected(eResult::not_found, L">>>>> [zViewSettings.GetParam(...)] Tag '" + tag + L"' not found");
				}
			}

			// Ищем атрибут
			auto attr = node->GetAttribute(paramName);
			if (!attr)
			{
				return Unexpected(eResult::not_found, L">>>>> [zViewSettings.GetParam(...)] Parameter '" + paramName + L"' not found");
			}

			// Преобразуем значение
			return ConvertValue<T>(*attr.value());
		}

	private:
		std::wstring filePath;
		std::unique_ptr<zamlNode> settings;

		result<> LoadSettings();
	};

	zViewSettings::zViewSettings(std::wstring _filePath) :
		filePath{ std::move(_filePath) },
		settings{}
	{
		if (filePath.empty())
			throw_runtime_error("Empty filePath");

		auto res = LoadSettings();
		if (!res)
			throw_runtime_error(std::format( "Failed to load settings from file: {}.\n{}", wstring_to_string(filePath), wstring_to_string(res.error().getMessage())));
	}

	result<> zViewSettings::LoadSettings()
	{
		ioZaml loader;
		auto res = loader.LoadFromFile(filePath)
			.and_then([this](const zamlNode& node)
			{
				settings = std::make_unique<zamlNode>(node);
				return result<>();
			})
			.or_else([](auto error) { return error; });

		return res;
	}
}