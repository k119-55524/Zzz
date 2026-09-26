#pragma once

#include <string>
#include <filesystem>
#include <core/Core.h>
#include <core/enums/eTargetPlatform.h>

namespace zzz::builder
{
	class PackagePacker final
	{
	public:
		/**
		 * @brief Начинает сессию сборки проекта. Выполняет Стадию 1 (валидация проекта) один раз и кэширует её.
		 */
		static bool BeginBuildSession(
			const std::filesystem::path& projectDir,
			std::string& outErrorMessage);

		/**
		 * @brief Завершает сессию сборки проекта и сбрасывает кэш.
		 */
		static void EndBuildSession() noexcept;

		/**
		 * @brief Упаковывает ресурсы проекта для указанной платформы (Стадия 2 + Стадия 3).
		 * При отсутствии открытой сессии выполняет Стадию 1 локально (fallback совместимости).
		 */
		static bool PackProject(
			const std::filesystem::path& sourceDir,
			const std::filesystem::path& destinationDir,
			zzz::core::eTargetPlatform targetPlatform,
			const std::string& platformConfigFile = "",
			uint64_t buildTimestamp = 0,
			uint64_t* outBuildTimestamp = nullptr,
			std::string* outErrorMessage = nullptr);
	};
}
