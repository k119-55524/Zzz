#pragma once

#include "engine/resources/IResourceLoader.h"

namespace zzz::engine
{
	/**
	 * @class MaterialLoader
	 * @brief Загрузчик ресурса Material из архива data.dat.
	 */
	class MaterialLoader final : public IResourceLoader
	{
	public:
		MaterialLoader() = default;
		~MaterialLoader() override = default;

		[[nodiscard]] ::zzz::core::eResourceType GetSupportedType() const noexcept override
		{
			return ::zzz::core::eResourceType::Material;
		}

		[[nodiscard]] std::expected<std::shared_ptr<::zzz::core::IResource>, std::string> Load(
			const ::zzz::core::PackageEntry& entry,
			PackageManager& packageManager,
			::zzz::core::DataAssetsManager& dataAssetsManager,
			::zzz::core::FileSystem& fileSystem,
			GAPI& gapi) override;
	};
}
