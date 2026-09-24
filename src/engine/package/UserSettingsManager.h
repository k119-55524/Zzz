#pragma once

#include "core/io/DatFileHeader.h"
#include "core/io/storage/ReadWriteFile.h"
#include "core/io/package/views/ViewUserData.h"
#include "core/io/package/views/PrimaryViewUserData.h"

using namespace zzz::core;

namespace zzz::core
{
	class DataAssetsManager;
}

namespace zzz::engine
{
	class View;
	class PackageManager;

	using ViewUserDataMap = std::unordered_map<Guid, ViewUserData>;

	class UserSettingsManager final : public ISerializable
	{
	public:
		UserSettingsManager() = delete;
		explicit UserSettingsManager(const std::filesystem::path& configPath);
		~UserSettingsManager() override = default;

		/**
		 * @brief Валидирует сохранённые идентификаторы окон по записям PackageManager (package.dat)
		 *        и DataAssetsManager (data.dat).
		 * @details Удаляет из конфигурации пользователя окна, чьи GUID отсутствуют в пакетах или
		 *          не соответствуют требуемому типу (PrimaryView, ChildView, IndependentView),
		 *          предотвращая падения движка на старте при изменении/удалении ресурсов разработчиком.
		 */
		void ValidateAgainstPackages(const PackageManager& packageManager, const DataAssetsManager* dataAssetsManager = nullptr);

		[[nodiscard]] inline const std::string& GetSelectedGpuId() const noexcept { return m_SelectedGpuId; }
		[[nodiscard]] inline const std::filesystem::path& GetPath() const noexcept { return m_File.GetPath(); }

		/**
		 * @brief Возвращает пользовательские настройки Основного окна (PrimaryViewUserData).
		 * @return Указатель на настройки, или nullptr, если настройки ещё не созданы в cfg.dat.
		 */
		[[nodiscard]] const PrimaryViewUserData* GetPrimaryViewUserData() const noexcept;

		/**
		 * @brief Возвращает пользовательские настройки Дочернего окна (ChildView).
		 * @param guid Уникальный идентификатор окна.
		 * @return Указатель на настройки, или nullptr, если окно открывается впервые и отсутствует в cfg.dat.
		 */
		[[nodiscard]] const ViewUserData* GetChildViewUserData(const Guid& guid) const noexcept;

		/**
		 * @brief Возвращает пользовательские настройки Независимого окна (IndependentView).
		 * @param guid Уникальный идентификатор окна.
		 * @return Указатель на настройки, или nullptr, если окно открывается впервые и отсутствует в cfg.dat.
		 */
		[[nodiscard]] const ViewUserData* GetIndependentViewUserData(const Guid& guid) const noexcept;

		/**
		 * @brief Возвращает все сохранённые в cfg.dat Дочерние окна (guid -> состояние).
		 * Используется при старте для восстановления набора открытых Child-окон прошлой сессии.
		 */
		[[nodiscard]] const ViewUserDataMap& GetChildViewsUserData() const noexcept { return m_ChildViewsUserData; }

		/**
		 * @brief Возвращает все сохранённые в cfg.dat Независимые окна (guid -> состояние).
		 * Используется при старте для восстановления набора открытых Independent-окон прошлой сессии.
		 */
		[[nodiscard]] const ViewUserDataMap& GetIndependentViewsUserData() const noexcept { return m_IndependentViewsUserData; }

		[[nodiscard]] ViewPlatformData* GetOrCreatePrimaryViewPlatformData(const Guid& guid, const ViewPlatformData& defaultData);
		[[nodiscard]] ViewPlatformData* GetOrCreateChildViewPlatformData(const Guid& guid, const ViewPlatformData& defaultData);
		[[nodiscard]] ViewPlatformData* GetOrCreateIndependentViewPlatformData(const Guid& guid, const ViewPlatformData& defaultData);

		void SetSelectedGpuId(std::string gpuId);

		/**
		 * @brief Запоминает или обновляет геометрическое состояние (размер, позиция, монитор) указанного View в cfg.dat.
		 * Если окна ещё нет в конфигурации пользователя, создаёт новую запись.
		 * @param view Ссылка на экземпляр окна.
		 */
		void StoreViewState(const View& view);

		[[nodiscard]] std::expected<void, std::string> SaveConfig();

	private:
		void Initialize();
		void LogUserData() const;
		void SetDefaultUserSettings();
		std::expected<void, std::string> LoadConfig(std::span<const std::byte> buffer);

		[[nodiscard]] std::expected<void, std::string> Serialize(std::vector<std::byte>& buffer, const Serializer& s) const override;
		[[nodiscard]] std::expected<void, std::string> Deserialize(std::span<const std::byte> buffer, std::size_t& offset, const Serializer& s) override;

		ReadWriteFile m_File;

		DatFileHeader m_Header;
		std::optional<PrimaryViewUserData> m_PrimaryViewUserData;
		ViewUserDataMap m_ChildViewsUserData;
		ViewUserDataMap m_IndependentViewsUserData;
		std::string m_SelectedGpuId;

		bool m_IsDirty;
	};
}
