
#include <fstream>

#include "ConfigManager.h"
#include "../IO/Path.h"
#include "../../../headers/Constants.h"

using namespace zzz::io;
using namespace zzz::engine;

ConfigManager::ConfigManager(std::shared_ptr<Path> path) :
	m_Path(path)
{
	ensure(m_Path, "Path must not be null.");
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
		if (!std::filesystem::exists(settingsPath))
		{
			DOutLite("Config file not found at {}. Creating default config.", settingsPath.string());
			engineConfig = zzz::safe_make_shared<EngineConfig>();

			return eInitConfigState::InitDefault;
		}
		else
		{
			std::ifstream file(settingsPath);

			if (!file.is_open())
			{
				DOutLite("Failed to open config file: {}. Creating default config.", settingsPath.string());
				engineConfig = zzz::safe_make_shared<EngineConfig>();

				return eInitConfigState::InitDefault;
			}

			//engineConfig = LoadConfig(file);
		}
	}
	catch (const std::filesystem::filesystem_error& e)
	{
		DOutLite("Filesystem error: {}. Setting to default config.", e.what());
		engineConfig = zzz::safe_make_shared<EngineConfig>();

		return eInitConfigState::InitDefault;
	}
	catch (const std::exception& e)
	{
		DOutLite("Config loading error: {}. Setting to default config.", e.what());
		engineConfig = zzz::safe_make_shared<EngineConfig>();

		return eInitConfigState::InitDefault;
	}
	catch (...)
	{
		DOutLite("Unknown config loading error. Setting to default config.");
		engineConfig = zzz::safe_make_shared<EngineConfig>();

		return eInitConfigState::InitDefault;
	}

	DOutLite("Settings file path: {}. OK.", settingsPath.string());

	return eInitConfigState::InitOK;
}