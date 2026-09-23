#pragma once

#include <string>
#include <vector>
#include <expected>
#include <filesystem>

#include "core/utils/Export.h"
#include "core/io/FileSystem.h"
#include "core/io/DatFileHeader.h"
#include "core/enums/eFileLocation.h"
#include "core/io/package/PackageEntry.h"

namespace zzz::core
{
	/**
	 * @struct ArchiveTable
	 * @brief Проверенное оглавление бинарного архива (.dat): заголовок и записи таблицы.
	 */
	struct ArchiveTable
	{
		DatFileHeader header{};
		std::vector<PackageEntry> entries;
	};

	/// @brief Проверка допустимости типа записи для конкретного архива (package.dat / data.dat).
	using ArchiveEntryTypeValidator = bool (*)(zU32 assetType) noexcept;

	/**
	 * @brief Читает и проверяет оглавление архива .dat.
	 * @details Единая точка разбора заголовка и таблицы записей для PackageManager и DataAssetsManager.
	 *          Проверяет по порядку:
	 *          1. Размер файла не меньше заголовка.
	 *          2. Сигнатуру и мажорную версию заголовка.
	 *          3. Размер таблицы (entryCount * PackageEntry::BinarySize()) без переполнения и то,
	 *             что заголовок и таблица помещаются в файл.
	 *          4. Для каждой записи: допустимость типа (isValidType) и то, что диапазон данных
	 *             [offset, offset + size) лежит в области полезной нагрузки после таблицы и внутри файла.
	 *          5. Только в Debug/Development: GUID не нулевой и уникален внутри архива.
	 * @param fileSystem Файловая система.
	 * @param location Логическая область размещения архива.
	 * @param relativePath Путь к архиву относительно области.
	 * @param expectedMagic Ожидаемая сигнатура архива.
	 * @param expectedMajorVersion Ожидаемая мажорная версия формата.
	 * @param isValidType Проверка допустимости типа записи для данного архива.
	 * @return Проверенное оглавление или текст ошибки.
	 */
	[[nodiscard]] Z_CORE_API std::expected<ArchiveTable, std::string> ReadArchiveTable(
		const FileSystem& fileSystem,
		eFileLocation location,
		const std::filesystem::path& relativePath,
		const DatFileHeader::Magic& expectedMagic,
		zU8 expectedMajorVersion,
		ArchiveEntryTypeValidator isValidType);
}
