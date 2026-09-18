#include "CpuShaderLoader.h"
#include "core/io/package/DataAssetsManager.h"
#include "core/io/package/ShaderData.h"
#include "core/utils/MemoryUtils.h"
#include <format>
#include <logger.h>

Z_SET_LOG_CATEGORY(::zzz::core::LogEngine);

using namespace zzz::core;
using namespace zzz::logger;

namespace zzz::engine
{
	std::expected<std::shared_ptr<CpuShader>, std::string> CpuShaderLoader::Load(
		const PackageEntry& entry,
		DataAssetsManager& dataAssetsManager)
	{
		auto shaderDataRes = dataAssetsManager.DeserializeAsset<ShaderData>(entry);
		if (!shaderDataRes)
		{
			return std::unexpected(std::format(
				"[CpuShaderLoader] Ошибка десериализации шейдера '{}' (GUID: {}, смещение: {}): {}",
				entry.GetName(), entry.GetGuid().ToString(), entry.GetOffset(), shaderDataRes.error()));
		}

		std::string shaderName = entry.GetName().empty() ? "DefaultShader" : std::string(entry.GetName());
		if (!shaderDataRes->GetName().empty())
		{
			shaderName = shaderDataRes->GetName();
		}

		DOut("[CpuShaderLoader] Загружен шейдер '{}' (GUID: {})", shaderName, entry.GetGuid().ToString());

		return safe_make_shared<CpuShader>(entry.GetGuid(), std::move(shaderName));
	}
}
