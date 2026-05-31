
#include <fstream>

#include "IO/Path.h"
#include "ConfigManager.h"
#include "headers/constants.h"
#include "Serialize/Serializer.h"

using namespace zzz::io;
using namespace zzz::engine;

ConfigManager::ConfigManager(std::shared_ptr<Path> path) :
	m_Path(path)
{
	ensure(m_Path, "Path must not be null.");
}

[[nodiscard]] std::expected<void, std::string> ConfigManager::Serialize()
{
	try
	{
		std::vector<std::byte> buffer;
		if (auto res = m_Serializer.Serialize(buffer, *m_EngineConfig); !res)
			UNEXPECTED("Failed to serialize config: {}.", res.error());

		std::ofstream file(m_ConfigPath, std::ios::binary);
		if (!file)
			UNEXPECTED("Failed to open file: {}.", m_ConfigPath.string());

		file.write(reinterpret_cast<const char*>(buffer.data()), static_cast<std::streamsize>(buffer.size()));
		if (!file)
			UNEXPECTED("Failed to write file: {}.", m_Path->GetUserDataDirectory().string());
	}
	catch (const std::filesystem::filesystem_error& e)
	{
		UNEXPECTED("Filesystem error: {}.", std::string(e.what()));
	}
	catch (const std::exception& e)
	{
		UNEXPECTED("Config serialization error: {}.", std::string(e.what()));
	}
	catch (...)
	{
		UNEXPECTED("Unknown config serialization error.");
	}

	return {};
}

[[nodiscard]] std::expected<eInitConfigState, std::string> ConfigManager::Initialize(std::string_view configPath)
{
	try
	{
		std::filesystem::path userPath(configPath);
		if (userPath.is_absolute())
			UNEXPECTED("Config path must be relative.");

		auto resPath = GetSettingsDirectory();
		if (!resPath)
			UNEXPECTED("Failed to get settings directory: {}.", resPath.error());

		m_ConfigPath = resPath.value();
		if (!userPath.empty())
			m_ConfigPath /= userPath;

		m_ConfigPath /= configFileName;
		m_ConfigPath = m_ConfigPath.lexically_normal();

		// Далее работаем с файлом
		m_EngineConfig = zzz::safe_make_shared<EngineConfig>();
		auto loadResult = LoadConfig(m_ConfigPath);
		if (!loadResult)
		{
			DOutWarning("Failed to load config file: {}. Creating default config.", m_ConfigPath.string());
			m_EngineConfig = zzz::safe_make_shared<EngineConfig>();
			return eInitConfigState::InitDefault;
		}
	}
	catch (const std::filesystem::filesystem_error& e)
	{
		DOutException("Filesystem error: {}. Setting to default config.", e.what());
		m_EngineConfig = zzz::safe_make_shared<EngineConfig>();

		return eInitConfigState::InitDefault;
	}
	catch (const std::exception& e)
	{
		DOutException("Config loading error: {}. Setting to default config.", e.what());
		m_EngineConfig = zzz::safe_make_shared<EngineConfig>();

		return eInitConfigState::InitDefault;
	}
	catch (...)
	{
		DOutException("Unknown config loading error. Setting to default config.");
		m_EngineConfig = zzz::safe_make_shared<EngineConfig>();

		return eInitConfigState::InitDefault;
	}

	DOut("Config initialized successfully from file: {}.", m_ConfigPath.string());

	return eInitConfigState::InitOK;
}

std::expected<void, std::string> ConfigManager::LoadConfig(std::filesystem::path path)
{
	try
	{
		std::ifstream in(path, std::ios::binary | std::ios::ate);
		if (!in)
			UNEXPECTED("Failed to open config file.");

		std::streamsize fileSize = in.tellg();
		in.seekg(0, std::ios::beg); // Возвращаемся в начало файла

		// Читаем весь файл в буфер
		std::vector<char> buffer(fileSize);
		if (!in.read(buffer.data(), fileSize))
			UNEXPECTED("Failed to read config file.");

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
			UNEXPECTED("Failed to deserialize config: {}.", result.error());
	}
	catch (const std::filesystem::filesystem_error& e)
	{
		UNEXPECTED("Filesystem error: {}", std::string(e.what()));
	}
	catch (const std::exception& e)
	{
		UNEXPECTED("Config loading error: {}", std::string(e.what()));
	}
	catch (...)
	{
		UNEXPECTED("Unknown config loading error.");
	}

	DOut("Config file {} loaded successfully.", path.string());

	return {};
}

std::expected<std::filesystem::path, std::string> ConfigManager::GetSettingsDirectory()
{
#if defined(__APPLE__) || defined(__ANDROID__)
	return m_Path->GetUserDataDirectory();
#elif defined(_WIN32) || defined(__linux__)
	return m_Path->GetExecutableDirectory();
#else
#error Unsupported platform
#endif
}