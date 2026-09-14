#pragma once

#include <cstdint>
#include <string_view>
#include <logger/logger.h>

namespace zzz::engine
{
	/**
	 * @struct TaskDispatcherConfig
	 * @brief Конфигурация пулов потоков TaskDispatcher с поддержкой гетерогенных и однородных топологий.
	 */
	struct TaskDispatcherConfig
	{
		bool isHeterogeneous{ false };

		// Однородный режим (SMP) / малоядерный фоллбэк
		uint32_t commonThreads{ 0 };

		// Гетерогенный режим (Prime, Perf, Eff)
		uint32_t primeThreads{ 0 };
		uint32_t perfThreads{ 0 };
		uint32_t effThreads{ 0 };

		// Выделенный кадровый пул (Critical)
		uint32_t criticalThreads{ 2 };

		inline void LogFileBlock([[maybe_unused]] std::string_view indentation = {}) const
		{
#if Z_ADD_LOGGER
			const std::string nestedIndentation = std::string(indentation) + "  ";
			DOut(::zzz::core::Hardware, "{}[TaskDispatcherConfig]", indentation);
			if (isHeterogeneous)
			{
				DOut(::zzz::core::Hardware, "{}mode: Heterogeneous", nestedIndentation);
				DOut(::zzz::core::Hardware, "{}criticalThreads: {}", nestedIndentation, criticalThreads);
				if (primeThreads > 0)
					DOut(::zzz::core::Hardware, "{}primeThreads: {}", nestedIndentation, primeThreads);
				DOut(::zzz::core::Hardware, "{}perfThreads: {}", nestedIndentation, perfThreads);
				DOut(::zzz::core::Hardware, "{}effThreads: {}", nestedIndentation, effThreads);
			}
			else
			{
				DOut(::zzz::core::Hardware, "{}mode: Homogeneous", nestedIndentation);
				if (criticalThreads > 0)
					DOut(::zzz::core::Hardware, "{}criticalThreads: {}", nestedIndentation, criticalThreads);
				DOut(::zzz::core::Hardware, "{}commonThreads: {}", nestedIndentation, commonThreads);
			}
#endif
		}
	};
}
