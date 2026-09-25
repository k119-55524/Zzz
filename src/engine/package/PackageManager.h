#pragma once

#include <string>
#include <expected>
#include <string_view>
#include <unordered_map>

#include "core/enums/ePackageDatType.h"
#include "core/io/package/PackageEntry.h"
#include "core/io/package/ArchiveReaderBase.h"
#include "core/io/package/ProjectManifestData.h"
#include "core/io/package/views/PrimaryViewData.h"

namespace zzz::engine
{
	using namespace zzz::core;
	/**
	 * @class PackageManager
	 * @brief Менеджер для чтения ресурсов уровня движка/проекта из архива package.dat.
	 * @details Предоставляет доступ к манифесту проекта, настройкам окон и сценам.
	 */
	class PackageManager final : public ArchiveReaderBase<ePackageDatType>
	{
	public:
		PackageManager() = delete;

		/**
		 * @brief Конструктор менеджера пакетов package.dat.
		 * @param physicalPath Физический путь к файлу архива на диске.
		 * @param nativeData Опциональный указатель на платформенные данные приложения (для Android/AssetManager).
		 */
		explicit PackageManager(
			const std::filesystem::path& physicalPath,
			NativeAppData* nativeData = nullptr);
		~PackageManager() override = default;

		using ArchiveReaderBase<ePackageDatType>::GetEntry;
		using ArchiveReaderBase<ePackageDatType>::HasEntry;

		/**
		 * @brief Поиск записи пакета по общему типу ресурса движка.
		 * @param type Общий тип ресурса движка (eEngineResourceType).
		 * @param guid Уникальный идентификатор ресурса.
		 * @return Указатель на PackageEntry, или nullptr, если запись не найдена или тип не относится к package.dat.
		 */
		[[nodiscard]] const PackageEntry* GetEntry(eEngineResourceType type, const Guid& guid) const noexcept
		{
			const auto packageDatType = TryToPackageDatType(type);
			if (!packageDatType)
				return nullptr;
			return ArchiveReaderBase<ePackageDatType>::GetEntry(*packageDatType, guid);
		}

		/**
		 * @brief Проверка наличия записи пакета по общему типу ресурса движка.
		 * @param type Общий тип ресурса движка (eEngineResourceType).
		 * @param guid Уникальный идентификатор ресурса.
		 * @return true, если запись присутствует в package.dat.
		 */
		[[nodiscard]] bool HasEntry(eEngineResourceType type, const Guid& guid) const noexcept
		{
			return GetEntry(type, guid) != nullptr;
		}

		/**
		 * @brief Возвращает кэшированный манифест проекта.
		 * @return Константная ссылка на ProjectManifestData.
		 */
		[[nodiscard]] const ProjectManifestData& GetProjectManifestData() const noexcept { return m_ProjectManifest; }

		/**
		 * @brief Загружает и десериализует данные главного окна (PrimaryViewData).
		 * @return PrimaryViewData в случае успеха, либо строка с описанием ошибки.
		 */
		[[nodiscard]] std::expected<PrimaryViewData, std::string> GetPrimaryViewData() const;

		/**
		 * @brief Возвращает имя компании-разработчика из манифеста проекта.
		 */
		[[nodiscard]] const std::string& GetCompanyName() const noexcept { return m_ProjectManifest.GetCompanyName(); }

		/**
		 * @brief Возвращает имя приложения/игры из манифеста проекта.
		 */
		[[nodiscard]] const std::string& GetAppName() const noexcept { return m_ProjectManifest.GetAppName(); }

		/**
		 * @brief Поиск записи ресурса по GUID для типизированных структур (определяющих c_PackageType).
		 * @tparam T Тип десериализуемой структуры ассета, содержащий static constexpr ePackageDatType c_PackageType.
		 * @param guid Уникальный идентификатор ресурса.
		 * @return Указатель на PackageEntry или nullptr, если запись не найдена.
		 */
		template <typename T>
		[[nodiscard]] const PackageEntry* GetEntry(const Guid& guid) const noexcept
		{
			static_assert(requires { { T::c_PackageType } -> std::convertible_to<ePackageDatType>; },
				"T must define static constexpr ePackageDatType c_PackageType");
			return ArchiveReaderBase<ePackageDatType>::GetEntry<T::c_PackageType>(guid);
		}

		/**
		 * @brief Загружает и десериализует ресурс заданного типа из архива package.dat.
		 * @tparam T Тип десериализуемого ассета.
		 * @param guid Уникальный идентификатор ресурса.
		 * @return Экземпляр T в случае успеха, либо строка с описанием ошибки.
		 */
		template <typename T>
		[[nodiscard]] std::expected<T, std::string> LoadAsset(const Guid& guid) const
		{
			static_assert(requires { { T::c_PackageType } -> std::convertible_to<ePackageDatType>; },
				"T must define static constexpr ePackageDatType c_PackageType");

			constexpr ePackageDatType type = T::c_PackageType;
			ensure(ArchiveTraits<ePackageDatType>::IsTypeAllowed(type), "Тип ассета недопустим в package.dat");

			const auto* entry = GetEntry<type>(guid);
			if (!entry)
				return UNEXPECTED("Запись пакета с типом {} и GUID '{}' не найдена.", ToString(type), guid.ToString());

			return DeserializeEntryRaw<T>(*entry);
		}

		/**
		 * @brief Поиск GUID сцены по её имени из списка зарегистрированных в манифесте сцен.
		 * @param name Имя сцены.
		 * @return GUID сцены, если сцена найдена, иначе std::nullopt.
		 */
		[[nodiscard]] std::optional<Guid> FindSceneGuidByName(std::string_view name) const noexcept;

	private:
		void LogEntryDetails(const PackageEntry& entry) const override;

		struct StringHash
		{
			using is_transparent = void;
			[[nodiscard]] size_t operator()(std::string_view sv) const noexcept { return std::hash<std::string_view>{}(sv); }
		};
		std::unordered_map<std::string, Guid, StringHash, std::equal_to<>> m_SceneGuidsByName;
		ProjectManifestData m_ProjectManifest{};
	};
}
