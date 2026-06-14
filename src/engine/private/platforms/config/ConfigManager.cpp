
#include <fstream>
#include <system_error>

#include "io/Path.h"
#include "ConfigManager.h"
#include "headers/constants.h"
#include "../../core/serialize/Serializer.h"

using namespace zzz::io;
using namespace zzz::engine;

ConfigManager::ConfigManager(const Path& path) :
	m_Path(path),
	m_IsDirty(true)
{
	Initialize();
}

void ConfigManager::Initialize()
{
	try
	{
		auto resPath = GetSettingsDirectory();
		if (!resPath)
			THROW_RUNTIME("Failed to get settings directory: {}.", resPath.error());

		m_ConfigPath = (resPath.value() / c_ConfigFileName)
			.lexically_normal()
			.make_preferred();

		// Далее работаем с файлом
		m_EngineConfig = safe_make_shared<EngineConfig>();

		if (!std::filesystem::exists(m_ConfigPath))
			DOutWarning("Config file not found: {}. Using default config.", m_ConfigPath.string());

		auto loadResult = LoadConfig(m_ConfigPath);
		if (!loadResult)
		{
			DOutWarning("Failed to load config file: {}. Creating default config.", m_ConfigPath.string());
			m_EngineConfig = safe_make_shared<EngineConfig>();
		}
	}
	catch (const std::filesystem::filesystem_error& e)
	{
		DOutException("Filesystem error: {}. Setting to default config.", e.what());
		m_EngineConfig = safe_make_shared<EngineConfig>();

		return;
	}
	catch (const std::exception& e)
	{
		DOutException("Config loading error: {}. Setting to default config.", e.what());
		m_EngineConfig = safe_make_shared<EngineConfig>();

		return;
	}
	catch (...)
	{
		DOutException("Unknown config loading error. Setting to default config.");
		m_EngineConfig = safe_make_shared<EngineConfig>();

		return;
	}

	DOut("Config deserialized successfully from file: {}.", m_ConfigPath.string());
}

[[nodiscard]] std::expected<void, std::string> ConfigManager::SaveConfig()
{
	if (!m_IsDirty)
	{
		DOut("Config is not dirty. No need to save.");
		return {};
	}

	try
	{
		std::vector<std::byte> buffer;
		if (auto res = m_Serializer.Serialize(buffer, *m_EngineConfig); !res)
			return UNEXPECTED("Failed to serialize config: {}.", res.error());

		std::error_code ec;
		std::filesystem::create_directories(m_ConfigPath.parent_path(), ec);
		if (ec)
			return UNEXPECTED("Failed to create directories: {}. Error: {}", m_ConfigPath.parent_path().string(), ec.message());

		std::ofstream file(m_ConfigPath, std::ios::binary);
		if (!file)
			return UNEXPECTED("Failed to open file: {}.", m_ConfigPath.string());

		file.write(reinterpret_cast<const char*>(buffer.data()), static_cast<std::streamsize>(buffer.size()));
		if (!file)
			return UNEXPECTED("Failed to write file: {}.", m_Path.GetUserDataDirectory().string());
	}
	catch (const std::filesystem::filesystem_error& e)
	{
		return UNEXPECTED("Filesystem error: {}.", std::string(e.what()));
	}
	catch (const std::exception& e)
	{
		return UNEXPECTED("Config serialization error: {}.", std::string(e.what()));
	}
	catch (...)
	{
		return UNEXPECTED("Unknown config serialization error.");
	}

	m_IsDirty = false;
	DOut("Config serialized successfully to file: {}.", m_ConfigPath.string());

	return {};
}

std::expected<void, std::string> ConfigManager::LoadConfig(std::filesystem::path path)
{
	try
	{
		std::ifstream in(path, std::ios::binary | std::ios::ate);
		if (!in)
			return UNEXPECTED("Failed to open config file.");

		std::streamsize fileSize = in.tellg();
		in.seekg(0, std::ios::beg); // Возвращаемся в начало файла

		// Читаем весь файл в буфер
		std::vector<char> buffer(fileSize);
		if (!in.read(buffer.data(), fileSize))
			return UNEXPECTED("Failed to read config file.");

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
			return UNEXPECTED("Failed to deserialize config: {}", result.error());
	}
	catch (const std::filesystem::filesystem_error& e)
	{
		return UNEXPECTED("Filesystem error: {}", std::string(e.what()));
	}
	catch (const std::exception& e)
	{
		return UNEXPECTED("Config loading error: {}", std::string(e.what()));
	}
	catch (...)
	{
		return UNEXPECTED("Unknown config loading error.");
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
#error >>>>> Unsupported platform
#endif
}