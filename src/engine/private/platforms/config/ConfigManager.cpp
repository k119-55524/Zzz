
#include <fstream>
#include <system_error>

#include "io/Path.h"
#include "ConfigManager.h"
#include <common/constants.h>
#include <common/serialize/Serializer.h>

using namespace zzz::io;
using namespace zzz::engine;

ConfigManager::ConfigManager(const Path& path) :
	m_Path(path),
	m_IsDirty(true)
{
#if !Z_EDITOR
	Initialize();
#endif
}

void ConfigManager::Initialize()
{
	try
	{
		auto resPath = GetSettingsDirectory();
		if (!resPath)
			THROW_RUNTIME("Не удалось получить каталог настроек: {}.", resPath.error());

		m_ConfigPath = (resPath.value() / c_ConfigFileName)
			.lexically_normal()
			.make_preferred();

		// Далее работаем с файлом
		m_EngineConfig = safe_make_shared<EngineConfig>();

		if (!std::filesystem::exists(m_ConfigPath))
			DOutWarning("Файл конфигурации не найден: {}. Используется конфигурация по умолчанию.", m_ConfigPath.string());

		auto loadResult = LoadConfig(m_ConfigPath);
		if (!loadResult)
		{
			DOutWarning("Не удалось загрузить файл конфигурации: {}. Создаётся конфигурация по умолчанию.", m_ConfigPath.string());
			m_EngineConfig = safe_make_shared<EngineConfig>();
		}
	}
	catch (const std::filesystem::filesystem_error& e)
	{
		DOutException("Ошибка файловой системы: {}. Установка конфигурации по умолчанию.", e.what());
		m_EngineConfig = safe_make_shared<EngineConfig>();

		return;
	}
	catch (const std::exception& e)
	{
		DOutException("Ошибка загрузки конфигурации: {}. Установка конфигурации по умолчанию.", e.what());
		m_EngineConfig = safe_make_shared<EngineConfig>();

		return;
	}
	catch (...)
	{
		DOutException("Неизвестная ошибка загрузки конфигурации. Установка конфигурации по умолчанию.");
		m_EngineConfig = safe_make_shared<EngineConfig>();

		return;
	}

	DOut("Конфигурация успешно десериализована из файла: {}.", m_ConfigPath.string());
}

[[nodiscard]] std::expected<void, std::string> ConfigManager::SaveConfig()
{
#if Z_EDITOR
	return {};
#else
	if (!m_IsDirty)
	{
		DOut("Конфигурация не изменена. Сохранение не требуется.");
		return {};
	}

	try
	{
		std::vector<std::byte> buffer;
		if (auto res = m_Serializer.Serialize(buffer, *m_EngineConfig); !res)
			return UNEXPECTED("Не удалось сериализовать конфигурацию: {}.", res.error());

		std::error_code ec;
		std::filesystem::create_directories(m_ConfigPath.parent_path(), ec);
		if (ec)
			return UNEXPECTED("Не удалось создать директории: {}. Ошибка: {}", m_ConfigPath.parent_path().string(), ec.message());

		std::ofstream file(m_ConfigPath, std::ios::binary);
		if (!file)
			return UNEXPECTED("Не удалось открыть файл: {}.", m_ConfigPath.string());

		file.write(reinterpret_cast<const char*>(buffer.data()), static_cast<std::streamsize>(buffer.size()));
		if (!file)
			return UNEXPECTED("Не удалось записать файл: {}.", m_Path.GetUserDataDirectory().string());
	}
	catch (const std::filesystem::filesystem_error& e)
	{
		return UNEXPECTED("Ошибка файловой системы: {}.", std::string(e.what()));
	}
	catch (const std::exception& e)
	{
		return UNEXPECTED("Ошибка сериализации конфигурации: {}.", std::string(e.what()));
	}
	catch (...)
	{
		return UNEXPECTED("Неизвестная ошибка сериализации конфигурации.");
	}

	m_IsDirty = false;
	DOut("Конфигурация успешно сериализована в файл: {}.", m_ConfigPath.string());

	return {};
#endif // Z_EDITOR
}

std::expected<void, std::string> ConfigManager::LoadConfig(std::filesystem::path path)
{
	try
	{
		std::ifstream in(path, std::ios::binary | std::ios::ate);
		if (!in)
			return UNEXPECTED("Не удалось открыть файл конфигурации.");

		std::streamsize fileSize = in.tellg();
		in.seekg(0, std::ios::beg); // Возвращаемся в начало файла

		// Читаем весь файл в буфер
		std::vector<char> buffer(fileSize);
		if (!in.read(buffer.data(), fileSize))
			return UNEXPECTED("Не удалось прочитать файл конфигурации.");

		// Создаем поток для чтения из буфера
		std::istringstream bufStream(std::string(buffer.data(), buffer.size()));

		std::size_t offset = 0;

		auto result = m_Serializer.DeSerialize(
			std::span(
				reinterpret_cast<const std::byte*>(buffer.data()),
				buffer.size()),
			offset,
			*m_EngineConfig);

		if (!result)
			return UNEXPECTED("Не удалось десериализовать конфигурацию: {}", result.error());
	}
	catch (const std::filesystem::filesystem_error& e)
	{
		return UNEXPECTED("Ошибка файловой системы: {}", std::string(e.what()));
	}
	catch (const std::exception& e)
	{
		return UNEXPECTED("Ошибка загрузки конфигурации: {}", std::string(e.what()));
	}
	catch (...)
	{
		return UNEXPECTED("Неизвестная ошибка загрузки конфигурации.");
	}

	return {};
}

std::expected<std::filesystem::path, std::string> ConfigManager::GetSettingsDirectory()
{
	// На Apple и Android используем директорию данных пользователя
#if Z_APPLE || Z_ANDROID
	return m_Path.GetUserDataDirectory();

	// На Windows и Linux используем директорию с исполняемым файлом
#elif Z_WINDOWS || Z_LINUX
	return m_Path.GetExecutableDirectory();
#else
#error >>>>> ConfigManager::GetSettingsDirectory(): Unsupported platform.
#endif
}
