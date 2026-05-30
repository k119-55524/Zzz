
#include "../../../header.h"
#include "Path.h"

namespace zzz::io
{
	Path::Path(std::string_view appName, std::shared_ptr<void> platformData) :
		m_AppName{ appName },
		m_PlatformData{ platformData }
	{	
		ensure(IsValidDirectoryName(m_AppName) == true, "Invalid application name for directory: {}", m_AppName);

		auto resPath = ResolveUserDataDirectory();
		if (!resPath)
			ensure(false, "Failed to resolve user data directory: {}.", resPath.error());
		m_UserDataDirectory = *resPath;
	}

	/// @brief Проверяет корректность имени каталога для всех поддерживаемых платформ.
	[[nodiscard]] bool Path::IsValidDirectoryName(std::string_view name)
	{
		if (name.empty())
			return false;

		static constexpr std::string_view invalidChars = R"(< > : " / \ | ? *)";
		for (char c : name)
		{
			if (static_cast<unsigned char>(c) < 32)
				return false;

			switch (c)
			{
			case '<':
			case '>':
			case ':':
			case '"':
			case '/':
			case '\\':
			case '|':
			case '?':
			case '*':
				return false;
			}
		}

		return true;
	}

	/// @brief Возвращает каталог, в котором расположен исполняемый файл приложения.
	[[nodiscard]] std::expected<std::filesystem::path, std::string> Path::GetExecutableDirectory()
	{
		try
		{
#if defined(_WIN32)
			wchar_t buffer[MAX_PATH];
			DWORD len = GetModuleFileNameW(nullptr, buffer, MAX_PATH);
			if (len == 0)
				UNEXPECTED("Failed to get executable path.");

			return std::filesystem::path(buffer).parent_path();
#elif defined(__APPLE__)
#if TARGET_OS_IPHONE
			UNEXPECTED("iOS is not supported.");
#else
			uint32_t size = 0;
			_NSGetExecutablePath(nullptr, &size);

			std::string path(size, '\0');

			if (_NSGetExecutablePath(path.data(), &size) != 0)
				UNEXPECTED("Failed to get executable path.");

			return std::filesystem::weakly_canonical(path).parent_path();
#endif
#elif defined(__linux__) && !defined(__ANDROID__)
			char buffer[PATH_MAX];
			ssize_t len = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
			if (len == -1)
				UNEXPECTED("Failed to get executable path.");

			buffer[len] = '\0';

			return std::filesystem::weakly_canonical(buffer).parent_path();
#else
			UNEXPECTED("Unsupported platform.");
#endif
		}
		catch (const std::filesystem::filesystem_error& e)
		{
			UNEXPECTED("Filesystem error: {}.", e.what());
		}
		catch (const std::exception& e)
		{
			UNEXPECTED("Failed to get executable path: {}.", e.what());
		}
		catch (...)
		{
			UNEXPECTED("Unknown error while getting executable path.");
		}
	}

	/// @brief Возвращает каталог пользовательских данных приложения для текущей платформы.
	[[nodiscard]] std::expected<std::filesystem::path, std::string> Path::ResolveUserDataDirectory()
	{
		try
		{
#if defined(_WIN32)
			wchar_t* localAppData = nullptr;
			size_t len = 0;
			_wdupenv_s(&localAppData, &len, L"LOCALAPPDATA");
			if (!localAppData)
				UNEXPECTED("Failed to get LOCALAPPDATA.");

			std::filesystem::path result(localAppData);
			free(localAppData);

			return result / m_AppName;
#elif defined(__APPLE__)
			auto path = GetAppleUserDataDirectory();
			if (!path)
				return std::unexpected(path.error());

			return *path / m_AppName;
#elif defined(__ANDROID__)
			auto app = static_cast<android_app*>(m_PlatformData.get());
			if (!app)
				UNEXPECTED("Android app context is null.");

			if (!app->activity)
				UNEXPECTED("Android activity is null.");

			if (!app->activity->internalDataPath)
				UNEXPECTED("Android internal data path is null.");

			return std::filesystem::path(app->activity->internalDataPath) / m_AppName;
#elif defined(__linux__)
			const char* xdgConfigHome = std::getenv("XDG_CONFIG_HOME");
			if (xdgConfigHome)
				return std::filesystem::path(xdgConfigHome) / m_AppName;

			const char* home = std::getenv("HOME");
			if (!home)
				UNEXPECTED("Failed to get HOME.");

			return std::filesystem::path(home) / ".config" / m_AppName;
#else
			UNEXPECTED("Unsupported platform.");
#endif
		}
		catch (const std::filesystem::filesystem_error& e)
		{
			UNEXPECTED("Filesystem error: {}.", e.what());
		}
		catch (const std::exception& e)
		{
			UNEXPECTED("Failed to get user data directory: {}", e.what());
		}
		catch (...)
		{
			UNEXPECTED("Unknown error while getting executable path.");
		}
	}
}
