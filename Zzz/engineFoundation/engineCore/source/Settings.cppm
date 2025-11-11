
#include "pch.h"

export module Settings;

import result;
import iozaml;
import StrConvert;

export namespace zzz::engineCore
{
	export class Settings
	{
	public:
		Settings() = delete;
		Settings(std::wstring _filePath);
		Settings(const Settings&) = delete;
		Settings(Settings&&) = delete;

		Settings& operator=(const Settings&) = delete;

		template<typename T, typename... Path>
		result<T> GetParam(Path&&... pathAndParamName) const
		{
			if (!m_settings)
				return Unexpected(eResult::not_initialized, L">>>>> [Settings.GetParam(...)] Settings not loaded");

			constexpr size_t argCount = sizeof...(Path);
			static_assert(argCount >= 1, "At least one argument (paramName) is required");

			const zamlNode* node = m_settings.get();

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
					return Unexpected(eResult::not_found, L">>>>> [Settings.GetParam(...)] Tag '" + tag + L"' not found");
				}
			}

			// Ищем атрибут
			auto attr = node->GetAttribute(paramName);
			if (!attr)
			{
				return Unexpected(eResult::not_found, L">>>>> [Settings.GetParam(...)] Parameter '" + paramName + L"' not found");
			}

			// Преобразуем значение
			return ConvertValue<T>(*attr.value());
		}

	private:
		std::wstring filePath;
		std::unique_ptr<zamlNode> m_settings;

		result<> LoadSettings();
	};

	Settings::Settings(std::wstring _filePath) :
		filePath{ std::move(_filePath) },
		m_settings{}
	{
		if (filePath.empty())
			throw_runtime_error("Empty filePath");

		auto res = LoadSettings();
		if (!res)
			throw_runtime_error(std::format("Failed to load Settings from file: {}.\n{}", wstring_to_string(filePath), wstring_to_string(res.error().getMessage())));
	}

	result<> Settings::LoadSettings()
	{
		ioZaml loader;
		auto res = loader.LoadFromFile(filePath)
			.and_then([this](const zamlNode& node)
				{
					m_settings = std::make_unique<zamlNode>(node);
					return result<>();
				})
			.or_else([](auto error) { return error; });

		return res;
	}
}