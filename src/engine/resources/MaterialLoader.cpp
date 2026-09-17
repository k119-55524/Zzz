#include "MaterialLoader.h"
#include "Material.h"
#include "core/io/package/DataAssetsManager.h"
#include "core/io/package/MaterialData.h"
#include "core/utils/MemoryUtils.h"

using namespace zzz::core;

namespace zzz::engine
{
	std::expected<std::shared_ptr<IResource>, std::string> MaterialLoader::Load(
		const PackageEntry& entry,
		PackageManager& /*packageManager*/,
		DataAssetsManager& dataAssetsManager,
		FileSystem& /*fileSystem*/,
		GAPI& /*gapi*/)
	{
		auto matDataRes = dataAssetsManager.DeserializeAsset<MaterialData>(entry);
		if (!matDataRes)
		{
			return std::unexpected(matDataRes.error());
		}

		std::string matName = entry.GetName().empty() ? "DefaultMaterial" : std::string(entry.GetName());
		if (!matDataRes->GetName().empty())
		{
			matName = matDataRes->GetName();
		}

		return safe_make_shared<Material>(entry.GetGuid(), std::move(matName));
	}
}
