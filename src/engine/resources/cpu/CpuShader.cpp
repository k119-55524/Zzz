#include <format>
#include "core/io/package/DataAssetsManager.h"
#include "core/io/package/ShaderData.h"
#include "core/utils/MemoryUtils.h"
#include "CpuShader.h"

using namespace zzz::core;

namespace zzz::engine
{
	CpuShader::CpuShader(const Guid& guid, std::string name)
		: ResourceBase(guid, eResourceType::Shader, std::move(name))
	{
	}

	std::expected<std::shared_ptr<CpuShader>, std::string> CpuShader::CreateCpuResourceFromPackageBytes(
		const PackageEntry& entry,
		std::span<const std::byte> bytes)
	{
		auto shaderDataRes = DataAssetsManager::DeserializeAssetFromMemory<ShaderData>(entry, bytes);
		if (!shaderDataRes)
		{
			return std::unexpected(std::format(
				"[CpuShader] Ошибка десериализации шейдера (GUID: {}, смещение: {}): {}",
				entry.GetGuid().ToString(), entry.GetOffset(), shaderDataRes.error()));
		}

		std::string shaderName = shaderDataRes->GetName().empty() ? entry.GetGuid().ToString() : shaderDataRes->GetName();

		return safe_make_shared<CpuShader>(entry.GetGuid(), std::move(shaderName));
	}
}
