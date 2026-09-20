#pragma once

#include <span>
#include <memory>
#include <string>
#include <expected>
#include "math/utils/Types.h"
#include "core/io/package/PackageEntry.h"
#include "engine/resources/cpu/CpuMesh.h"

namespace zzz::core
{
	class DataAssetsManager;
}

namespace zzz::engine
{
	/**
	 * @class CpuMeshLoader
	 * @brief Статический загрузчик ресурса CpuMesh из хранилища ассетов.
	 */
	class CpuMeshLoader final
	{
	public:
		CpuMeshLoader() = delete;

		[[nodiscard]] static std::expected<std::shared_ptr<CpuMesh>, std::string> Load(
			const ::zzz::core::PackageEntry& entry,
			::zzz::core::DataAssetsManager& dataAssetsManager);

		[[nodiscard]] static std::expected<std::shared_ptr<CpuMesh>, std::string> LoadFromMemory(
			const ::zzz::core::PackageEntry& entry,
			std::span<const std::byte> bytes);

		[[nodiscard]] static std::expected<std::shared_ptr<CpuMesh>, std::string> LoadFromMemory(
			const ::zzz::core::PackageEntry& entry,
			std::span<const zU8> bytes)
		{
			return LoadFromMemory(entry, std::as_bytes(bytes));
		}
	};
}
