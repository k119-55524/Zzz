
#include "DataAssetsManager.h"

using namespace zzz::core;

Z_SET_LOG_CATEGORY(::zzz::core::Assets);

namespace zzz::core
{
	DataAssetsManager::DataAssetsManager(
		const std::filesystem::path& physicalPath,
		NativeAppData* nativeData)
		: ArchiveReaderBase(physicalPath, nativeData)
	{
	}
}
