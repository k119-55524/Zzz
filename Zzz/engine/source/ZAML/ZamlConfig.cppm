
export module ZamlConfig;

import Result;
import ioZaml;
import GAPIConfig;
import StrConvert;
import AppConfig;

using namespace zzz::core;

export namespace zzz
{
	export class ZamlConfig final
	{
		Z_NO_CREATE_COPY(ZamlConfig);

	public:
		ZamlConfig(std::wstring _filePath);
		virtual ~ZamlConfig() = default;

		template<typename T, typename... Path>
		Result<T> GetParam(Path&&... pathAndParamName) const
		{
			if (!m_Settings)
				return Unexpected(eResult::not_initialized, L">>>>> [Settings.GetParam(...)] Settings not loaded");

			constexpr size_t argCount = sizeof...(Path);
			static_assert(argCount >= 1, "At least one argument (paramName) is required");

			const zamlNode* node = m_Settings.get();

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
		std::shared_ptr<zamlNode> m_Settings;

		Result<> LoadSettings();
	};

	ZamlConfig::ZamlConfig(std::wstring _filePath) :
		filePath{ std::move(_filePath) },
		m_Settings{}
	{
		if (filePath.empty())
			throw_runtime_error("Empty filePath");

		auto res = LoadSettings();
		if (!res)
			throw_runtime_error(std::format("Failed to load Settings from file: {}.\n{}", wstring_to_string(filePath), wstring_to_string(res.error().getMessage())));
	}

	Result<> ZamlConfig::LoadSettings()
	{
		ioZaml loader;
		auto res = loader.LoadFromFile(filePath)
			.and_then([this](const zamlNode& node)
				{
					m_Settings = std::make_unique<zamlNode>(node);
					return Result<>();
				})
			.or_else([](auto error) { return error; });

		return res;
	}
}