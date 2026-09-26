#pragma once

#include <filesystem>
#include "StageValidationResult.h"

namespace zzz::builder
{
	/**
	 * @class Stage1_ProjectValidator
	 * @brief Стадия 1 модульного конвейера сборщика (общая на сессию сборки).
	 *
	 * Сканирует директорию Assets/, проверяет целостность и уникальность GUID,
	 * валидирует .meta файлы и собирает полный реестр ассетов проекта.
	 */
	class Stage1_ProjectValidator
	{
	public:
		[[nodiscard]] static StageValidationResult Validate(const std::filesystem::path& projectDir);
	};
}
