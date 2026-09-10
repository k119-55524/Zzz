#pragma once

#include "engine/resources/IResourceLoader.h"

namespace zzz::engine
{
	/**
	 * @class MeshLoader
	 * @brief Загрузчик ресурса Mesh из архива data.dat.
	 */
	class MeshLoader final : public IResourceLoader
	{
	public:
		MeshLoader() = default;
		~MeshLoader() override = default;

		[[nodiscard]] ::zzz::core::eResourceType GetSupportedType() const noexcept override
		{
			return ::zzz::core::eResourceType::Mesh;
		}

		[[nodiscard]] std::expected<std::shared_ptr<::zzz::core::IResource>, std::string> Load(
			const ::zzz::core::PackageEntry& entry,
			PackageManager& packageManager,
			::zzz::core::DataAssetsManager& dataAssetsManager,
			::zzz::core::FileSystem& fileSystem,
			GAPI& gapi) override;
	};
}
