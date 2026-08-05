
#include <cstdlib>
#include <memory>

#include "core/utils/Types.h"
#include "core/utils/Ensure.h"
#include "core/utils/Defines.h"
#include "core/utils/Macroses.h"
#include "core/utils/MemoryUtils.h"
#include "core/utils/ThrowWrappers.h"
#include "core/utils/ThrowWrappers.h"
#include "core/headers/Android.h"
#include "core/headers/Apple.h"
#include "core/headers/Linux.h"
#include "core/headers/MSWin.h"
#include "core/io/Path.h"
#include <logger/logger.h>

namespace zzz::core
{
	Path::Path(std::string_view appName, std::shared_ptr<NativeAppData> nativeData) :
		m_AppName{appName},
		m_NativeData{ nativeData }
	{	
		ensure(IsValidDirectoryName(appName) == true, "Некорректное имя приложения для каталога: {}", appName);

		auto resPath = ResolveUserDataDirectory(appName);
		if (!resPath)
			THROW_RUNTIME("Не удалось определить каталог пользовательских данных: {}.", resPath.error());

		m_UserDataDirectory = *resPath;
	}

	/// @brief Проверяет корректность имени каталога для всех поддерживаемых платформ.
	[[nodiscard]] bool Path::IsValidDirectoryName(std::string_view name) const noexcept
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
	[[nodiscard]] const std::expected<std::filesystem::path, std::string> Path::GetExecutableDirectory() const noexcept
	{
		try
		{
#if Z_WINDOWS
			wchar_t buffer[MAX_PATH];
			DWORD len = GetModuleFileNameW(nullptr, buffer, MAX_PATH);
			if (len == 0)
				return UNEXPECTED("Не удалось получить путь к исполняемому файлу.");

			return std::filesystem::path(buffer).parent_path();
#elif Z_MACOS
			uint32_t size = 0;
			_NSGetExecutablePath(nullptr, &size);

			std::string path(size, '\0');

			if (_NSGetExecutablePath(path.data(), &size) != 0)
				return UNEXPECTED("Не удалось получить путь к исполняемому файлу.");

			return std::filesystem::weakly_canonical(path).parent_path();
#elif Z_LINUX
			char buffer[PATH_MAX];
			ssize_t len = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
			if (len == -1)
				return UNEXPECTED("Не удалось получить путь к исполняемому файлу.");

			buffer[len] = '\0';

			return std::filesystem::weakly_canonical(buffer).parent_path();
#else
			return UNEXPECTED("Неподдерживаемая платформа.");
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

	/// @brief Возвращает каталог пользовательских данных приложения для текущей платформы.
	[[nodiscard]] std::expected<std::filesystem::path, std::string> Path::ResolveUserDataDirectory(std::string_view appName)
	{
		try
		{
#if Z_WINDOWS
			wchar_t* rawPtr = nullptr;
			size_t len = 0;
			_wdupenv_s(&rawPtr, &len, L"LOCALAPPDATA");
			std::unique_ptr<wchar_t, decltype(&free)> localAppData(rawPtr, &free);
			if (!localAppData)
				return UNEXPECTED("Не удалось получить LOCALAPPDATA.");

			return std::filesystem::path(localAppData.get()) / appName;
#elif Z_APPLE
			auto path = GetAppleUserDataDirectory();
			if (!path)
				return std::unexpected(path.error());

			return *path / appName;
#elif Z_ANDROID
			auto app = m_NativeData.get();
			if (!app->activity)
				return UNEXPECTED("Android activity равен null.");

			if (!app->activity->internalDataPath)
				return UNEXPECTED("Внутренний путь данных Android равен null.");

			return std::filesystem::path(app->activity->internalDataPath) / appName;
#elif Z_LINUX
			const char* xdgConfigHome = std::getenv("XDG_CONFIG_HOME");
			if (xdgConfigHome)
				return std::filesystem::path(xdgConfigHome) / appName;

			const char* home = std::getenv("HOME");
			if (!home)
				return UNEXPECTED("Не удалось получить HOME.");

			return std::filesystem::path(home) / ".config" / appName;
#else
#error >>>>> zzz::core::Path::ResolveUserDataDirectory(): Unsupported platform.
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
}
