#pragma once

#include <string>
#include <string_view>
#include <memory>
#include <unordered_map>
#include "IAssetImporter.h"

namespace zzz::builder
{
	class AssetImporterRegistry
	{
	public:
		static AssetImporterRegistry& Instance();

		void RegisterImporter(std::string_view extension, std::shared_ptr<IAssetImporter> importer);
		[[nodiscard]] std::shared_ptr<IAssetImporter> GetImporter(std::string_view extension) const;

	private:
		AssetImporterRegistry();
		std::unordered_map<std::string, std::shared_ptr<IAssetImporter>> m_Importers;
	};
}
