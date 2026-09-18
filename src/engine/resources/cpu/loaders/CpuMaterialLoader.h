#pragma once

#include <memory>
#include <string>
#include <expected>
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
	};
}
