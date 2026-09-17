#pragma once

#include <string>
#include <string_view>
#include <memory>
#include <optional>
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

		// Регистрация "известного" типа ассета, который не проходит через IAssetImporter::Import
		// (структурные package.dat-ресурсы вроде Scene/View, обрабатываемые PackagePacker напрямую).
		// GetKnownType - единая точка правды "известно ли расширение вообще", используемая и
		// валидатором, и упаковщиком, независимо от того, есть ли у типа блоб-импортёр.
		void RegisterKnownType(std::string_view extension, core::eResourceType type);
		[[nodiscard]] std::optional<core::eResourceType> GetKnownType(std::string_view extension) const;

	private:
		AssetImporterRegistry();
		std::unordered_map<std::string, std::shared_ptr<IAssetImporter>> m_Importers;
		std::unordered_map<std::string, core::eResourceType> m_KnownKinds;
	};
}
