#include <array>
#include <cctype>
#include <memory>
#include <cstdlib>
#include <algorithm>

#include "core/utils/Ensure.h"
#include "core/utils/Defines.h"
#include "core/constants/PackageConstants.h"

#include "Path.h"

namespace
{
	/// Зарезервированные Windows-имена устройств - запрещены как имена каталогов независимо от платформы,
	/// регистра и расширения (например "CON", "con", "CON.txt" - все запрещены для кроссплатформенной совместимости).
	constexpr std::array<std::string_view, 22> c_ReservedWindowsNames =
	{
		"CON", "PRN", "AUX", "NUL",
		"COM1", "COM2", "COM3", "COM4", "COM5", "COM6", "COM7", "COM8", "COM9",
		"LPT1", "LPT2", "LPT3", "LPT4", "LPT5", "LPT6", "LPT7", "LPT8", "LPT9"
	};

	[[nodiscard]] bool IsReservedWindowsName(std::string_view stem) noexcept
	{
		for (const auto& reserved : c_ReservedWindowsNames)
		{
			if (stem.size() != reserved.size())
				continue;

			bool equalCaseInsensitive = true;
			for (std::size_t i = 0; i < stem.size(); ++i)
			{
				if (std::toupper(static_cast<unsigned char>(stem[i])) != static_cast<int>(reserved[i]))
				{
					equalCaseInsensitive = false;
					break;
				}
			}

			if (equalCaseInsensitive)
				return true;
		}

		return false;
	}
}

namespace zzz::core
{
	Path::Path(std::shared_ptr<NativeAppData> nativeData) :
		m_NativeData{ std::move(nativeData) }
	{
		auto execDir = ResolveExecutableDirectory();
		if (!execDir)
			THROW_RUNTIME("Path: не удалось определить каталог исполняемого файла: {}", execDir.error());

		m_ExecutableDirectory = *execDir;
	}

	[[nodiscard]] std::expected<void, std::string> Path::InitializeUserData(std::string_view companyName, std::string_view appName)
	{
		ensure(IsValidDirectoryName(companyName), "Недопустимое имя компании для каталога пользовательских данных: '{}'.", companyName);
		ensure(IsValidDirectoryName(appName), "Недопустимое имя приложения для каталога пользовательских данных: '{}'.", appName);

		auto resPath = ResolveUserDataDirectory(companyName, appName);
		if (!resPath)
			return UNEXPECTED("Не удалось определить каталог пользовательских данных: {}.", resPath.error());

		m_UserDataDirectory = *resPath;

		return {};
	}

	[[nodiscard]] bool Path::IsValidDirectoryName(std::string_view name) noexcept
	{
		if (name.empty())
			return false;

		if (name == "." || name == "..")
			return false;

		if (name.back() == '.' || name.back() == ' ')
			return false;

		static constexpr std::string_view invalidChars = R"(< > : " / \ | ? *)";
		(void)invalidChars;
		for (unsigned char c : name)
		{
			if (c < 32)
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
			default:
				break;
			}
		}

		const auto dotPos = name.find('.');
		const std::string_view stem = (dotPos == std::string_view::npos) ? name : name.substr(0, dotPos);
		if (IsReservedWindowsName(stem))
			return false;

		return true;
	}

	/// @brief Определяет каталог, в котором расположен исполняемый файл приложения (вызывается один раз в конструкторе).
	[[nodiscard]] std::expected<std::filesystem::path, std::string> Path::ResolveExecutableDirectory() noexcept
	{
		try
		{
#if defined(Z_WINDOWS)
			wchar_t buffer[MAX_PATH];
			DWORD len = GetModuleFileNameW(nullptr, buffer, MAX_PATH);
			if (len == 0)
				return UNEXPECTED("Не удалось получить путь к исполняемому файлу.");

			return std::filesystem::path(buffer).parent_path();
#elif defined(Z_APPLE)
			uint32_t size = 0;
			_NSGetExecutablePath(nullptr, &size);

			std::string path(size, '\0');

			if (_NSGetExecutablePath(path.data(), &size) != 0)
				return UNEXPECTED("Не удалось получить путь к исполняемому файлу.");

			return std::filesystem::weakly_canonical(path).parent_path();
#elif defined(Z_ANDROID)
			auto app = m_NativeData.get();
			if (!app || !app->activity || !app->activity->internalDataPath)
				return UNEXPECTED("Android activity или internalDataPath равен null.");

			return std::filesystem::path(app->activity->internalDataPath);
#elif defined(Z_LINUX)
			char buffer[PATH_MAX];
			ssize_t len = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
			if (len == -1)
				return UNEXPECTED("Не удалось получить путь к исполняемому файлу.");

			buffer[len] = '\0';

			return std::filesystem::weakly_canonical(buffer).parent_path();
#else
#error ">>>>> zzz::core::Path::ResolveExecutableDirectory(): Unsupported platform."
#endif
		}
		catch (const std::filesystem::filesystem_error& e)
		{
			return UNEXPECTED("Ошибка файловой системы: {}.", e.what());
		}
		catch (const std::exception& e)
		{
			return UNEXPECTED("Не удалось получить путь к исполняемому файлу: {}.", e.what());
		}
		catch (...)
		{
			return UNEXPECTED("Неизвестная ошибка при получении пути к исполняемому файлу.");
		}
	}

	/// @brief Возвращает каталог пользовательских данных приложения (двухуровневый company/name) для текущей платформы.
	[[nodiscard]] std::expected<std::filesystem::path, std::string> Path::ResolveUserDataDirectory(std::string_view companyName, std::string_view appName)
	{
		try
		{
#if defined(Z_WINDOWS)
			wchar_t* rawPtr = nullptr;
			size_t len = 0;
			_wdupenv_s(&rawPtr, &len, L"LOCALAPPDATA");
			std::unique_ptr<wchar_t, decltype(&free)> localAppData(rawPtr, &free);
			if (!localAppData)
				return UNEXPECTED("Не удалось получить LOCALAPPDATA.");

			return std::filesystem::path(localAppData.get()) / companyName / appName;
#elif defined(Z_APPLE)
			auto path = GetAppleUserDataDirectory();
			if (!path)
				return UNEXPECTED("{}", path.error());

			return *path / companyName / appName;
#elif defined(Z_ANDROID)
			auto app = m_NativeData.get();
			if (!app->activity)
				return UNEXPECTED("Android activity равен null.");

			if (!app->activity->internalDataPath)
				return UNEXPECTED("Внутренний путь данных Android равен null.");

			return std::filesystem::path(app->activity->internalDataPath) / companyName / appName;
#elif defined(Z_LINUX)
			const char* xdgConfigHome = std::getenv("XDG_CONFIG_HOME");
			if (xdgConfigHome)
				return std::filesystem::path(xdgConfigHome) / companyName / appName;

			const char* home = std::getenv("HOME");
			if (!home)
				return UNEXPECTED("Не удалось получить HOME.");

			return std::filesystem::path(home) / ".config" / companyName / appName;
#else
#error ">>>>> zzz::core::Path::ResolveUserDataDirectory(): Unsupported platform."
#endif
		}
		catch (const std::filesystem::filesystem_error& e)
		{
			return UNEXPECTED("Ошибка файловой системы: {}.", e.what());
		}
		catch (const std::exception& e)
		{
			return UNEXPECTED("Не удалось получить каталог пользовательских данных: {}", e.what());
		}
		catch (...)
		{
			return UNEXPECTED("Неизвестная ошибка при получении каталога пользовательских данных.");
		}
	}

	[[nodiscard]] std::expected<std::filesystem::path, std::string> Path::GetDirectory(eFileLocation location) const noexcept
	{
		if (location == eFileLocation::App)
			return m_ExecutableDirectory;

		if (m_UserDataDirectory.empty())
			return UNEXPECTED("Каталог пользовательских данных не инициализирован. Вызовите InitializeUserData() перед обращением к подкаталогам.");

		switch (location)
		{
		case eFileLocation::User:  return m_UserDataDirectory;
		case eFileLocation::Cache: return m_UserDataDirectory / c_CacheDirectoryName;
		case eFileLocation::Saves: return m_UserDataDirectory / c_SavesDirectoryName;
		case eFileLocation::Logs:  return m_UserDataDirectory / c_LogsDirectoryName;
		default:
			return UNEXPECTED("Path::GetDirectory(): необработанный eFileLocation: {}.", static_cast<int>(location));
		}
	}
}
