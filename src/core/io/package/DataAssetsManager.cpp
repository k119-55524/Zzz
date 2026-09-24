
#include "DataAssetsManager.h"

using namespace zzz::core;

Z_SET_LOG_CATEGORY(::zzz::core::Assets);

namespace
{
	[[nodiscard]] constexpr bool IsDataArchiveResourceType(eResourceType resType) noexcept
	{
		switch (resType)
		{
		case eResourceType::Prefab:
		case eResourceType::Mesh:
		case eResourceType::Material:
		case eResourceType::Shader:
		case eResourceType::Animation:
		case eResourceType::Texture2D:
		case eResourceType::AudioClip:
		case eResourceType::Video:
		case eResourceType::Font:
		case eResourceType::BinaryData:
			return true;
		default:
			return false;
		}
	}
}

namespace zzz::core
{
	DataAssetsManager::DataAssetsManager(std::shared_ptr<FileSystem> fileSystem)
		: PackageArchive(std::move(fileSystem))
	{
		InitializeArchive(ArchiveInitParams<eResourceType>{
			.relativePath = c_DataPackageRelativePath,
			.expectedFormat = c_DataDatFormat,
			.isTypeAllowed = &IsDataArchiveResourceType
		});
	}
}
