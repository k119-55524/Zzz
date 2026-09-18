#include "CpuMaterialLoader.h"
#include "core/io/package/DataAssetsManager.h"
#include "core/io/package/MaterialData.h"
#include "core/utils/MemoryUtils.h"
#include <format>
#include <logger.h>

Z_SET_LOG_CATEGORY(::zzz::core::LogEngine);

using namespace zzz::core;
using namespace zzz::logger;

namespace zzz::engine
{
	std::expected<std::shared_ptr<CpuMaterial>, std::string> CpuMaterialLoader::Load(
		const PackageEntry& entry,
		DataAssetsManager& dataAssetsManager)
	{
		auto matDataRes = dataAssetsManager.DeserializeAsset<MaterialData>(entry);
		if (!matDataRes)
		{
			return std::unexpected(std::format(
				"[CpuMaterialLoader] Ошибка десериализации материала '{}' (GUID: {}, смещение: {}): {}",
				entry.GetName(), entry.GetGuid().ToString(), entry.GetOffset(), matDataRes.error()));
		}

		std::string matName = entry.GetName().empty() ? "DefaultMaterial" : std::string(entry.GetName());
		if (!matDataRes->GetName().empty())
		{
			matName = matDataRes->GetName();
		}

		Guid shaderGuid = matDataRes->GetShaderGuid();
		DOut("[CpuMaterialLoader] Загружен материал '{}' (GUID: {}, шейдер: {})",
			matName, entry.GetGuid().ToString(), shaderGuid.ToString());

		return safe_make_shared<CpuMaterial>(entry.GetGuid(), std::move(matName), shaderGuid);
	}
}
