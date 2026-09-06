#include "AssetImporterRegistry.h"
#include "ObjImporter.h"
#include "AssetExtensions.h"

namespace zzz::builder
{
	AssetImporterRegistry& AssetImporterRegistry::Instance()
	{
		static AssetImporterRegistry instance;
		return instance;
	}

	AssetImporterRegistry::AssetImporterRegistry()
	{
		RegisterImporter(c_ExtMeshObj, std::make_shared<ObjImporter>());
	}

	void AssetImporterRegistry::RegisterImporter(std::string_view extension, std::shared_ptr<IAssetImporter> importer)
	{
		m_Importers[std::string(extension)] = std::move(importer);
	}

	std::shared_ptr<IAssetImporter> AssetImporterRegistry::GetImporter(std::string_view extension) const
	{
		auto it = m_Importers.find(std::string(extension));
		if (it != m_Importers.end())
			return it->second;
		return nullptr;
	}
}
