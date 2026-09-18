#include "ShaderLoader.h"
#include <logger.h>
#include "Shader.h"
#include "core/io/package/DataAssetsManager.h"
#include "core/io/package/ShaderData.h"
#include "core/utils/MemoryUtils.h"

Z_SET_LOG_CATEGORY(::zzz::core::LogEngine);

using namespace zzz::core;
using namespace zzz::logger;

namespace zzz::engine
{
	std::expected<std::shared_ptr<IResource>, std::string> ShaderLoader::Load(
		const PackageEntry& entry,
		PackageManager& /*packageManager*/,
		DataAssetsManager& dataAssetsManager,
		FileSystem& /*fileSystem*/,
		GAPI& /*gapi*/)
	{
		auto shaderDataRes = dataAssetsManager.DeserializeAsset<ShaderData>(entry);
		if (!shaderDataRes)
		{
			return std::unexpected(shaderDataRes.error());
		}

		std::string shaderName = entry.GetName().empty() ? "DefaultShader" : std::string(entry.GetName());
		if (!shaderDataRes->GetName().empty())
		{
			shaderName = shaderDataRes->GetName();
		}

		DOut("[ShaderLoader] Загружен шейдер '{}' (GUID: {})", shaderName, entry.GetGuid().ToString());

		return safe_make_shared<Shader>(entry.GetGuid(), std::move(shaderName));
	}
}
