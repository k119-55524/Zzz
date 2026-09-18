#pragma once

#include <memory>
#include <string>
#include <expected>
#include "core/io/package/PackageEntry.h"
#include "engine/resources/cpu/CpuShader.h"

namespace zzz::core
{
	class DataAssetsManager;
}

namespace zzz::engine
{
	/**
	 * @class CpuShaderLoader
	 * @brief Статический загрузчик ресурса CpuShader из хранилища ассетов.
	 */
	class CpuShaderLoader final
	{
	public:
		CpuShaderLoader() = delete;

		[[nodiscard]] static std::expected<std::shared_ptr<CpuShader>, std::string> Load(
			const ::zzz::core::PackageEntry& entry,
			::zzz::core::DataAssetsManager& dataAssetsManager);
	};
}
