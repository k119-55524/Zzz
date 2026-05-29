
#include "ConfigManager.h"
#include "../IO/Path.h"

using namespace zzz::io;
using namespace zzz::engine;

ConfigManager::ConfigManager()
{
}

[[nodiscard]] std::expected<std::filesystem::path, std::string> ConfigManager::GetSettingsDirectory()
{
#if defined(_WIN32)
	wchar_t* localAppData = nullptr;
	size_t len = 0;

	_wdupenv_s(&localAppData, &len, L"LOCALAPPDATA");
	if (!localAppData)
		return std::unexpected("Failed to get LOCALAPPDATA.");

	std::filesystem::path result(localAppData);
	free(localAppData);

	return result / "ZzzEngine";
#elif defined(__APPLE__)
	const char* home = std::getenv("HOME");
	if (!home)
		return std::unexpected("Failed to get HOME.");

	return std::filesystem::path(home) / "Library" / "Application Support" / "ZzzEngine";
#elif defined(__linux__)
	const char* xdgConfigHome = std::getenv("XDG_CONFIG_HOME");
	if (xdgConfigHome)
		return std::filesystem::path(xdgConfigHome) / "ZzzEngine";

	const char* home = std::getenv("HOME");
	if (!home)
		return std::unexpected("Failed to get HOME.");

	return std::filesystem::path(home) / ".config" / "ZzzEngine";
#else
#error Unsupported platform
#endif
}

std::expected<void, std::string> ConfigManager::Initialize(std::string_view configPath)
{
	if (!configPath.empty())
	{
		if (!Path::IsPathStringValid(configPath))
		{
			auto errorMessage = std::format("Invalid config path: {}.", configPath);
			DOutLite(errorMessage);
			return std::unexpected(errorMessage);
		}
	}

	std::filesystem::path userPath(configPath);
	if (userPath.is_absolute())
	{
		auto errorMessage = std::format("Config path must be relative: {}.", configPath);
		DOutLite(errorMessage);
		return std::unexpected(errorMessage);
	}

	auto resPath = GetSettingsDirectory();
	if (!resPath)
	{
		DOutLite("Failed to get settings directory: {}.", resPath.error());
		return std::unexpected(resPath.error());
	}

	auto settingsPath = resPath.value();
	if (!userPath.empty())
		settingsPath /= userPath;

	settingsPath /= configFileName;
	settingsPath = settingsPath.lexically_normal();

	// TODO: далее работаем с файлом

	DOutLite("Settings file path: {}.", settingsPath.string());

	return {};
}