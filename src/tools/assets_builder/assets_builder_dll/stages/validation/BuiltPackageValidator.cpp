#include "BuiltPackageValidator.h"
#include <fstream>
#include <format>
#include <unordered_set>
#include <algorithm>
#include "core/constants/PackagesConstants.h"
#include "core/io/DatFileHeader.h"
#include "core/serialize/Serializer.h"
#include "core/enums/ePackageDatType.h"
#include "core/io/package/ProjectManifestData.h"

namespace zzz::builder
{
	namespace
	{
		[[nodiscard]] bool IsValidPackageDatType(core::ePackageDatType type) noexcept
		{
			return std::to_underlying(type) >= std::to_underlying(core::ePackageDatType::First)
				&& std::to_underlying(type) <= std::to_underlying(core::ePackageDatType::Last);
		}

		struct Interval
		{
			uint64_t begin;
			uint64_t end;
			core::Guid guid;

			bool operator<(const Interval& other) const noexcept
			{
				return begin < other.begin;
			}
		};

		bool CheckNonOverlapping(std::vector<Interval>& intervals, std::string& outError, std::string_view fileContext)
		{
			std::sort(intervals.begin(), intervals.end());
			for (size_t i = 1; i < intervals.size(); ++i)
			{
				if (intervals[i].begin < intervals[i - 1].end)
				{
					outError = std::format("В файле '{}' обнаружено пересечение диапазонов ресурсов: [{:#x}, {:#x}) (GUID: {}) и [{:#x}, {:#x}) (GUID: {})",
						fileContext,
						intervals[i - 1].begin, intervals[i - 1].end, intervals[i - 1].guid.ToString(),
						intervals[i].begin, intervals[i].end, intervals[i].guid.ToString());
					return false;
				}
			}
			return true;
		}

		[[nodiscard]] bool IsInlineType(core::eDataDatType type) noexcept
		{
			switch (type)
			{
			case core::eDataDatType::Mesh:
			case core::eDataDatType::Material:
			case core::eDataDatType::Shader:
			case core::eDataDatType::Animation:
				return true;
			default:
				return false;
			}
		}

		[[nodiscard]] bool IsValidExternalPakType(core::eDataDatType type) noexcept
		{
			switch (type)
			{
			case core::eDataDatType::Texture2D:
			case core::eDataDatType::AudioClip:
			case core::eDataDatType::Video:
			case core::eDataDatType::Font:
			case core::eDataDatType::BinaryData:
				return true;
			default:
				return false;
			}
		}
	}

	PackageValidationReport BuiltPackageValidator::Validate(const std::filesystem::path& assetsDir)
	{
		PackageValidationReport report;
		core::Serializer serializer;

		std::error_code ec;
		if (!std::filesystem::exists(assetsDir, ec) || !std::filesystem::is_directory(assetsDir, ec))
		{
			report.isValid = false;
			report.errorMessage = std::format("Директория пакетов не существует: {}", assetsDir.string());
			return report;
		}

		// 1. Валидация package.dat
		const auto packageDatPath = assetsDir / core::c_GamePackageFileName;
		if (!std::filesystem::exists(packageDatPath, ec))
		{
			report.isValid = false;
			report.errorMessage = std::format("Отсутствует обязательный системный пакет: {}", packageDatPath.string());
			return report;
		}

		const auto packageDatSize = std::filesystem::file_size(packageDatPath, ec);
		if (packageDatSize < core::PackageDatHeader::c_FullHeaderSize)
		{
			report.isValid = false;
			report.errorMessage = std::format("Файл package.dat повреждён (размер {} < 31 байт)", packageDatSize);
			return report;
		}

		std::vector<std::byte> packageDatBytes(static_cast<size_t>(packageDatSize));
		{
			std::ifstream f(packageDatPath, std::ios::binary);
			f.read(reinterpret_cast<char*>(packageDatBytes.data()), packageDatBytes.size());
			if (!f)
			{
				report.isValid = false;
				report.errorMessage = std::format("Не удалось прочитать package.dat: {}", packageDatPath.string());
				return report;
			}
		}

		core::PackageDatHeader pkgHeader(core::c_PackageDatFormat);
		std::size_t pkgOffset = 0;
		auto desRes = serializer.Deserialize(packageDatBytes, pkgOffset, pkgHeader);
		if (!desRes)
		{
			report.isValid = false;
			report.errorMessage = std::format("Невалидный заголовок package.dat: {}", desRes.error());
			return report;
		}

		if (pkgHeader.GetMagic() != core::c_PackageDatFormat.Magic)
		{
			report.isValid = false;
			report.errorMessage = std::format("Некорректная сигнатура package.dat: '{}'", pkgHeader.GetMagic());
			return report;
		}

		if (pkgHeader.GetVersion() != core::c_PackageDatFormat.FormatVersion)
		{
			report.isValid = false;
			report.errorMessage = std::format("Несовместимая версия package.dat: {} (ожидалась {})",
				pkgHeader.GetVersion().ToString(), core::c_PackageDatFormat.FormatVersion.ToString());
			return report;
		}

		report.packageDatTimestamp = pkgHeader.GetTimestamp();

		const uint32_t pkgEntryCount = pkgHeader.GetEntryCount();
		const auto tableSizeOpt = core::PackageEntry::CalculateTableSize(pkgEntryCount);
		if (!tableSizeOpt)
		{
			report.isValid = false;
			report.errorMessage = std::format("Переполнение размера таблицы записей в package.dat (записей: {})", pkgEntryCount);
			return report;
		}

		const std::size_t pkgPayloadBegin = pkgOffset + *tableSizeOpt;
		if (pkgPayloadBegin > packageDatSize)
		{
			report.isValid = false;
			report.errorMessage = std::format("Таблица записей в package.dat выходит за пределы файла: {} > {}", pkgPayloadBegin, packageDatSize);
			return report;
		}

		std::vector<Interval> pkgIntervals;
		std::unordered_set<core::Guid> pkgGuids;
		uint32_t manifestCount = 0;
		uint32_t primaryViewCount = 0;
		std::optional<core::PackageEntry> manifestEntry;

		for (uint32_t i = 0; i < pkgEntryCount; ++i)
		{
			core::PackageEntry entry;
			auto r = serializer.Deserialize(packageDatBytes, pkgOffset, entry);
			if (!r)
			{
				report.isValid = false;
				report.errorMessage = std::format("Ошибка чтения записи TOC #{} в package.dat: {}", i, r.error());
				return report;
			}

			if (!pkgGuids.insert(entry.GetGuid()).second)
			{
				report.isValid = false;
				report.errorMessage = std::format("Обнаружен дубликат GUID '{}' в package.dat", entry.GetGuid().ToString());
				return report;
			}

			const auto pType = static_cast<core::ePackageDatType>(entry.GetAssetType());
			if (!IsValidPackageDatType(pType))
			{
				report.isValid = false;
				report.errorMessage = std::format("Запись #{} в package.dat содержит неизвестный тип: {}", i, entry.GetAssetType());
				return report;
			}

			if (entry.GetOffset() < pkgPayloadBegin || entry.GetOffset() > packageDatSize ||
				entry.GetSize() > packageDatSize - entry.GetOffset())
			{
				report.isValid = false;
				report.errorMessage = std::format("Запись {} в package.dat выходит за границы файла (offset: {}, size: {}, fileSize: {})",
					entry.GetGuid().ToString(), entry.GetOffset(), entry.GetSize(), packageDatSize);
				return report;
			}

			if (entry.GetSize() > 0)
			{
				pkgIntervals.push_back(Interval{ entry.GetOffset(), entry.GetOffset() + entry.GetSize(), entry.GetGuid() });
			}

			if (pType == core::ePackageDatType::ProjectManifest)
			{
				manifestCount++;
				manifestEntry = entry;
			}
			else if (pType == core::ePackageDatType::PrimaryView)
			{
				primaryViewCount++;
			}
		}

		std::string pkgIntervalErr;
		if (!CheckNonOverlapping(pkgIntervals, pkgIntervalErr, core::c_GamePackageFileName))
		{
			report.isValid = false;
			report.errorMessage = pkgIntervalErr;
			return report;
		}

		if (manifestCount != 1)
		{
			report.isValid = false;
			report.errorMessage = std::format("В package.dat ожидался ровно 1 ProjectManifest, обнаружено: {}", manifestCount);
			return report;
		}

		if (manifestEntry.has_value())
		{
			std::size_t mOffset = static_cast<std::size_t>(manifestEntry->GetOffset());
			core::ProjectManifestData manifest;
			auto manRes = serializer.Deserialize(packageDatBytes, mOffset, manifest);
			if (!manRes)
			{
				report.isValid = false;
				report.errorMessage = std::format("Ошибка десериализации ProjectManifestData из package.dat: {}", manRes.error());
				return report;
			}
			if (manifest.GetAppName().empty())
			{
				report.isValid = false;
				report.errorMessage = "ProjectManifestData в package.dat содержит пустое имя приложения (appName)";
				return report;
			}

			if (!manifest.GetViewGuids().empty() && primaryViewCount != 1)
			{
				report.isValid = false;
				report.errorMessage = std::format("В package.dat ожидался ровно 1 PrimaryView, обнаружено: {}", primaryViewCount);
				return report;
			}
		}

		if (primaryViewCount > 1)
		{
			report.isValid = false;
			report.errorMessage = std::format("В package.dat обнаружено несколько ({}) PrimaryView. Допустим максимум 1.", primaryViewCount);
			return report;
		}

		// 2. Валидация data.dat
		const auto dataDatPath = assetsDir / core::c_DataPackageFileName;
		if (!std::filesystem::exists(dataDatPath, ec))
		{
			report.isValid = false;
			report.errorMessage = std::format("Отсутствует обязательный архив данных: {}", dataDatPath.string());
			return report;
		}

		const auto dataDatSize = std::filesystem::file_size(dataDatPath, ec);
		if (dataDatSize < core::DataDatHeader::c_FullHeaderSize)
		{
			report.isValid = false;
			report.errorMessage = std::format("Файл data.dat повреждён (размер {} < 64 байт)", dataDatSize);
			return report;
		}

		std::vector<std::byte> dataDatBytes(static_cast<size_t>(dataDatSize));
		{
			std::ifstream f(dataDatPath, std::ios::binary);
			f.read(reinterpret_cast<char*>(dataDatBytes.data()), dataDatBytes.size());
		}

		core::DataDatHeader dataHeader(core::c_DataDatFormat);
		std::size_t offset = 0;
		{
			auto desRes = serializer.Deserialize(dataDatBytes, offset, dataHeader);
			if (!desRes)
			{
				report.isValid = false;
				report.errorMessage = std::format("Невалидный заголовок data.dat: {}", desRes.error());
				return report;
			}
		}

		if (dataHeader.GetMagic() != core::c_DataDatFormat.Magic)
		{
			report.isValid = false;
			report.errorMessage = std::format("Некорректная сигнатура data.dat: '{}'", dataHeader.GetMagic());
			return report;
		}

		if (dataHeader.GetVersion() != core::c_DataDatFormat.FormatVersion)
		{
			report.isValid = false;
			report.errorMessage = std::format("Несовместимая версия data.dat: {} (ожидалась {})",
				dataHeader.GetVersion().ToString(), core::c_DataDatFormat.FormatVersion.ToString());
			return report;
		}

		if (dataHeader.GetHeaderSize() != core::DataDatHeader::c_FullHeaderSize)
		{
			report.isValid = false;
			report.errorMessage = std::format("Некорректный размер заголовка data.dat: {} (ожидалось 64 байта)", dataHeader.GetHeaderSize());
			return report;
		}

		report.dataDatTimestamp = dataHeader.GetTimestamp();
		if (report.dataDatTimestamp != report.packageDatTimestamp)
		{
			report.isValid = false;
			report.errorMessage = std::format("Рассинхронизация timestamp между package.dat ({}) и data.dat ({})",
				report.packageDatTimestamp, report.dataDatTimestamp);
			return report;
		}

		const auto& extra = dataHeader.GetExtra();
		report.inlineCount = extra.inlineCount;
		report.externalCount = extra.externalCount;

		if (dataHeader.GetEntryCount() != extra.inlineCount + extra.externalCount)
		{
			report.isValid = false;
			report.errorMessage = std::format("m_EntryCount ({}) не равен inlineCount ({}) + externalCount ({}) в data.dat",
				dataHeader.GetEntryCount(), extra.inlineCount, extra.externalCount);
			return report;
		}

		// 2.1. Таблица 1: packCount элементов eDataDatType
		std::unordered_set<core::eDataDatType> manifestTypeSet;
		zU32 prevTypeVal = 0;
		for (zU32 i = 0; i < extra.packCount; ++i)
		{
			zU32 typeVal = 0;
			auto res = serializer.Deserialize(dataDatBytes, offset, typeVal);
			if (!res)
			{
				report.isValid = false;
				report.errorMessage = std::format("Ошибка чтения Таблицы 1 (манифест паков, индекс {}): {}", i, res.error());
				return report;
			}

			const auto pType = static_cast<core::eDataDatType>(typeVal);
			if (!IsValidExternalPakType(pType))
			{
				report.isValid = false;
				report.errorMessage = std::format("Таблица 1 содержит недопустимый тип внешнего пака: {}", typeVal);
				return report;
			}

			if (typeVal <= prevTypeVal && i > 0)
			{
				report.isValid = false;
				report.errorMessage = std::format("Типы в Таблице 1 не отсортированы строго по возрастанию: {} <= {}", typeVal, prevTypeVal);
				return report;
			}
			prevTypeVal = typeVal;

			manifestTypeSet.insert(pType);
			report.packManifestTypes.push_back(pType);
		}

		std::unordered_set<core::Guid> allGuids;

		// 2.2. Таблица 2: inlineCount элементов PackageEntry
		std::vector<Interval> inlineIntervals;
		report.inlineEntries.reserve(extra.inlineCount);
		for (zU32 i = 0; i < extra.inlineCount; ++i)
		{
			core::PackageEntry entry;
			auto res = serializer.Deserialize(dataDatBytes, offset, entry);
			if (!res)
			{
				report.isValid = false;
				report.errorMessage = std::format("Ошибка чтения Таблицы 2 (inline entries, индекс {}): {}", i, res.error());
				return report;
			}

			if (!allGuids.insert(entry.GetGuid()).second)
			{
				report.isValid = false;
				report.errorMessage = std::format("Обнаружен дубликат GUID '{}' в записях TOC", entry.GetGuid().ToString());
				return report;
			}

			const auto assetType = static_cast<core::eDataDatType>(entry.GetAssetType());
			if (!IsInlineType(assetType))
			{
				report.isValid = false;
				report.errorMessage = std::format("Запись Таблицы 2 имеет внешний тип ассета: {}", entry.GetAssetType());
				return report;
			}

			if (entry.GetSize() == 0)
			{
				report.isValid = false;
				report.errorMessage = std::format("Запись Таблицы 2 (GUID: {}) имеет нулевой размер", entry.GetGuid().ToString());
				return report;
			}

			inlineIntervals.push_back(Interval{ entry.GetOffset(), entry.GetOffset() + entry.GetSize(), entry.GetGuid() });
			report.inlineEntries.push_back(entry);
		}

		// 2.3. Таблица 3: externalCount элементов PackageEntry
		std::unordered_map<core::eDataDatType, std::vector<Interval>> externalIntervals;
		report.externalEntries.reserve(extra.externalCount);
		for (zU32 i = 0; i < extra.externalCount; ++i)
		{
			core::PackageEntry entry;
			auto res = serializer.Deserialize(dataDatBytes, offset, entry);
			if (!res)
			{
				report.isValid = false;
				report.errorMessage = std::format("Ошибка чтения Таблицы 3 (external entries, индекс {}): {}", i, res.error());
				return report;
			}

			if (!allGuids.insert(entry.GetGuid()).second)
			{
				report.isValid = false;
				report.errorMessage = std::format("Обнаружен дубликат GUID '{}' в записях TOC", entry.GetGuid().ToString());
				return report;
			}

			const auto assetType = static_cast<core::eDataDatType>(entry.GetAssetType());
			if (!IsValidExternalPakType(assetType))
			{
				report.isValid = false;
				report.errorMessage = std::format("Запись Таблицы 3 имеет недопустимый тип внешнего пака: {}", entry.GetAssetType());
				return report;
			}

			if (!manifestTypeSet.contains(assetType))
			{
				report.isValid = false;
				report.errorMessage = std::format("Запись Таблицы 3 (GUID: {}) ссылается на тип {}, отсутствующий в Таблице 1",
					entry.GetGuid().ToString(), entry.GetAssetType());
				return report;
			}

			if (entry.GetSize() == 0)
			{
				report.isValid = false;
				report.errorMessage = std::format("Запись Таблицы 3 (GUID: {}) имеет нулевой размер", entry.GetGuid().ToString());
				return report;
			}

			if (entry.GetOffset() < 4096)
			{
				report.isValid = false;
				report.errorMessage = std::format("Запись Таблицы 3 (GUID: {}) имеет смещение < 4096: {}",
					entry.GetGuid().ToString(), entry.GetOffset());
				return report;
			}

			if (entry.GetOffset() % 16 != 0)
			{
				report.isValid = false;
				report.errorMessage = std::format("Запись Таблицы 3 (GUID: {}) не выровнена по 16 байтам: смещение {}",
					entry.GetGuid().ToString(), entry.GetOffset());
				return report;
			}

			externalIntervals[assetType].push_back(Interval{ entry.GetOffset(), entry.GetOffset() + entry.GetSize(), entry.GetGuid() });
			report.externalEntries.push_back(entry);
		}

		// 2.4. Проверка границ TOC и полезной нагрузки data.dat
		const std::size_t tocEnd = 64 + extra.packCount * sizeof(zU32) +
		                           (extra.inlineCount + extra.externalCount) * core::PackageEntry::BinarySize();
		const std::size_t payloadBegin = (tocEnd + 15) & ~static_cast<std::size_t>(15);

		if (offset != tocEnd)
		{
			report.isValid = false;
			report.errorMessage = std::format("Фактическое смещение конца TOC ({}) не совпадает с расчётным ({})", offset, tocEnd);
			return report;
		}

		for (const auto& entry : report.inlineEntries)
		{
			if (entry.GetOffset() < payloadBegin)
			{
				report.isValid = false;
				report.errorMessage = std::format("Смещение inline ресурса {} ({}) меньше payloadBegin ({})",
					entry.GetGuid().ToString(), entry.GetOffset(), payloadBegin);
				return report;
			}

			if (entry.GetOffset() % 16 != 0)
			{
				report.isValid = false;
				report.errorMessage = std::format("Смещение inline ресурса {} ({}) не кратно 16",
					entry.GetGuid().ToString(), entry.GetOffset());
				return report;
			}

			if (entry.GetOffset() + entry.GetSize() > dataDatBytes.size())
			{
				report.isValid = false;
				report.errorMessage = std::format("Inline ресурс {} выходит за границы файла data.dat ({} + {} > {})",
					entry.GetGuid().ToString(), entry.GetOffset(), entry.GetSize(), dataDatBytes.size());
				return report;
			}
		}

		std::string overlapErr;
		if (!CheckNonOverlapping(inlineIntervals, overlapErr, "data.dat"))
		{
			report.isValid = false;
			report.errorMessage = overlapErr;
			return report;
		}

		// 3. Валидация внешних пакетов N.dat
		for (const auto pakType : report.packManifestTypes)
		{
			const auto pakName = core::GetPakFileName(pakType);
			const auto pakPath = assetsDir / pakName;

			if (!std::filesystem::exists(pakPath, ec))
			{
				report.isValid = false;
				report.errorMessage = std::format("Манифест Таблицы 1 объявляет пак '{}', но файл отсутствует на диске", pakName);
				return report;
			}

			const auto pakFileSize = std::filesystem::file_size(pakPath, ec);
			if (pakFileSize < core::PakFileHeader::c_FullHeaderSize)
			{
				report.isValid = false;
				report.errorMessage = std::format("Файл '{}' повреждён (размер {} < 4096 байт)", pakName, pakFileSize);
				return report;
			}

			core::PakFileHeader pakHeader(core::c_PakFileFormat);
			{
				std::ifstream f(pakPath, std::ios::binary);
				std::vector<std::byte> headBytes(core::PakFileHeader::c_FullHeaderSize);
				f.read(reinterpret_cast<char*>(headBytes.data()), headBytes.size());
				auto desRes = pakHeader.DeserializeAndValidate(headBytes, pakFileSize);
				if (!desRes)
				{
					report.isValid = false;
					report.errorMessage = std::format("Невалидный заголовок в '{}': {}", pakName, desRes.error());
					return report;
				}
			}

			if (pakHeader.GetTimestamp() != report.dataDatTimestamp)
			{
				report.isValid = false;
				report.errorMessage = std::format("Рассинхронизация timestamp в '{}' ({}) относительно data.dat ({})",
					pakName, pakHeader.GetTimestamp(), report.dataDatTimestamp);
				return report;
			}

			auto& intervals = externalIntervals[pakType];
			if (pakHeader.GetEntryCount() != intervals.size())
			{
				report.isValid = false;
				report.errorMessage = std::format("entryCount в заголовке '{}' ({}) не равен числу записей в Таблице 3 ({})",
					pakName, pakHeader.GetEntryCount(), intervals.size());
				return report;
			}

			for (const auto& iv : intervals)
			{
				if (iv.end > pakFileSize)
				{
					report.isValid = false;
					report.errorMessage = std::format("Ресурс {} в '{}' выходит за границы файла: {:#x} > {:#x}",
						iv.guid.ToString(), pakName, iv.end, pakFileSize);
					return report;
				}
			}

			if (!CheckNonOverlapping(intervals, overlapErr, pakName))
			{
				report.isValid = false;
				report.errorMessage = overlapErr;
				return report;
			}
		}

		// 4. Проверка на посторонние .dat файлы пакетов в assetsDir
		for (const auto& entry : std::filesystem::directory_iterator(assetsDir, ec))
		{
			if (entry.is_regular_file() && entry.path().extension() == core::c_DatExtension)
			{
				const auto fn = entry.path().filename().string();
				if (fn == core::c_GamePackageFileName || fn == core::c_DataPackageFileName)
				{
					continue;
				}

				bool isKnown = false;
				for (const auto pt : report.packManifestTypes)
				{
					if (core::GetPakFileName(pt) == fn)
					{
						isKnown = true;
						break;
					}
				}
				if (!isKnown)
				{
					report.isValid = false;
					report.errorMessage = std::format("Обнаружен посторонний .dat пакет '{}', не заявленный в Манифесте data.dat", fn);
					return report;
				}
			}
		}


		report.isValid = true;
		return report;
	}
}
