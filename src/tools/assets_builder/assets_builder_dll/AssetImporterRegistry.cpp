#include "AssetImporterRegistry.h"
#include "ObjImporter.h"
#include "MaterialImporter.h"
#include "AssetExtensions.h"
#include <algorithm>
#include <cctype>

namespace zzz::builder
{
	namespace
	{
		std::string NormalizeExtension(std::string_view extension)
		{
			std::string result(extension);
			std::ranges::transform(result, result.begin(), [](unsigned char ch) {
				return static_cast<char>(std::tolower(ch));
			});
			return result;
		}
	}

	AssetImporterRegistry& AssetImporterRegistry::Instance()
	{
		static AssetImporterRegistry instance;
		return instance;
	}

	AssetImporterRegistry::AssetImporterRegistry()
	{
		RegisterImporter(c_ExtMeshObj, std::make_shared<ObjImporter>());
		RegisterImporter(c_ExtMaterial, std::make_shared<MaterialImporter>());

		// Scene/View - структурные ресурсы package.dat, не блоб-импортёры data.dat (обрабатываются
		// PackagePacker напрямую), но должны быть "известны" реестру наравне с остальными типами,
		// чтобы сканирование и проверка незарегистрированных файлов были едиными.
		RegisterKnownType(c_ExtScene, core::eResourceType::Scene);
		RegisterKnownType(c_ExtView, core::eResourceType::View);
	}

	void AssetImporterRegistry::RegisterImporter(std::string_view extension, std::shared_ptr<IAssetImporter> importer)
	{
		m_Importers[NormalizeExtension(extension)] = std::move(importer);
	}

	std::shared_ptr<IAssetImporter> AssetImporterRegistry::GetImporter(std::string_view extension) const
	{
		auto it = m_Importers.find(NormalizeExtension(extension));
		if (it != m_Importers.end())
			return it->second;
		return nullptr;
	}

	void AssetImporterRegistry::RegisterKnownType(std::string_view extension, core::eResourceType type)
	{
		m_KnownKinds[NormalizeExtension(extension)] = type;
	}

	std::optional<core::eResourceType> AssetImporterRegistry::GetKnownType(std::string_view extension) const
	{
		const std::string norm = NormalizeExtension(extension);

		auto importerIt = m_Importers.find(norm);
		if (importerIt != m_Importers.end())
			return importerIt->second->GetResourceType();

		auto knownIt = m_KnownKinds.find(norm);
		if (knownIt != m_KnownKinds.end())
			return knownIt->second;

		return std::nullopt;
	}
}
