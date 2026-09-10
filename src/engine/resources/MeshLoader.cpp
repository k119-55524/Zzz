#include "MeshLoader.h"
#include "Mesh.h"
#include "core/io/package/DataAssetsManager.h"
#include "core/io/package/MeshData.h"
#include "core/utils/MemoryUtils.h"

namespace zzz::engine
{
	std::expected<std::shared_ptr<::zzz::core::IResource>, std::string> MeshLoader::Load(
		const ::zzz::core::PackageEntry& entry,
		PackageManager& /*packageManager*/,
		::zzz::core::DataAssetsManager& dataAssetsManager,
		::zzz::core::FileSystem& /*fileSystem*/,
		GAPI& /*gapi*/)
	{
		auto meshDataRes = dataAssetsManager.DeserializeAsset<::zzz::core::MeshData>(entry);
		if (!meshDataRes)
		{
			return std::unexpected(meshDataRes.error());
		}

		auto mesh = ::zzz::core::safe_make_shared<Mesh>(entry.GetGuid(), entry.GetName(), std::move(*meshDataRes));
		return mesh;
	}
}
