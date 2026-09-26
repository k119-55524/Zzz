#include "Stage3_PakBuilder.h"
#include <format>
#include "core/constants/PackagesConstants.h"
#include "core/logger/logger.h"
#include "pak_builders/StandardExternalPakBuilder.h"
#include "pak_builders/InlineDataDatBuilder.h"
#include "pak_builders/PackageDatBuilder.h"
#include "validation/BuiltPackageValidator.h"

namespace zzz::builder
{
	bool Stage3_PakBuilder::BuildAndPublish(
		const StagePackPlan& plan,
		const std::filesystem::path& destinationDir,
		std::string& outErrorMessage)
	{
		std::error_code ec;

		const auto assetsDir = destinationDir / core::c_AssetsDirectoryName;
		const auto assetsOldDir = destinationDir / "assets.old";
		const auto stagingDir = destinationDir / ".staging";
		const auto stagingOutputDir = stagingDir / "output" / "assets.new";

		// 1. Recovery незавершённого swap и подготовка .staging
		if (!std::filesystem::exists(assetsDir, ec) && std::filesystem::exists(assetsOldDir, ec))
		{
			// Прошлый swap был аварийно прерван между rename assets->assets.old и assets.new->assets
			DOutWarning("Stage3_PakBuilder: Обнаружен незавершённый swap, восстановление assets.old -> assets...");
			std::filesystem::rename(assetsOldDir, assetsDir, ec);
			if (ec)
			{
				outErrorMessage = std::format("Не удалось восстановить assets из assets.old: {}", ec.message());
				return false;
			}
		}
		else if (std::filesystem::exists(assetsDir, ec) && std::filesystem::exists(assetsOldDir, ec))
		{
			// Прошлый swap успешно опубликовал assets, но не успел удалить старый backup
			std::filesystem::remove_all(assetsOldDir, ec);
		}

		// Очищаем исключительно .staging/
		std::filesystem::remove_all(stagingDir, ec);
		std::filesystem::create_directories(stagingOutputDir, ec);
		if (ec)
		{
			outErrorMessage = std::format("Не удалось создать директорию staging: {}", ec.message());
			return false;
		}

		// 2. Сборка пакетов в stagingOutputDir
		// 2.1. Внешние паки (0.dat, 1.dat и т.д.)
		StandardExternalPakBuilder externalBuilder;
		auto extRes = externalBuilder.Build(plan, stagingOutputDir);
		if (!extRes.success)
		{
			outErrorMessage = extRes.errorMessage;
			std::filesystem::remove_all(stagingDir, ec);
			return false;
		}

		// 2.2. data.dat (с 3-мя таблицами TOC)
		InlineDataDatBuilder inlineBuilder(extRes.entries);
		auto dataRes = inlineBuilder.Build(plan, stagingOutputDir);
		if (!dataRes.success)
		{
			outErrorMessage = dataRes.errorMessage;
			std::filesystem::remove_all(stagingDir, ec);
			return false;
		}

		// 2.3. package.dat
		PackageDatBuilder pkgBuilder;
		auto pkgRes = pkgBuilder.Build(plan, stagingOutputDir);
		if (!pkgRes.success)
		{
			outErrorMessage = pkgRes.errorMessage;
			std::filesystem::remove_all(stagingDir, ec);
			return false;
		}

		// 2.4. Полная валидация готового комплекта пакетов перед публикацией
		auto valReport = BuiltPackageValidator::Validate(stagingOutputDir);
		if (!valReport.isValid)
		{
			outErrorMessage = std::format("Валидация собранных архивов перед публикацией не прошла: {}", valReport.errorMessage);
			std::filesystem::remove_all(stagingDir, ec);
			return false;
		}

		// 3. Публикация (Recoverable Directory Swap)
		if (std::filesystem::exists(assetsDir, ec))
		{
			std::filesystem::rename(assetsDir, assetsOldDir, ec);
			if (ec)
			{
				outErrorMessage = std::format("Не удалось переместить существующий assets в assets.old: {}", ec.message());
				std::filesystem::remove_all(stagingDir, ec);
				return false;
			}
		}

		std::filesystem::rename(stagingOutputDir, assetsDir, ec);
		if (ec)
		{
			outErrorMessage = std::format("Не удалось перенести assets.new в assets: {}. Выполняется откат...", ec.message());
			std::error_code rollEc;
			if (std::filesystem::exists(assetsOldDir, rollEc))
			{
				std::filesystem::rename(assetsOldDir, assetsDir, rollEc);
			}
			std::filesystem::remove_all(stagingDir, rollEc);
			return false;
		}

		// Swap завершён успешно, удаляем backup
		if (std::filesystem::exists(assetsOldDir, ec))
		{
			std::filesystem::remove_all(assetsOldDir, ec);
		}

		// 4. Очистка временного каталога
		std::filesystem::remove_all(stagingDir, ec);

		return true;
	}
}
