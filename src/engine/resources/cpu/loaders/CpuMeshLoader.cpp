#include "CpuMeshLoader.h"
#include "core/io/package/DataAssetsManager.h"
#include "core/io/package/MeshData.h"
#include "core/utils/MemoryUtils.h"
#include <format>

using namespace zzz::core;

namespace zzz::engine
{
	std::expected<std::shared_ptr<CpuMesh>, std::string> CpuMeshLoader::Load(
		const PackageEntry& entry,
		DataAssetsManager& dataAssetsManager)
	{
		auto meshDataRes = dataAssetsManager.DeserializeAsset<MeshData>(entry);
		if (!meshDataRes)
		{
			return std::unexpected(std::format(
				"[CpuMeshLoader] Ошибка десериализации меша '{}' (GUID: {}, смещение: {}): {}",
				entry.GetName(), entry.GetGuid().ToString(), entry.GetOffset(), meshDataRes.error()));
		}

		auto mesh = safe_make_shared<CpuMesh>(entry.GetGuid(), entry.GetName(), std::move(*meshDataRes));
		return mesh;
	}
}
