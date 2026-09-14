#pragma once

#include <logger.h>

#include "core/CoreIncludes.h"

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
			DOut(Hardware, "{}[CpuTopology]", indentation);
			DOut(Hardware, "{}name: {}", nestedIndentation, name);
			DOut(Hardware, "{}arch: {}", nestedIndentation, architecture);
			DOut(Hardware, "{}totalLogicalCores: {}", nestedIndentation, totalLogicalCores);
			DOut(Hardware, "{}totalPhysicalCores: {}", nestedIndentation, totalPhysicalCores);
			if (primeLogicalCapacity > 0)
				DOut(Hardware, "{}primeLogicalCapacity: {}", nestedIndentation, primeLogicalCapacity);
			DOut(Hardware, "{}performanceLogicalCapacity: {}", nestedIndentation, performanceLogicalCapacity);
			if (efficiencyLogicalCapacity > 0)
				DOut(Hardware, "{}efficiencyLogicalCapacity: {}", nestedIndentation, efficiencyLogicalCapacity);
#endif
		}
	};
}
