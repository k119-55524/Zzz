#pragma once

#include <filesystem>
#include <string>
#include "StagePackPlan.h"

namespace zzz::builder
{
	/**
	 * @class Stage3_PakBuilder
	 * @brief Стадия 3 модульного конвейера сборщика (индивидуальна для каждого таргета).
	 *
	 * Реализует строгий транзакционный цикл Recover-Build-Validate-Swap-Cleanup:
	 * 1. Recovery незавершённого swap и очистка временной папки .staging/;
	 * 2. Сборка внешних паков (0.dat, 1.dat), data.dat и package.dat в .staging/output/assets.new/;
	 * 3. Полная независимая валидация через BuiltPackageValidator;
	 * 4. Безопасный directory swap с откатом при ошибке;
	 * 5. Очистка .staging/ и старых резервных копий.
	 */
	class Stage3_PakBuilder
	{
	public:
		[[nodiscard]] static bool BuildAndPublish(
			const StagePackPlan& plan,
			const std::filesystem::path& destinationDir,
			std::string& outErrorMessage);
	};
}
