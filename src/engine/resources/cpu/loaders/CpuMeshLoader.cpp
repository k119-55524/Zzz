#include "CpuMeshLoader.h"
#include "core/io/package/DataAssetsManager.h"
#include "core/io/package/MeshData.h"
#include "core/utils/MemoryUtils.h"
#include <format>

using namespace zzz::core;

namespace zzz::engine
{
	std::expected<std::shared_ptr<CpuMesh>, std::string> CpuMeshLoader::LoadFromMemory(
		const PackageEntry& entry,
		std::span<const std::byte> bytes)
	{
		auto meshDataRes = DataAssetsManager::DeserializeAssetFromMemory<MeshData>(entry, bytes);
		if (!meshDataRes)
		{
			return std::unexpected(std::format(
				"[CpuMeshLoader] Ошибка десериализации меша '{}' (GUID: {}, смещение: {}): {}",
				entry.GetName(), entry.GetGuid().ToString(), entry.GetOffset(), meshDataRes.error()));
		}

		auto mesh = safe_make_shared<CpuMesh>(entry.GetGuid(), entry.GetName(), std::move(*meshDataRes));
		return mesh;
	}

	std::expected<std::shared_ptr<CpuMesh>, std::string> CpuMeshLoader::Load(
		const PackageEntry& entry,
		DataAssetsManager& dataAssetsManager)
	{
		auto rawRes = dataAssetsManager.ReadRawBytes(entry);
		if (!rawRes)
		{
			return std::unexpected(std::format(
				"[CpuMeshLoader] Ошибка чтения сырых данных меша '{}' (GUID: {}, смещение: {}): {}",
				entry.GetName(), entry.GetGuid().ToString(), entry.GetOffset(), rawRes.error()));
		}

		return LoadFromMemory(entry, *rawRes);
	}
}
