
#include "PackageManager.h"
#include "UserSettingsManager.h"
#include "core/io/package/ViewUserData.h"
#include "engine/view/View.h"

Z_SET_LOG_CATEGORY(::zzz::core::Assets);

using namespace zzz::core;
using zzz::core::PrimaryViewUserData;

namespace zzz::engine
{
	UserSettingsManager::UserSettingsManager(const Path& path) :
		m_Path(path),
		m_Version(c_ConfigFileMajorVersion, c_ConfigFileMinorVersion, c_ConfigFilePatchVersion),
		m_IsDirty(true)
	{
#if Z_EDITOR
#else
		Initialize();
		ApplyLogCategorySettings();
		LogUserData();
#endif
	}

	void UserSettingsManager::Initialize()
	{
		try
		{
			m_ConfigPath = m_Path.GetUserDatPath()
				.lexically_normal()
				.make_preferred();

			SetDefaultUserSettings();
			if (!std::filesystem::exists(m_ConfigPath))
			{
				DOutWarning("Файл конфигурации не найден: {}. Используется конфигурация по умолчанию.", m_ConfigPath.string());
				return;
			}

			auto res = LoadConfig(m_ConfigPath);
			if (!res)
			{
				DOutWarning("Не удалось загрузить файл конфигурации: {}. Создаётся конфигурация по умолчанию.", m_ConfigPath.string());
				SetDefaultUserSettings();
			}
			else
			{
				m_IsFirstRun = false;
			}
		}
		catch (const std::filesystem::filesystem_error& e)
		{
			DOutException("Ошибка файловой системы: {}. Установка конфигурации по умолчанию.", e.what());
			SetDefaultUserSettings();
			return;
		}
		catch (const std::exception& e)
		{
			DOutException("Ошибка загрузки конфигурации: {}. Установка конфигурации по умолчанию.", e.what());
			SetDefaultUserSettings();
			return;
		}
		catch (...)
		{
			DOutException("Неизвестная ошибка загрузки конфигурации. Установка конфигурации по умолчанию.");
			SetDefaultUserSettings();
			return;
		}

		DOut("[UserSettingsManager] Конфигурация десериализована: {}.", m_ConfigPath.string());
	}

	void UserSettingsManager::SetDefaultUserSettings()
	{
		m_Version = Version(c_ConfigFileMajorVersion, c_ConfigFileMinorVersion, c_ConfigFilePatchVersion);
		m_PrimaryViewUserData.reset();
	}

	const PrimaryViewUserData* UserSettingsManager::GetPrimaryViewUserData() const noexcept
	{
		return m_PrimaryViewUserData.has_value() ? &m_PrimaryViewUserData.value() : nullptr;
	}

	void UserSettingsManager::SetSelectedGpuId(std::string gpuId)
	{
		if (m_SelectedGpuId != gpuId)
		{
			m_SelectedGpuId = std::move(gpuId);
			m_IsDirty = true;
		}
	}

	bool UserSettingsManager::IsLogCategoryDisabled(std::string_view categoryName) const
	{
		return m_DisabledLogCategories.contains(std::string(categoryName));
	}

	void UserSettingsManager::SetLogCategoryEnabled(std::string_view categoryName, bool enabled)
	{
		if (enabled)
		{
			if (m_DisabledLogCategories.erase(std::string(categoryName)) > 0)
				m_IsDirty = true;
		}
		else
		{
			auto [it, inserted] = m_DisabledLogCategories.insert(std::string(categoryName));
			(void)it;
			if (inserted)
				m_IsDirty = true;
		}

		zzz::logger::g_Logger.SetCategoryEnabled(categoryName, enabled);
	}

	void UserSettingsManager::ApplyLogCategorySettings() const
	{
		for (const auto& name : m_DisabledLogCategories)
		{
			zzz::logger::g_Logger.SetCategoryEnabled(name, false);
		}
	}

	const ViewUserData* UserSettingsManager::GetChildViewUserData(const Guid& guid) const noexcept
	{
		auto it = m_ChildViewsUserData.find(guid);
		if (it != m_ChildViewsUserData.end())
			return &it->second;
		return nullptr;
	}

	const ViewUserData* UserSettingsManager::GetIndependentViewUserData(const Guid& guid) const noexcept
	{
		auto it = m_IndependentViewsUserData.find(guid);
		if (it != m_IndependentViewsUserData.end())
			return &it->second;
		return nullptr;
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
				return UNEXPECTED("Не удалось записать файл: {}.", m_ConfigPath.string());
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
			if (m_PrimaryViewUserData)
			{
				auto& startPlatformData = m_PrimaryViewUserData->GetPlatformData();
				auto state = startPlatformData.GetWindowState();
				if (state == eWindowState::Closed || state == eWindowState::Minimized)
				{
					DOutWarning("[UserSettingsManager] Зафиксирован невалидный статус стартового окна ('{}'). Автоматический сброс на 'Normal'.", EnumToString::ToString(state));
					startPlatformData.SetWindowState(eWindowState::Normal);
					m_IsDirty = true;
				}
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

	ViewPlatformData* UserSettingsManager::GetOrCreatePrimaryViewPlatformData(const Guid& guid, const ViewPlatformData& defaultData)
	{
		if (!m_PrimaryViewUserData || m_PrimaryViewUserData->GetViewGuid() != guid)
		{
			m_PrimaryViewUserData = PrimaryViewUserData(guid, defaultData);
		}
		m_IsDirty = true;
		return &m_PrimaryViewUserData->GetPlatformData();
	}

	ViewPlatformData* UserSettingsManager::GetOrCreateChildViewPlatformData(const Guid& guid, const ViewPlatformData& defaultData)
	{
		auto it = m_ChildViewsUserData.find(guid);
		if (it == m_ChildViewsUserData.end())
		{
			NativeWindowState navState(defaultData.GetMonitorId(), defaultData.GetWindowRect(), defaultData.GetWindowState());
			ViewWindowState viewState(guid, navState);
			it = m_ChildViewsUserData.emplace(guid, ViewUserData(viewState)).first;
		}
		m_IsDirty = true;
		return &it->second.GetPlatformDataRef();
	}

	ViewPlatformData* UserSettingsManager::GetOrCreateIndependentViewPlatformData(const Guid& guid, const ViewPlatformData& defaultData)
	{
		auto it = m_IndependentViewsUserData.find(guid);
		if (it == m_IndependentViewsUserData.end())
		{
			NativeWindowState navState(defaultData.GetMonitorId(), defaultData.GetWindowRect(), defaultData.GetWindowState());
			ViewWindowState viewState(guid, navState);
			it = m_IndependentViewsUserData.emplace(guid, ViewUserData(viewState)).first;
		}
		m_IsDirty = true;
		return &it->second.GetPlatformDataRef();
	}

	void UserSettingsManager::StoreViewState(const View& view)
	{
		const ViewWindowState viewState = view.GetState();
		const Guid& guid = viewState.GetViewGuid();
		const NativeWindowState& navState = viewState.GetNativeState();
		const eWindowState state = navState.GetState();

		if (!m_PrimaryViewUserData || m_PrimaryViewUserData->GetViewGuid() == guid)
		{
			if (!m_PrimaryViewUserData)
			{
				ViewPlatformData pd;
				pd.SetWindowRect(navState.GetWindowRect());
				pd.SetMonitorId(navState.GetMonitorId());
				if (state != eWindowState::Closed && state != eWindowState::Minimized)
					pd.SetWindowState(state);
				m_PrimaryViewUserData = PrimaryViewUserData(guid, pd);
			}
			else
			{
				auto& platformData = m_PrimaryViewUserData->GetPlatformData();
				platformData.SetWindowRect(navState.GetWindowRect());
				platformData.SetMonitorId(navState.GetMonitorId());

				if (state != eWindowState::Closed && state != eWindowState::Minimized)
					platformData.SetWindowState(state);
			}

			m_IsDirty = true;
			return;
		}

		if (auto it = m_ChildViewsUserData.find(guid); it != m_ChildViewsUserData.end())
		{
			it->second.GetWindowState().GetNativeState().SetWindowRect(navState.GetWindowRect());
			it->second.GetWindowState().GetNativeState().SetMonitorId(navState.GetMonitorId());
			if (state != eWindowState::Closed && state != eWindowState::Minimized)
				it->second.GetWindowState().GetNativeState().SetState(state);
			m_IsDirty = true;
			return;
		}

		if (auto it = m_IndependentViewsUserData.find(guid); it != m_IndependentViewsUserData.end())
		{
			it->second.GetWindowState().GetNativeState().SetWindowRect(navState.GetWindowRect());
			it->second.GetWindowState().GetNativeState().SetMonitorId(navState.GetMonitorId());
			if (state != eWindowState::Closed && state != eWindowState::Minimized)
				it->second.GetWindowState().GetNativeState().SetState(state);
			m_IsDirty = true;
			return;
		}

		THROW_RUNTIME("Не удалось сохранить состояние окна: View с GUID {} не найдено в конфигурации пользователя.", guid.ToString());
	}

	[[nodiscard]] std::expected<void, std::string> UserSettingsManager::Serialize(std::vector<std::byte>& buffer, const Serializer& s) const
	{
		auto res = s.Serialize(buffer, c_ConfigHeader)
			.and_then([&]() { return s.Serialize(buffer, m_Version); })
			.and_then([&]() -> std::expected<void, std::string> {
				bool hasPrimary = m_PrimaryViewUserData.has_value();
				auto resP = s.Serialize(buffer, hasPrimary);
				if (!resP) return resP;
				if (hasPrimary)
					return s.Serialize(buffer, *m_PrimaryViewUserData);
				return {};
			});

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

		auto resGpu = s.Serialize(buffer, m_SelectedGpuId);
		if (!resGpu)
			return resGpu;

		zU32 disabledCatCount = static_cast<zU32>(m_DisabledLogCategories.size());
		auto resCount = s.Serialize(buffer, disabledCatCount);
		if (!resCount)
			return resCount;

		for (const auto& name : m_DisabledLogCategories)
		{
			auto resName = s.Serialize(buffer, name);
			if (!resName)
				return resName;
		}

		return {};
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
			.and_then([&]() -> std::expected<void, std::string> {
				bool hasPrimary = false;
				auto resP = s.Deserialize(buffer, offset, hasPrimary);
				if (!resP) return resP;
				if (hasPrimary)
				{
					PrimaryViewUserData data;
					auto resD = s.Deserialize(buffer, offset, data);
					if (!resD) return resD;
					m_PrimaryViewUserData = std::move(data);
				}
				else
				{
					m_PrimaryViewUserData.reset();
				}
				return {};
			});

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

		auto resGpu = s.Deserialize(buffer, offset, m_SelectedGpuId);
		if (!resGpu)
			return resGpu;

		m_DisabledLogCategories.clear();
		zU32 disabledCatCount = 0;
		auto resCount = s.Deserialize(buffer, offset, disabledCatCount);
		if (!resCount)
			return resCount;

		for (zU32 i = 0; i < disabledCatCount; ++i)
		{
			std::string name;
			auto resName = s.Deserialize(buffer, offset, name);
			if (!resName)
				return resName;
			m_DisabledLogCategories.insert(std::move(name));
		}

		return {};
	}

#pragma region Logging
	void UserSettingsManager::LogUserData() const
	{
#if Z_ADD_LOGGER
		DOut("========== [UserSettingsManager] User Data: {} ==========", m_ConfigPath.string());
		if (m_PrimaryViewUserData)
			m_PrimaryViewUserData->LogFileBlock("  ");
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
