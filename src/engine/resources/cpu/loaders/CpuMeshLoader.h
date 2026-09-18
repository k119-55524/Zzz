#pragma once

#include <memory>
#include <string>
#include <expected>
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
	};
}
