
#include <fstream>
#include <core/utils/Constants.h>
#include <core/Serialize/Serializer.h>
#include <core/io/Path.h>
#include <engine/platforms/package/PackageManager.h>

#include "UserSettingsManager.h"

using namespace zzz::core;
using zzz::core::AppViewUserData;

UserSettingsManager::UserSettingsManager(const Path& path, const zzz::engine::PackageManager& packageManager) :
	m_Path(path),
	m_Version(c_ConfigFileMajorVersion, c_ConfigFileMinorVersion, c_ConfigFilePatchVersion),
	m_IsDirty(true)
{
#if Z_EDITOR
	(void)packageManager;
#else
	Initialize(packageManager);
	LogUserData();
#endif
}

void UserSettingsManager::Initialize(const zzz::engine::PackageManager& packageManager)
{
	try
	{
		auto path = GetSettingsDirectory();
		if (!path)
			THROW_RUNTIME("Не удалось получить каталог настроек: {}.", path.error());

		m_ConfigPath = (path.value() / c_ConfigFileName)
			.lexically_normal()
			.make_preferred();

		SetDefaultUserSettings(packageManager);
		if (!std::filesystem::exists(m_ConfigPath))
		{
			DOutWarning("Файл конфигурации не найден: {}. Используется конфигурация по умолчанию.", m_ConfigPath.string());
			return;
		}

		auto res = LoadConfig(m_ConfigPath);
		if (!res)
		{
			DOutWarning("Не удалось загрузить файл конфигурации: {}. Создаётся конфигурация по умолчанию.", m_ConfigPath.string());
			SetDefaultUserSettings(packageManager);
		}
	}
	catch (const std::filesystem::filesystem_error& e)
	{
		DOutException("Ошибка файловой системы: {}. Установка конфигурации по умолчанию.", e.what());
		SetDefaultUserSettings(packageManager);
		return;
	}
	catch (const std::exception& e)
	{
		DOutException("Ошибка загрузки конфигурации: {}. Установка конфигурации по умолчанию.", e.what());
		SetDefaultUserSettings(packageManager);
		return;
	}
	catch (...)
	{
		DOutException("Неизвестная ошибка загрузки конфигурации. Установка конфигурации по умолчанию.");
		SetDefaultUserSettings(packageManager);
		return;
	}

	DOut("[UserSettingsManager] Конфигурация десериализована: {}.", m_ConfigPath.string());
}

void UserSettingsManager::SetDefaultUserSettings(const zzz::engine::PackageManager& packageManager)
{
	auto appViewData = packageManager.GetAppViewData();
	if (!appViewData)
		THROW_RUNTIME("Не удалось загрузить AppViewData: {}", appViewData.error());

	m_AppViewUserData = AppViewUserData(*appViewData);
	m_PlatformConfig = PlatformConfig();
	m_Version = Version(c_ConfigFileMajorVersion, c_ConfigFileMinorVersion, c_ConfigFilePatchVersion);
}

[[nodiscard]] std::expected<void, std::string> UserSettingsManager::SaveConfig()
{
#if Z_EDITOR
	return {};
#else

	if (!m_IsDirty)
	{
		DOut("[UserSettingsManager] Конфигурация не изменена. Сохранение не требуется.");
		return {};
	}

	try
	{
		std::vector<std::byte> buffer;
		if (auto res = m_Serializer.Serialize(buffer, *this); !res)
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
	DOut("[UserSettingsManager] Конфигурация сериализована: {}.", m_ConfigPath.string());

	return {};
#endif // Z_EDITOR
}

std::expected<void, std::string> UserSettingsManager::LoadConfig(std::filesystem::path path)
{
	try
	{
		std::ifstream in(path, std::ios::binary | std::ios::ate);
		if (!in)
			return UNEXPECTED("Не удалось открыть файл конфигурации.");

		std::streamsize fileSize = in.tellg();
		in.seekg(0, std::ios::beg);

		std::vector<char> buffer(fileSize);
		if (!in.read(buffer.data(), fileSize))
			return UNEXPECTED("Не удалось прочитать файл конфигурации.");

		std::size_t offset = 0;

		auto result = m_Serializer.Deserialize(
			std::span(
				reinterpret_cast<const std::byte*>(buffer.data()),
				buffer.size()),
			offset,
			*this);

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

std::expected<std::filesystem::path, std::string> UserSettingsManager::GetSettingsDirectory()
{
#if Z_APPLE || Z_ANDROID
	return m_Path.GetUserDataDirectory();
#elif Z_WINDOWS || Z_LINUX
	return m_Path.GetExecutableDirectory();
#else
#error >>>>> UserSettingsManager::GetSettingsDirectory(): Unsupported platform.
#endif
}

[[nodiscard]] std::expected<void, std::string> UserSettingsManager::Serialize(std::vector<std::byte>& buffer, const Serializer& s) const
{
	return s.Serialize(buffer, c_ConfigHeader)
		.and_then([&]() { return s.Serialize(buffer, m_Version); })
		.and_then([&]() { return s.Serialize(buffer, m_AppViewUserData); })
		.and_then([&]() { return s.Serialize(buffer, m_PlatformConfig); });
}

[[nodiscard]] std::expected<void, std::string> UserSettingsManager::Deserialize(std::span<const std::byte> buffer, std::size_t& offset, const Serializer& s)
{
	FileHeader<3> header{};

	return s.Deserialize(buffer, offset, header)
		.and_then([&]() -> std::expected<void, std::string>
			{
				if (header != c_ConfigHeader)
					return UNEXPECTED("Некорректный заголовок конфигурации.");

				return s.Deserialize(buffer, offset, m_Version);
			})
		.and_then([&]() { return s.Deserialize(buffer, offset, m_AppViewUserData); })
		.and_then([&]() { return s.Deserialize(buffer, offset, m_PlatformConfig); });
}

#pragma region Logging
void UserSettingsManager::LogUserData() const
{
#if Z_ADD_LOGGER || Z_DEVELOPMENT_BUILD
	DOut("========== User Data: {} ==========", m_ConfigPath.string());
	m_AppViewUserData.LogFileBlock("  ");
	DOut("  [PlatformConfig]");
	m_PlatformConfig.LogFileBlock("    ");
#endif
}
#pragma endregion
