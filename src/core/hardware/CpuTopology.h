#pragma once

#include "core/CoreIncludes.h"
#include <logger/logger.h>

namespace zzz::core
{
	/**
	 * @struct CpuTopology
	 * @brief Аппаратная топология процессора, описывающая кластеры ядер (Prime, Performance, Efficiency).
	 */
	struct CpuTopology
	{
		std::string name{ "Unknown CPU" };
		std::string architecture{ "x64" };
		uint32_t totalLogicalCores{ 1 };
		uint32_t totalPhysicalCores{ 1 };

		uint32_t primeLogicalCapacity{ 0 };
		uint32_t performanceLogicalCapacity{ 0 };
		uint32_t efficiencyLogicalCapacity{ 0 };

		[[nodiscard]] bool operator==(const CpuTopology& other) const noexcept
		{
			return name == other.name &&
				architecture == other.architecture &&
				totalLogicalCores == other.totalLogicalCores &&
				totalPhysicalCores == other.totalPhysicalCores &&
				primeLogicalCapacity == other.primeLogicalCapacity &&
				performanceLogicalCapacity == other.performanceLogicalCapacity &&
				efficiencyLogicalCapacity == other.efficiencyLogicalCapacity;
		}

		inline void LogFileBlock([[maybe_unused]] std::string_view indentation = {}) const
		{
#if Z_ADD_LOGGER
			const std::string nestedIndentation = std::string(indentation) + "  ";
			DOut(::zzz::core::Hardware, "{}[CpuTopology]", indentation);
			DOut(::zzz::core::Hardware, "{}name: {}", nestedIndentation, name);
			DOut(::zzz::core::Hardware, "{}arch: {}", nestedIndentation, architecture);
			DOut(::zzz::core::Hardware, "{}totalLogicalCores: {}", nestedIndentation, totalLogicalCores);
			DOut(::zzz::core::Hardware, "{}totalPhysicalCores: {}", nestedIndentation, totalPhysicalCores);
			if (primeLogicalCapacity > 0)
				DOut(::zzz::core::Hardware, "{}primeLogicalCapacity: {}", nestedIndentation, primeLogicalCapacity);
			DOut(::zzz::core::Hardware, "{}performanceLogicalCapacity: {}", nestedIndentation, performanceLogicalCapacity);
			if (efficiencyLogicalCapacity > 0)
				DOut(::zzz::core::Hardware, "{}efficiencyLogicalCapacity: {}", nestedIndentation, efficiencyLogicalCapacity);
#endif
		}
	};
}
