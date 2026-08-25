#pragma once

#include "engine/EngineIncludes.h"

namespace zzz::core
{
	class ViewUserData;
}

using namespace zzz::core;

namespace zzz::engine
{
	class View;

	using ViewUserDataMap = std::unordered_map<Guid, ViewUserData>;

	class UserSettingsManager final : public ISerializable
	{
	public:
		UserSettingsManager() = delete;
		UserSettingsManager(const Path& path);

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
		 * @brief Проверяет, запущен ли движок впервые (отсутствовал файл user.dat).
		 */
		[[nodiscard]] inline bool IsFirstRun() const noexcept { return m_IsFirstRun; }

		void SetSelectedGpuId(std::string gpuId);

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
		std::expected<std::filesystem::path, std::string> GetSettingsDirectory();
		std::expected<void, std::string> LoadConfig(std::filesystem::path path);

		[[nodiscard]] std::expected<void, std::string> Serialize(std::vector<std::byte>& buffer, const Serializer& s) const override;
		[[nodiscard]] std::expected<void, std::string> Deserialize(std::span<const std::byte> buffer, std::size_t& offset, const Serializer& s) override;

		Path m_Path;
		std::filesystem::path m_ConfigPath;

		Version m_Version;
		std::optional<PrimaryViewUserData> m_PrimaryViewUserData;
		ViewUserDataMap m_ChildViewsUserData;
		ViewUserDataMap m_IndependentViewsUserData;
		std::string m_SelectedGpuId;

		bool m_IsDirty;
		bool m_IsFirstRun = true;
	};
}
