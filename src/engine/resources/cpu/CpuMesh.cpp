#include <format>
#include "core/io/package/DataAssetsManager.h"
#include "core/utils/MemoryUtils.h"
#include "CpuMesh.h"

using namespace zzz::core;

namespace zzz::engine
{
	CpuMesh::CpuMesh(const Guid& guid, std::string name, MeshData meshData)
		: ResourceBase(guid, eEngineResourceType::Mesh, std::move(name))
		, m_MeshData(std::move(meshData))
	{
	}

	std::expected<std::shared_ptr<CpuMesh>, std::string> CpuMesh::CreateCpuResourceFromPackageBytes(
		const PackageEntry& entry,
		std::span<const std::byte> bytes)
	{
		auto meshDataRes = DataAssetsManager::DeserializeAssetFromMemory<MeshData>(entry, bytes);
		if (!meshDataRes)
		{
			return std::unexpected(std::format(
				"[CpuMesh] Ошибка десериализации меша (GUID: {}, смещение: {}): {}",
				entry.GetGuid().ToString(), entry.GetOffset(), meshDataRes.error()));
		}

		return safe_make_shared<CpuMesh>(entry.GetGuid(), entry.GetGuid().ToString(), std::move(*meshDataRes));
	}
}
