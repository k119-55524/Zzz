#pragma once

#include <map>
#include <string>
#include <memory>
#include <expected>
#include <unordered_map>

#include "core/utils/Guid.h"
#include "core/utils/Export.h"
#include "core/io/FileSystem.h"
#include "core/io/DatFileHeader.h"
#include "core/enums/eResourceType.h"
#include "core/serialize/Serializer.h"
#include "core/io/package/PackageEntry.h"

namespace zzz::engine
{
	class CpuResourceManager;
}

namespace zzz::core
{
	class MeshData;
	class PrefabData;
	class MaterialData;
	class ShaderData;

	/**
	 * @struct AssetLocation
	 * @brief Физические координаты размещения ассета на диске для чтения.
	 */
	struct AssetLocation
	{
		eFileLocation location{ eFileLocation::App };
		std::filesystem::path relativePath;
		std::size_t offset = 0;
		std::size_t size = 0;
		const PackageEntry* entry = nullptr;
	};

	/**
	 * @class DataAssetsManager
	 * @brief Менеджер для чтения игровых ресурсов из архива по пути c_DataPackageRelativePath.
	 */
	class Z_CORE_API DataAssetsManager final
	{
		friend class ::zzz::engine::CpuResourceManager;

	public:
		DataAssetsManager() = delete;
		explicit DataAssetsManager(std::shared_ptr<FileSystem> fileSystem);
		~DataAssetsManager() = default;

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

		void Initialize();
		void LogDataEntriesSummary() const;

		std::shared_ptr<FileSystem> m_FileSystem;
		DatFileHeader m_Header{};
		std::unordered_map<Guid, PackageEntry> m_Entries;
	};
}
