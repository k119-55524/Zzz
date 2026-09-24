#pragma once

#include <expected>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

#include "core/enums/eEngineResourceType.h"
#include "core/enums/eTargetPlatform.h"
#include "core/utils/Guid.h"

namespace zzz::builder
{
	struct ImportedDataAsset
	{
		std::string name;
		core::Guid guid;
		core::eEngineResourceType resourceType{ core::eEngineResourceType::Unknown };
		std::vector<std::byte> payload;
	};

	/**
	 * @brief Единая build-time граница для зарегистрированных ресурсов data.dat.
	 *
	 * Проверяет регистрацию расширения, обязательную пару asset + asset.meta,
	 * полностью читает исходник и только после этого вызывает форматный импортёр.
	 * Импортёры не открывают файлы и не записывают архивы.
	 */
	class AssetImportPipeline final
	{
	public:
		[[nodiscard]] static std::expected<std::optional<ImportedDataAsset>, std::string> TryImport(
			const std::filesystem::path& sourceFilePath,
			core::eTargetPlatform targetPlatform);
	};
}
