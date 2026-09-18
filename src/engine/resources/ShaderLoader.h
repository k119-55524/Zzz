#pragma once

#include "engine/resources/IResourceLoader.h"

namespace zzz::engine
{
	/**
	 * @class ShaderLoader
	 * @brief Загрузчик ресурса Shader из архива data.dat.
	 */
	class ShaderLoader final : public IResourceLoader
	{
	public:
		ShaderLoader() = default;
		~ShaderLoader() override = default;

		[[nodiscard]] ::zzz::core::eResourceType GetSupportedType() const noexcept override
		{
			return ::zzz::core::eResourceType::Shader;
		}

		[[nodiscard]] std::expected<std::shared_ptr<::zzz::core::IResource>, std::string> Load(
			const ::zzz::core::PackageEntry& entry,
			PackageManager& packageManager,
			::zzz::core::DataAssetsManager& dataAssetsManager,
			::zzz::core::FileSystem& fileSystem,
			GAPI& gapi) override;
	};
}
