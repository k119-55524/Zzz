#pragma once

#include <string>
#include <expected>
#include <unordered_map>

#include "core/utils/Guid.h"
#include "core/utils/Export.h"
#include "core/io/FileSystem.h"
#include "core/io/DatFileHeader.h"
#include "core/enums/eResourceType.h"
#include "core/serialize/Serializer.h"
#include "core/io/package/PackageEntry.h"
#include "core/constants/PackagesConstants.h"

namespace zzz::core
{
	/**
	 * @struct AssetLocation
	 * @brief Физические координаты размещения ассета на диске для чтения.
	 */
	struct AssetLocation
	{
		eFileLocation location{ eFileLocation::App };

		struct PathRef
		{
			std::filesystem::path path;
			[[nodiscard]] const std::filesystem::path& get() const noexcept { return path; }
			[[nodiscard]] operator const std::filesystem::path&() const noexcept { return path; }
		} relativePath;

		std::size_t offset = 0;
		std::size_t size = 0;

		struct EntryRef
		{
			const PackageEntry* ptr = nullptr;
			[[nodiscard]] const PackageEntry& operator*() const noexcept { return *ptr; }
			[[nodiscard]] const PackageEntry* operator->() const noexcept { return ptr; }
			[[nodiscard]] const PackageEntry& get() const noexcept { return *ptr; }
			[[nodiscard]] operator const PackageEntry*() const noexcept { return ptr; }
		} entry;
	};

	/**
	 * @class DataAssetsManager
	 * @brief Менеджер для чтения игровых ресурсов из архива по пути c_DataPackageRelativePath.
	 */
	class Z_CORE_API DataAssetsManager final
	{
	public:
		DataAssetsManager() = delete;
		explicit DataAssetsManager(const FileSystem& fileSystem);
		explicit DataAssetsManager(const std::shared_ptr<FileSystem>& fileSystem)
			: DataAssetsManager(*fileSystem)
		{}
		~DataAssetsManager() = default;

		[[nodiscard]] const DatFileHeader& GetHeader() const noexcept { return m_Header; }
		[[nodiscard]] std::expected<AssetLocation, std::string> GetAssetLocation(eResourceType type, const Guid& guid) const;

		template <typename T>
		[[nodiscard]] static std::expected<T, std::string> DeserializeAssetFromMemory(
			const PackageEntry& entry,
			std::span<const std::byte> bytes)
		{
			constexpr eResourceType expectedType = T::c_ResourceType;
			if (entry.GetAssetType() != static_cast<zU32>(expectedType))
			{
				return UNEXPECTED("Несоответствие типа ассета с GUID '{}'. Ожидался: {}, в записи: {}",
					entry.GetGuid().ToString(), ToString(expectedType), entry.GetAssetType());
			}

			std::size_t offset = 0;
			Serializer serializer;
			T data{};
			auto res = serializer.Deserialize(bytes, offset, data);
			if (!res)
				return UNEXPECTED("Ошибка десериализации ресурса с GUID '{}': {}", entry.GetGuid().ToString(), res.error());

			return data;
		}

	private:
		[[nodiscard]] const PackageEntry* GetEntryPtr(const Guid& guid) const;

		void Initialize(const FileSystem& fileSystem);
		void LogDataEntriesSummary(const DatFileHeader& header) const;

		DatFileHeader m_Header{ c_DataDatFormat };
		std::unordered_map<Guid, PackageEntry> m_Entries;
	};
}
