#include "PackagePacker.h"
#include <mutex>
#include <chrono>
#include <format>
#include "core/logger/logger.h"
#include "stages/Stage1_ProjectValidator.h"
#include "stages/Stage2_PackPlanner.h"
#include "stages/Stage3_PakBuilder.h"

namespace zzz::builder
{
	namespace
	{
		struct SessionState
		{
			bool                  active{ false };
			std::filesystem::path projectDir;
			std::shared_ptr<const StageValidationResult> valResult;
			std::unordered_map<std::string, FileSnapshot> trackedExternalPlatformConfigs;
		};

		static std::mutex   s_SessionMutex;
		static SessionState s_Session;
	}

	bool PackagePacker::BeginBuildSession(
		const std::filesystem::path& projectDir,
		std::string& outErrorMessage)
	{
		std::lock_guard lock(s_SessionMutex);

		if (s_Session.active)
		{
			outErrorMessage = "Сессия сборки уже активна. Завершите предыдущую сессию вызовом EndBuildSession.";
			return false;
		}

		std::error_code ec;
		auto canonProj = std::filesystem::weakly_canonical(projectDir, ec);

		auto valRes = Stage1_ProjectValidator::Validate(projectDir);
		if (!valRes.isValid)
		{
			outErrorMessage = valRes.errorMessage;
			return false;
		}

		s_Session.active = true;
		s_Session.projectDir = canonProj;
		s_Session.valResult = std::make_shared<const StageValidationResult>(std::move(valRes));
		s_Session.trackedExternalPlatformConfigs.clear();

		return true;
	}

	void PackagePacker::EndBuildSession() noexcept
	{
		std::lock_guard lock(s_SessionMutex);
		s_Session.active = false;
		s_Session.projectDir.clear();
		s_Session.valResult.reset();
		s_Session.trackedExternalPlatformConfigs.clear();
	}

	bool PackagePacker::PackProject(
		const std::filesystem::path& sourceDir,
		const std::filesystem::path& destinationDir,
		zzz::core::eTargetPlatform targetPlatform,
		const std::string& platformConfigFile,
		uint64_t buildTimestamp,
		uint64_t* outBuildTimestamp,
		std::string* outErrorMessage)
	{
		uint64_t actualTimestamp = buildTimestamp;
		if (actualTimestamp == 0)
		{
			actualTimestamp = static_cast<uint64_t>(
				std::chrono::duration_cast<std::chrono::milliseconds>(
					std::chrono::system_clock::now().time_since_epoch()).count());
		}
		if (outBuildTimestamp)
		{
			*outBuildTimestamp = actualTimestamp;
		}

		std::shared_ptr<const StageValidationResult> pVal;

		{
			std::lock_guard lock(s_SessionMutex);
			if (s_Session.active)
			{
				std::error_code ec;
				auto canonSource = std::filesystem::weakly_canonical(sourceDir, ec);
				if (canonSource != s_Session.projectDir)
				{
					std::string err = std::format("sourceDir '{}' не совпадает с projectDir активной сессии '{}'",
						sourceDir.string(), s_Session.projectDir.string());
					if (outErrorMessage) *outErrorMessage = err;
					DOutError("{}", err);
					return false;
				}

				std::string snapErr;
				if (!s_Session.valResult->CheckSnapshotConsistent(&snapErr))
				{
					if (outErrorMessage) *outErrorMessage = snapErr;
					DOutError("{}", snapErr);
					return false;
				}

				// Проверка платформенного конфигурационного файла (если задан)
				if (!platformConfigFile.empty())
				{
					std::filesystem::path p(platformConfigFile);
					std::filesystem::path configPath = p.is_absolute() ? p : (s_Session.projectDir / p);
					if (!std::filesystem::exists(configPath, ec))
					{
						configPath = s_Session.projectDir / "build_settings" / p;
					}

					if (std::filesystem::exists(configPath, ec))
					{
						if (!s_Session.valResult->CheckPlatformConfigFile(configPath, &snapErr))
						{
							if (outErrorMessage) *outErrorMessage = snapErr;
							DOutError("{}", snapErr);
							return false;
						}

						const auto key = configPath.string();
						const auto curTime = std::filesystem::last_write_time(configPath, ec);
						const auto curSize = std::filesystem::file_size(configPath, ec);
						auto it = s_Session.trackedExternalPlatformConfigs.find(key);
						if (it != s_Session.trackedExternalPlatformConfigs.end())
						{
							if (it->second.lastWriteTime != curTime || it->second.fileSize != curSize)
							{
								std::string err = std::format("Платформенный конфигурационный файл '{}' изменён во время сессии сборки. Пожалуйста, начните новую сессию сборки.", key);
								if (outErrorMessage) *outErrorMessage = err;
								DOutError("{}", err);
								return false;
							}
						}
						else
						{
							s_Session.trackedExternalPlatformConfigs.emplace(key, FileSnapshot{ curTime, curSize });
						}
					}
				}

				pVal = s_Session.valResult;
			}
		}

		// Fallback обратной совместимости, если PackProject вызван вне сессии
		if (!pVal)
		{
			auto localVal = Stage1_ProjectValidator::Validate(sourceDir);
			if (!localVal.isValid)
			{
				if (outErrorMessage) *outErrorMessage = localVal.errorMessage;
				DOutError("{}", localVal.errorMessage);
				return false;
			}
			pVal = std::make_shared<const StageValidationResult>(std::move(localVal));
		}

		// Стадия 2: Планирование состава ресурсов таргета
		auto plan = Stage2_PackPlanner::Plan(
			*pVal,
			sourceDir,
			destinationDir,
			targetPlatform,
			platformConfigFile,
			actualTimestamp);

		if (!plan.isValid)
		{
			if (outErrorMessage) *outErrorMessage = plan.errorMessage;
			DOutError("{}", plan.errorMessage);
			return false;
		}

		// Стадия 3: Изолированная сборка и безопасная публикация
		std::string buildErr;
		if (!Stage3_PakBuilder::BuildAndPublish(plan, destinationDir, buildErr))
		{
			if (outErrorMessage) *outErrorMessage = buildErr;
			DOutError("{}", buildErr);
			return false;
		}

		return true;
	}
}
