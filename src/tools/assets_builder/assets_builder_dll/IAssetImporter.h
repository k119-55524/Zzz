#pragma once

#include <filesystem>
#include <expected>
#include <span>
#include <string>
#include <vector>
#include <cstddef>
#include "core/utils/Guid.h"
#include "core/enums/eEngineResourceType.h"
#include "core/enums/eTargetPlatform.h"

namespace zzz::builder
{
	struct ImportContext
	{
		std::filesystem::path sourceFilePath;
		std::filesystem::path metaFilePath;
		core::Guid assetGuid;
		std::string assetName;
		core::eTargetPlatform targetPlatform{ core::eTargetPlatform::Windows };
		std::span<const std::byte> sourceData;
	};

	struct ImportedAssetData
	{
		std::vector<std::byte> binaryPayload;
	};
	using ImportResult = std::expected<ImportedAssetData, std::string>;

	class IAssetImporter
	{
	public:
		virtual ~IAssetImporter() = default;
		[[nodiscard]] virtual core::eEngineResourceType GetResourceType() const noexcept = 0;
		[[nodiscard]] virtual ImportResult Import(const ImportContext& ctx) = 0;
	};
}
