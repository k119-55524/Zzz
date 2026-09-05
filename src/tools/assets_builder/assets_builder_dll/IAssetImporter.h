#pragma once

#include <filesystem>
#include <string>
#include <vector>
#include <cstddef>
#include "core/utils/Guid.h"
#include "core/enums/eResourceType.h"
#include "core/enums/eTargetPlatform.h"

namespace zzz::builder
{
	struct ImportContext
	{
		std::filesystem::path sourceFilePath;
		core::Guid assetGuid;
		std::string assetName;
		core::eTargetPlatform targetPlatform{ core::eTargetPlatform::Windows };
	};

	struct ImportResult
	{
		bool success{ false };
		std::string errorMessage;
		core::eResourceType resourceType{ core::eResourceType::Unknown };
		std::vector<std::byte> binaryPayload;
	};

	class IAssetImporter
	{
	public:
		virtual ~IAssetImporter() = default;
		[[nodiscard]] virtual ImportResult Import(const ImportContext& ctx) = 0;
	};
}
