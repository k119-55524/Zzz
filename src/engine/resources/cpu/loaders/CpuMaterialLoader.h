#pragma once

#include <span>
#include <memory>
#include <string>
#include <expected>
#include "math/utils/Types.h"
#include "core/io/package/PackageEntry.h"
#include "engine/resources/cpu/CpuMaterial.h"

namespace zzz::core
{
	class DataAssetsManager;
}

namespace zzz::engine
{
	/**
	 * @class CpuMaterialLoader
	 * @brief Статический загрузчик ресурса CpuMaterial из хранилища ассетов.
	 */
	class CpuMaterialLoader final
	{
	public:
		CpuMaterialLoader() = delete;

		[[nodiscard]] static std::expected<std::shared_ptr<CpuMaterial>, std::string> Load(
			const ::zzz::core::PackageEntry& entry,
			::zzz::core::DataAssetsManager& dataAssetsManager);

		[[nodiscard]] static std::expected<std::shared_ptr<CpuMaterial>, std::string> LoadFromMemory(
			const ::zzz::core::PackageEntry& entry,
			std::span<const std::byte> bytes);

		[[nodiscard]] static std::expected<std::shared_ptr<CpuMaterial>, std::string> LoadFromMemory(
			const ::zzz::core::PackageEntry& entry,
			std::span<const zU8> bytes)
		{
			return LoadFromMemory(entry, std::as_bytes(bytes));
		}
	};
}
