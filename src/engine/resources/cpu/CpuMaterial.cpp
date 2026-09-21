#include <format>
#include "core/io/package/DataAssetsManager.h"
#include "core/io/package/MaterialData.h"
#include "core/utils/MemoryUtils.h"
#include "CpuMaterial.h"

using namespace zzz::core;

namespace zzz::engine
{
	CpuMaterial::CpuMaterial(const Guid& guid, std::string name, Guid shaderGuid)
		: ResourceBase(guid, eResourceType::Material, std::move(name))
		, m_ShaderGuid(shaderGuid)
	{
	}

	std::expected<std::shared_ptr<CpuMaterial>, std::string> CpuMaterial::CreateCpuResourceFromPackageBytes(
		const PackageEntry& entry,
		std::span<const std::byte> bytes)
	{
		auto matDataRes = DataAssetsManager::DeserializeAssetFromMemory<MaterialData>(entry, bytes);
		if (!matDataRes)
		{
			return std::unexpected(std::format(
				"[CpuMaterial] Ошибка десериализации материала '{}' (GUID: {}, смещение: {}): {}",
				entry.GetName(), entry.GetGuid().ToString(), entry.GetOffset(), matDataRes.error()));
		}

		std::string matName = entry.GetName().empty() ? "DefaultMaterial" : std::string(entry.GetName());
		if (!matDataRes->GetName().empty())
		{
			matName = matDataRes->GetName();
		}

		return safe_make_shared<CpuMaterial>(entry.GetGuid(), std::move(matName), matDataRes->GetShaderGuid());
	}
}
