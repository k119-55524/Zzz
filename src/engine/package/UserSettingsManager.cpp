
#include "engine/view/View.h"
#include "core/utils/Ensure.h"
#include "engine/package/PackageManager.h"
#include "core/io/storage/ReadWriteFile.h"
#include "core/constants/PackagesConstants.h"
#include "core/io/package/DataAssetsManager.h"
#include "core/io/package/views/ViewUserData.h"

#include "UserSettingsManager.h"

Z_SET_LOG_CATEGORY(::zzz::core::Assets);

using namespace zzz::core;

namespace zzz::engine
{
	UserSettingsManager::UserSettingsManager(const std::filesystem::path& configPath) :
		m_ConfigPath{ configPath },
		m_File{ configPath },
		m_Header{},
		m_IsDirty{ false }
	{
#if Z_EDITOR
#else
		Initialize();
		LogUserData();
#endif
	}

	void UserSettingsManager::ValidateAgainstPackages(const PackageManager& packageManager, const DataAssetsManager* /*dataAssetsManager*/)
	{
		// Валидация первичного окна
		if (m_PrimaryViewUserData.has_value())
		{
			const Guid& guid = m_PrimaryViewUserData->GetViewGuid();
			if (!packageManager.HasEntry(ePackageDatType::PrimaryView, guid))
			{
				DOutWarning("[UserSettingsManager] Сохранённое первичное окно (GUID: {}) не найдено как PrimaryView в пакете. Запись сброшена.", guid.ToString());
				m_PrimaryViewUserData.reset();
				m_IsDirty = true;
			}
		}

		// Валидация дочерних окон
		for (auto it = m_ChildViewsUserData.begin(); it != m_ChildViewsUserData.end(); )
		{
			const Guid& guid = it->first;
			if (!packageManager.HasEntry(ePackageDatType::ChildView, guid))
			{
				DOutWarning("[UserSettingsManager] Сохранённое дочернее окно (GUID: {}) не найдено как ChildView в пакете. Запись удалена из конфигурации.", guid.ToString());
				it = m_ChildViewsUserData.erase(it);
				m_IsDirty = true;
			}
			else
			{
				++it;
			}
		}

		// Валидация независимых окон
		for (auto it = m_IndependentViewsUserData.begin(); it != m_IndependentViewsUserData.end(); )
		{
			const Guid& guid = it->first;
			if (!packageManager.HasEntry(ePackageDatType::IndependentView, guid))
			{
				DOutWarning("[UserSettingsManager] Сохранённое независимое окно (GUID: {}) не найдено как IndependentView в пакете. Запись удалена из конфигурации.", guid.ToString());
				it = m_IndependentViewsUserData.erase(it);
				m_IsDirty = true;
			}
			else
			{
				++it;
			}
		}
	}

	void UserSettingsManager::Initialize()
	{
		SetDefaultUserSettings();

		try
		{
			// Читаем байтмассив файла из ReadWriteFile
			auto bufferRes = m_File.Read();
			if (!bufferRes)
			{
				DOut("[UserSettingsManager] Конфигурация не загружена ({}). Используется конфигурация по умолчанию.", bufferRes.error());
				return;
			}

			const auto& buffer = *bufferRes;
			if (buffer.empty())
			{
				DOut("[UserSettingsManager] Файл конфигурации пуст: {}. Используется конфигурация по умолчанию.", m_ConfigPath.string());
				return;
			}

			// Проверка минимального размера на базовый заголовок
			if (buffer.size() < DatFileHeader::c_BaseHeaderSize)
			{
				DOutWarning("Файл конфигурации повреждён (размер {} байт меньше заголовка). Создаётся конфигурация по умолчанию.", buffer.size());
				SetDefaultUserSettings();
				return;
			}

			// Анализ и десериализация из байтмассива
			auto res = LoadConfig(buffer);
			if (!res)
			{
				DOutWarning("Не удалось загрузить файл конфигурации: {}. Создаётся конфигурация по умолчанию.", res.error());
				SetDefaultUserSettings();
				return;
			}

			DOut("[UserSettingsManager] Конфигурация десериализована: {}.", m_ConfigPath.string());
		}
		catch (const std::exception& e)
		{
			DOutException("Ошибка загрузки конфигурации: {}. Установка конфигурации по умолчанию.", e.what());
			SetDefaultUserSettings();
		}
		catch (...)
		{
			DOutException("Неизвестная ошибка загрузки конфигурации. Установка конфигурации по умолчанию.");
			SetDefaultUserSettings();
		}
	}

	void UserSettingsManager::SetDefaultUserSettings()
	{
		m_Header = DatFileHeader{ c_UserConfigFormat };
		m_PrimaryViewUserData.reset();
		m_ChildViewsUserData.clear();
		m_IndependentViewsUserData.clear();
		m_SelectedGpuId.clear();
		m_IsDirty = false;
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

		const zU64 saveTime = static_cast<zU64>(
			std::chrono::duration_cast<std::chrono::milliseconds>(
				std::chrono::system_clock::now().time_since_epoch()).count());
		const zU64 prevSaveTime = m_Header.GetTimestamp();

		// DatFileHeader - неизменяемый value-type (нет сеттеров), поэтому обновление timestamp - это
		// пересборка через конструктор с сохранением остальных полей заголовка без изменений.
		auto withTimestamp = [&](zU64 timestamp)
		{
			m_Header = DatFileHeader(c_UserConfigFormat, m_Header.GetEntryCount(), timestamp);
		};

		withTimestamp(saveTime);

		try
		{
			std::vector<std::byte> buffer;
			Serializer serializer;
			if (auto res = serializer.Serialize(buffer, *this); !res)
			{
				withTimestamp(prevSaveTime);
				return UNEXPECTED("Не удалось сериализовать конфигурацию: {}.", res.error());
			}

			if (auto writeRes = m_File.Write(buffer); !writeRes)
			{
				withTimestamp(prevSaveTime);
				return UNEXPECTED("Не удалось сохранить конфигурацию: {}.", writeRes.error());
			}
		}
		catch (const std::exception& e)
		{
			withTimestamp(prevSaveTime);
			return UNEXPECTED("Ошибка сериализации конфигурации: {}.", std::string(e.what()));
		}
		catch (...)
		{
			withTimestamp(prevSaveTime);
			return UNEXPECTED("Неизвестная ошибка сериализации конфигурации.");
		}

		m_IsDirty = false;
		DOut("[UserSettingsManager] Конфигурация сохранена: {}.", m_ConfigPath.string());

		return {};
#endif // Z_EDITOR
	}

	std::expected<void, std::string> UserSettingsManager::LoadConfig(std::span<const std::byte> buffer)
	{
		try
		{
			std::size_t offset = 0;

			Serializer serializer;
			auto result = serializer.Deserialize(buffer, offset, *this);
			if (!result)
				return UNEXPECTED("Не удалось десериализовать конфигурацию: {}", result.error());

			// Санитария состояния стартового окна для релиза: Closed или Minimized исправление на Normal
			bool stateSanitized = false;
			if (m_PrimaryViewUserData)
			{
				auto& startPlatformData = m_PrimaryViewUserData->GetPlatformData();
				auto state = startPlatformData.GetWindowState();
				if (state == eWindowState::Closed || state == eWindowState::Minimized)
				{
					DOutWarning("[UserSettingsManager] Зафиксирован невалидный статус стартового окна ('{}'). Автоматический сброс на 'Normal'.", ToString(state));
					startPlatformData.SetWindowState(eWindowState::Normal);
					stateSanitized = true;
				}
			}
			m_IsDirty = stateSanitized;
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
			m_IsDirty = true;
		}

		ViewPlatformData& platformData = m_PrimaryViewUserData->GetPlatformData();
		// isPrimary не сериализуется (см. комментарий у поля в ViewDataMSWin/... .h) - проставляем
		// заново на каждый запрос. Для Child/Independent не нужно ставить false явно - это значение
		// по умолчанию (см. конструкторы ViewDataXxx), а в m_PrimaryViewUserData никогда не попадают
		// объекты, для которых до этого звали SetPrimary(true) от чужого View.
		platformData.SetPrimary(true);
		return &platformData;
	}

	ViewPlatformData* UserSettingsManager::GetOrCreateChildViewPlatformData(const Guid& guid, const ViewPlatformData& defaultData)
	{
		auto it = m_ChildViewsUserData.find(guid);
		if (it == m_ChildViewsUserData.end())
		{
			NativeWindowState navState(defaultData.GetMonitorId(), defaultData.GetWindowRect(), defaultData.GetWindowState());
			ViewWindowState viewState(guid, navState);
			it = m_ChildViewsUserData.emplace(guid, ViewUserData(viewState)).first;
			m_IsDirty = true;
		}
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
			m_IsDirty = true;
		}
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
			it->second.SyncPlatformDataFromState();
			m_IsDirty = true;
			return;
		}

		if (auto it = m_IndependentViewsUserData.find(guid); it != m_IndependentViewsUserData.end())
		{
			it->second.GetWindowState().GetNativeState().SetWindowRect(navState.GetWindowRect());
			it->second.GetWindowState().GetNativeState().SetMonitorId(navState.GetMonitorId());
			if (state != eWindowState::Closed && state != eWindowState::Minimized)
				it->second.GetWindowState().GetNativeState().SetState(state);
			it->second.SyncPlatformDataFromState();
			m_IsDirty = true;
			return;
		}

		THROW_RUNTIME("Не удалось сохранить состояние окна: View с GUID {} не найдено в конфигурации пользователя.", guid.ToString());
	}

	[[nodiscard]] std::expected<void, std::string> UserSettingsManager::Serialize(std::vector<std::byte>& buffer, const Serializer& s) const
	{
		auto res = s.Serialize(buffer, m_Header)
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

		return s.Serialize(buffer, m_SelectedGpuId);
	}

	[[nodiscard]] std::expected<void, std::string> UserSettingsManager::Deserialize(std::span<const std::byte> buffer, std::size_t& offset, const Serializer& s)
	{
		auto res = s.Deserialize(buffer, offset, m_Header)
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

		return s.Deserialize(buffer, offset, m_SelectedGpuId);
	}

#pragma region Logging
	void UserSettingsManager::LogUserData() const
	{
#if Z_ADD_LOGGER
		DOut("========== [UserSettingsManager] User Data: {} ==========", m_ConfigPath.string());
		m_Header.LogFileBlock("  ");
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
