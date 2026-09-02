#pragma once

#include "engine/EngineIncludes.h"
#include "core/io/FileSystem.h"
#include "core/io/package/ViewUserData.h"
#include "core/io/package/PrimaryViewUserData.h"

using namespace zzz::core;

namespace zzz::engine
{
	class View;

	using ViewUserDataMap = std::unordered_map<Guid, ViewUserData>;

	class UserSettingsManager final : public ISerializable
	{
	public:
		UserSettingsManager() = delete;
		explicit UserSettingsManager(std::shared_ptr<FileSystem> fileSystem);

		[[nodiscard]] inline const std::string& GetSelectedGpuId() const noexcept { return m_SelectedGpuId; }
		/**
		 * @brief Возвращает пользовательские настройки Основного окна ( PrimaryViewUserData ).
		 * @return Указатель на настройки, или nullptr, если настройки ещё не созданы в user.dat.
		 */
		[[nodiscard]] const PrimaryViewUserData* GetPrimaryViewUserData() const noexcept;

		/**
		 * @brief Возвращает пользовательские настройки Дочернего окна ( ChildView ).
		 * @param guid Уникальный идентификатор окна.
		 * @return Указатель на настройки, или nullptr, если окно открывается впервые и отсутствует в user.dat.
		 */
		[[nodiscard]] const ViewUserData* GetChildViewUserData(const Guid& guid) const noexcept;

		/**
		 * @brief Возвращает пользовательские настройки Независимого окна ( IndependentView ).
		 * @param guid Уникальный идентификатор окна.
		 * @return Указатель на настройки, или nullptr, если окно открывается впервые и отсутствует в user.dat.
		 */
		[[nodiscard]] const ViewUserData* GetIndependentViewUserData(const Guid& guid) const noexcept;

		/**
		 * @brief Возвращает все сохранённые в user.dat Дочерние окна (guid -> состояние).
		 * Используется при старте для восстановления набора открытых Child-окон прошлой сессии.
		 */
		[[nodiscard]] const ViewUserDataMap& GetChildViewsUserData() const noexcept { return m_ChildViewsUserData; }

		/**
		 * @brief Возвращает все сохранённые в user.dat Независимые окна (guid -> состояние).
		 * Используется при старте для восстановления набора открытых Independent-окон прошлой сессии.
		 */
		[[nodiscard]] const ViewUserDataMap& GetIndependentViewsUserData() const noexcept { return m_IndependentViewsUserData; }

		[[nodiscard]] ViewPlatformData* GetOrCreatePrimaryViewPlatformData(const Guid& guid, const ViewPlatformData& defaultData);
		[[nodiscard]] ViewPlatformData* GetOrCreateChildViewPlatformData(const Guid& guid, const ViewPlatformData& defaultData);
		[[nodiscard]] ViewPlatformData* GetOrCreateIndependentViewPlatformData(const Guid& guid, const ViewPlatformData& defaultData);

		/**
		 * @brief Удаляет из user.dat протухшую запись Дочернего окна - ассет с таким guid больше не существует
		 * в package.dat (удалён из проекта), восстанавливать нечего. Безопасно вызывать в любой момент - в
		 * отличие от закрытия окна пользователем (см. View::HandleWindowClose - там мы намеренно НЕ удаляем,
		 * а помечаем Closed, чтобы сохранить geometry на случай повторного открытия), тут восстанавливать
		 * уже нечего - самого определения окна в проекте больше нет.
		 */
		void RemoveChildViewUserData(const Guid& guid);

		/** @brief Аналогично RemoveChildViewUserData(), но для Независимых окон. */
		void RemoveIndependentViewUserData(const Guid& guid);

		/**
		 * @brief Проверяет, запущен ли движок впервые (отсутствовал файл user.dat).
		 */
		[[nodiscard]] inline bool IsFirstRun() const noexcept { return m_IsFirstRun; }

		void SetSelectedGpuId(std::string gpuId);

		/** @brief Проверяет, отключена ли категория логирования пользователем (persisted, см. user.dat). */
		[[nodiscard]] bool IsLogCategoryDisabled(std::string_view categoryName) const;

		/**
		 * @brief Включает/выключает категорию логирования по имени: сохраняет решение в user.dat (opt-out список
		 * отключённых категорий) и сразу применяет его к zzz::logger::g_Logger.
		 */
		void SetLogCategoryEnabled(std::string_view categoryName, bool enabled);

		/**
		 * @brief Запоминает или обновляет геометрическое состояние (размер, позиция, монитор) указанного View в user.dat.
		 * Если окна ещё нет в конфигурации пользователя, создаёт новую запись.
		 * @param view Ссылка на экземпляр окна.
		 */
		void StoreViewState(const View& view);

		[[nodiscard]] std::expected<void, std::string> SaveConfig();

	private:
		void Initialize();
		void LogUserData() const;
		void SetDefaultUserSettings();
		/** @brief Применяет persisted-список отключённых категорий логирования к zzz::logger::g_Logger (вызывается после Initialize()). */
		void ApplyLogCategorySettings() const;
		std::expected<void, std::string> LoadConfig();

		[[nodiscard]] std::expected<void, std::string> Serialize(std::vector<std::byte>& buffer, const Serializer& s) const override;
		[[nodiscard]] std::expected<void, std::string> Deserialize(std::span<const std::byte> buffer, std::size_t& offset, const Serializer& s) override;

		std::shared_ptr<FileSystem> m_FileSystem;

		Version m_Version;
		std::optional<PrimaryViewUserData> m_PrimaryViewUserData;
		ViewUserDataMap m_ChildViewsUserData;
		ViewUserDataMap m_IndependentViewsUserData;
		std::string m_SelectedGpuId;
		std::unordered_set<std::string> m_DisabledLogCategories;

		bool m_IsDirty;
		bool m_IsFirstRun = true;
	};
}
