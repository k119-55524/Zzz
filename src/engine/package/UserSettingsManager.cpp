
#include "PackageManager.h"
#include "UserSettingsManager.h"
#include "engine/view/View.h"

using namespace zzz::core;
using zzz::core::PrimaryViewUserData;

namespace zzz::engine
{
	UserSettingsManager::UserSettingsManager(const Path& path, const PrimaryViewData& defaultPrimaryViewData) :
		m_Path(path),
		m_Version(c_ConfigFileMajorVersion, c_ConfigFileMinorVersion, c_ConfigFilePatchVersion),
		m_IsDirty(true)
	{
#if Z_EDITOR
		(void)defaultPrimaryViewData;
#else
		Initialize(defaultPrimaryViewData);
		LogUserData();
#endif
	}

	void UserSettingsManager::Initialize(const PrimaryViewData& defaultPrimaryViewData)
	{
		try
		{
			auto path = GetSettingsDirectory();
			if (!path)
				THROW_RUNTIME("Не удалось получить каталог настроек: {}.", path.error());

			m_ConfigPath = (path.value() / c_ConfigFileName)
				.lexically_normal()
				.make_preferred();

			SetDefaultUserSettings(defaultPrimaryViewData);
			if (!std::filesystem::exists(m_ConfigPath))
			{
				DOutWarning("Файл конфигурации не найден: {}. Используется конфигурация по умолчанию.", m_ConfigPath.string());
				return;
			}

			auto res = LoadConfig(m_ConfigPath);
			if (!res)
			{
				DOutWarning("Не удалось загрузить файл конфигурации: {}. Создаётся конфигурация по умолчанию.", m_ConfigPath.string());
				SetDefaultUserSettings(defaultPrimaryViewData);
			}
			else
			{
				m_IsFirstRun = false;
			}

			auto& primaryPlatformData = m_PrimaryViewUserData.GetPlatformData();
			const auto currentState = primaryPlatformData.GetWindowState();
			if (currentState == eWindowState::Closed || currentState == eWindowState::Minimized)
			{
				const auto defaultState = defaultPrimaryViewData.GetPlatformData().GetWindowState();
				const bool isFullscreen = (defaultState == eWindowState::BorderlessFullscreen || defaultState == eWindowState::ExclusiveFullscreen);
				primaryPlatformData.SetWindowState(isFullscreen ? defaultState : eWindowState::Normal);
				m_IsDirty = true;
			}
		}
		catch (const std::filesystem::filesystem_error& e)
		{
			DOutException("Ошибка файловой системы: {}. Установка конфигурации по умолчанию.", e.what());
			SetDefaultUserSettings(defaultPrimaryViewData);
			return;
		}
		catch (const std::exception& e)
		{
			DOutException("Ошибка загрузки конфигурации: {}. Установка конфигурации по умолчанию.", e.what());
			SetDefaultUserSettings(defaultPrimaryViewData);
			return;
		}
		catch (...)
		{
			DOutException("Неизвестная ошибка загрузки конфигурации. Установка конфигурации по умолчанию.");
			SetDefaultUserSettings(defaultPrimaryViewData);
			return;
		}

		DOut("[UserSettingsManager] Конфигурация десериализована: {}.", m_ConfigPath.string());
	}

	void UserSettingsManager::SetDefaultUserSettings(const PrimaryViewData& defaultPrimaryViewData)
	{
		m_Version = Version(c_ConfigFileMajorVersion, c_ConfigFileMinorVersion, c_ConfigFilePatchVersion);
		m_PrimaryViewUserData = PrimaryViewUserData(defaultPrimaryViewData);
	}

	void UserSettingsManager::SetSelectedGpuId(std::string gpuId)
	{
		if (m_SelectedGpuId != gpuId)
		{
			m_SelectedGpuId = std::move(gpuId);
			m_IsDirty = true;
		}
	}

	const ViewUserData& UserSettingsManager::GetChildViewUserData(const Guid& guid) const
	{
		auto it = m_ChildViewsUserData.find(guid);
		ensure(it != m_ChildViewsUserData.end(), "Пользовательские настройки для View с GUID '" + guid.ToString() + "' не найдены в UserSettingsManager (ChildViews).");
		return it->second;
	}

	const ViewUserData& UserSettingsManager::GetIndependentViewUserData(const Guid& guid) const
	{
		auto it = m_IndependentViewsUserData.find(guid);
		ensure(it != m_IndependentViewsUserData.end(), "Пользовательские настройки для View с GUID '" + guid.ToString() + "' не найдены в UserSettingsManager (IndependentViews).");
		return it->second;
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
			Serializer serializer;
			if (auto res = serializer.Serialize(buffer, *this); !res)
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
		DOut("[UserSettingsManager] Конфигурация сохранена: {}.", m_ConfigPath.string());

		return {};
#endif // Z_EDITOR
	}

	std::expected<void, std::string> UserSettingsManager::LoadConfig(std::filesystem::path path)
	{
		try
		{
			if (!std::filesystem::exists(path))
				return UNEXPECTED("Файл конфигурации не существует: {}", path.string());

			std::ifstream file(path, std::ios::binary);
			if (!file.is_open())
				return UNEXPECTED("Не удалось открыть файл конфигурации: {}", path.string());

			const auto fileSize = std::filesystem::file_size(path);
			std::vector<std::byte> buffer(fileSize);
			file.read(reinterpret_cast<char*>(buffer.data()), fileSize);
			if (!file.good())
				return UNEXPECTED("Не удалось прочитать файл конфигурации: {}", path.string());

			std::size_t offset = 0;

			Serializer serializer;
			auto result = serializer.Deserialize(
				std::span(
					reinterpret_cast<const std::byte*>(buffer.data()),
					buffer.size()),
				offset,
				*this);

			if (!result)
				return UNEXPECTED("Не удалось десериализовать конфигурацию: {}", result.error());

			// Санитария состояния стартового окна для релиза: Closed или Minimized исправление на Normal
			auto& startPlatformData = m_PrimaryViewUserData.GetPlatformData();
			auto state = startPlatformData.GetWindowState();
			if (state == eWindowState::Closed || state == eWindowState::Minimized)
			{
				DOutWarning("[UserSettingsManager] Зафиксирован невалидный статус стартового окна ('{}'). Автоматический сброс на 'Normal'.", EnumToString::ToString(state));
				startPlatformData.SetWindowState(eWindowState::Normal);
			}
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

	void UserSettingsManager::StoreViewState(const View& view)
	{
		const ViewWindowState viewState = view.GetState();
		const Guid& guid = viewState.GetViewGuid();
		const NativeWindowState& navState = viewState.GetNativeState();

		if (m_PrimaryViewUserData.GetViewGuid() == guid)
		{
			auto& platformData = m_PrimaryViewUserData.GetPlatformData();
			platformData.SetWindowRect(navState.GetWindowRect());
			platformData.SetMonitorId(navState.GetMonitorId());

			auto state = navState.GetState();
			if (state != eWindowState::Closed && state != eWindowState::Minimized)
				platformData.SetWindowState(state);

			m_IsDirty = true;
			return;
		}

		if (auto it = m_ChildViewsUserData.find(guid); it != m_ChildViewsUserData.end())
		{
			it->second.GetWindowState() = viewState;
			m_IsDirty = true;
			return;
		}

		if (auto it = m_IndependentViewsUserData.find(guid); it != m_IndependentViewsUserData.end())
		{
			it->second.GetWindowState() = viewState;
			m_IsDirty = true;
			return;
		}

		THROW_RUNTIME("Не удалось сохранить состояние окна: View с GUID {} не найдено в конфигурации пользователя.", guid.ToString());
	}

	[[nodiscard]] std::expected<void, std::string> UserSettingsManager::Serialize(std::vector<std::byte>& buffer, const Serializer& s) const
	{
		auto res = s.Serialize(buffer, c_ConfigHeader)
			.and_then([&]() { return s.Serialize(buffer, m_Version); })
			.and_then([&]() { return s.Serialize(buffer, m_PrimaryViewUserData); });

		if (!res)
			return res;

		zU32 childCount = static_cast<zU32>(m_ChildViewsUserData.size());
		res = s.Serialize(buffer, childCount);
		if (!res)
			return res;

		for (const auto& [guid, item] : m_ChildViewsUserData)
		{
			res = s.Serialize(buffer, item);
			if (!res)
				return res;
		}

		zU32 independentCount = static_cast<zU32>(m_IndependentViewsUserData.size());
		res = s.Serialize(buffer, independentCount);
		if (!res)
			return res;

		for (const auto& [guid, item] : m_IndependentViewsUserData)
		{
			res = s.Serialize(buffer, item);
			if (!res)
				return res;
		}

		return s.Serialize(buffer, m_SelectedGpuId);
	}

	[[nodiscard]] std::expected<void, std::string> UserSettingsManager::Deserialize(std::span<const std::byte> buffer, std::size_t& offset, const Serializer& s)
	{
		FileHeader<3> header{};

		auto res = s.Deserialize(buffer, offset, header)
			.and_then([&]() -> std::expected<void, std::string>
				{
					if (header != c_ConfigHeader)
						return UNEXPECTED("Некорректный заголовок конфигурации.");

					return s.Deserialize(buffer, offset, m_Version);
				})
			.and_then([&]() { return s.Deserialize(buffer, offset, m_PrimaryViewUserData); });

		if (!res)
			return res;

		zU32 childCount = 0;
		res = s.Deserialize(buffer, offset, childCount);
		if (!res)
			return res;

		m_ChildViewsUserData.clear();
		m_ChildViewsUserData.reserve(childCount);
		for (zU32 i = 0; i < childCount; ++i)
		{
			ViewUserData item;
			res = s.Deserialize(buffer, offset, item);
			if (!res)
				return res;
			Guid guid = item.GetViewGuid();
			m_ChildViewsUserData.emplace(std::move(guid), std::move(item));
		}

		zU32 independentCount = 0;
		res = s.Deserialize(buffer, offset, independentCount);
		if (!res)
			return res;

		m_IndependentViewsUserData.clear();
		m_IndependentViewsUserData.reserve(independentCount);
		for (zU32 i = 0; i < independentCount; ++i)
		{
			ViewUserData item;
			res = s.Deserialize(buffer, offset, item);
			if (!res)
				return res;
			Guid guid = item.GetViewGuid();
			m_IndependentViewsUserData.emplace(std::move(guid), std::move(item));
		}

		return s.Deserialize(buffer, offset, m_SelectedGpuId);
	}

#pragma region Logging
	void UserSettingsManager::LogUserData() const
	{
#if Z_ADD_LOGGER || Z_DEVELOPMENT_BUILD
		DOut("========== [UserSettingsManager] User Data: {} ==========", m_ConfigPath.string());
		m_PrimaryViewUserData.LogFileBlock("  ");
		for (const auto& [guid, viewData] : m_ChildViewsUserData)
		{
			viewData.LogFileBlock("  ");
		}
		for (const auto& [guid, viewData] : m_IndependentViewsUserData)
		{
			viewData.LogFileBlock("  ");
		}
		if (!m_SelectedGpuId.empty())
		{
			DOut("  selectedGpuId: {}", m_SelectedGpuId);
		}
#endif
	}
#pragma endregion
}
