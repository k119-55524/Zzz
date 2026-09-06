#pragma once

#include <memory>
#include <string>
#include <expected>
#include "core/enums/eResourceType.h"
#include "core/resources/IResource.h"
#include "core/io/package/PackageEntry.h"
#include "core/io/FileSystem.h"
#include "engine/gapi/GAPI.h"

namespace zzz::engine
{
	class PackageManager;

	/**
	 * @class IResourceLoader
	 * @brief Полиморфный интерфейс загрузчика конкретного типа ассетов (из пакета или напрямую с диска).
	 */
	class IResourceLoader
	{
	public:
		virtual ~IResourceLoader() = default;

		[[nodiscard]] virtual ::zzz::core::eResourceType GetSupportedType() const noexcept = 0;

		/// @brief Загрузка ресурса из записи пакета package.dat
		[[nodiscard]] virtual std::expected<std::shared_ptr<::zzz::core::IResource>, std::string> Load(
			const ::zzz::core::PackageEntry& entry,
			PackageManager& packageManager,
			::zzz::core::FileSystem& fileSystem,
			GAPI& gapi) = 0;
	};
}
