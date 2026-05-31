
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
	return {};
}

[[nodiscard]] std::expected<eInitConfigState, std::string> ConfigManager::Initialize(std::string_view configPath)
{
	std::filesystem::path userPath(configPath);
	if (userPath.is_absolute())
		UNEXPECTED("Config path must be relative.");

	auto resPath = GetSettingsDirectory();
	if (!resPath)
		UNEXPECTED("Failed to get settings directory: {}.", resPath.error());

	auto settingsPath = resPath.value();
	if (!userPath.empty())
		settingsPath /= userPath;

	settingsPath /= configFileName;
	settingsPath = settingsPath.lexically_normal();

	// Далее работаем с файлом
	try
	{
		m_EngineConfig = zzz::safe_make_shared<EngineConfig>();

		if (!std::filesystem::exists(settingsPath))
		{
			DOutWarning("Config file not found at {}. Creating default config.", settingsPath.string());
			return eInitConfigState::InitDefault;
		}
		else
		{
			std::ifstream file(settingsPath);

			if (!std::filesystem::is_regular_file(settingsPath))
			{
				DOutWarning("Failed to open config file: {}. Creating default config.", settingsPath.string());
				return eInitConfigState::InitDefault;
			}

			auto loadResult = LoadConfig(settingsPath);
			if (!loadResult)
			{
				DOutWarning("Failed to load config file: {}. Creating default config.", settingsPath.string());
				m_EngineConfig = zzz::safe_make_shared<EngineConfig>();

				return eInitConfigState::InitDefault;
			}
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

	DOut("Settings file path: {}. OK.", settingsPath.string());

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

		auto result = serializer.DeSerialize(
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