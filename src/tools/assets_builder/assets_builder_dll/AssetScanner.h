#pragma once

#include <filesystem>
#include <functional>
#include <optional>
#include <string>

#include "core/enums/eEngineResourceType.h"

namespace zzz::builder
{
	// Один файл, найденный при обходе Assets/: путь, нормализованное расширение (как вернул
	// std::filesystem::path::extension()) и распознанный тип (см. AssetImporterRegistry::GetKnownType),
	// если расширение известно реестру. .meta-файлы и не-regular записи (директории и т.п.) в обход
	// не попадают - это общая для скана PackagePacker и ProjectIdentityValidator фильтрация "на входе".
	struct ScannedAssetFile
	{
		std::filesystem::path path;
		std::string extension;
		std::optional<zzz::core::eEngineResourceType> knownType;
	};

	// Рекурсивно обходит assetsDir (не ошибка, если её нет - тогда просто ничего не вызывается) и
	// вызывает visitor для каждого найденного файла. Общая точка правды для обоих сканирующих проходов
	// сборщика (PackagePacker::PackProject и ProjectIdentityValidator::Validate) - раньше каждый делал
	// свой независимый recursive_directory_iterator по Assets/, что уже приводило к рассинхронизации
	// политики скана между ними (Пункт 21).
	//
	// visitor возвращает true, чтобы продолжить обход, или false, если обработка файла обнаружила
	// ошибку (которую вызывающий уже залогировал/записал сам) - в этом случае обход останавливается
	// немедленно и ScanAssetsDirectory тоже возвращает false.
	bool ScanAssetsDirectory(const std::filesystem::path& assetsDir,
		const std::function<bool(const ScannedAssetFile&)>& visitor);
}
