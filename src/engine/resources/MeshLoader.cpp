#include "MeshLoader.h"
#include "Mesh.h"
#include "core/io/package/DataAssetsManager.h"
#include "core/io/package/MeshData.h"
#include "core/utils/MemoryUtils.h"

using namespace zzz::core;

namespace zzz::engine
{
	std::expected<std::shared_ptr<IResource>, std::string> MeshLoader::Load(
		const PackageEntry& entry,
		PackageManager& /*packageManager*/,
		DataAssetsManager& dataAssetsManager,
		FileSystem& /*fileSystem*/,
		GAPI& /*gapi*/)
	{
		auto meshDataRes = dataAssetsManager.DeserializeAsset<MeshData>(entry);
		if (!meshDataRes)
		{
			return std::unexpected(meshDataRes.error());
		}

		auto mesh = safe_make_shared<Mesh>(entry.GetGuid(), entry.GetName(), std::move(*meshDataRes));
		return mesh;
	}
}
