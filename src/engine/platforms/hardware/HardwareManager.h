#pragma once

#include "core/hardware/HardwareState.h"
#include "engine/platforms/hardware/CpuInfoCollector.h"
#include "engine/platforms/hardware/RamInfoCollector.h"
#include "engine/platforms/hardware/MotherboardInfoCollector.h"
#include "engine/platforms/hardware/GpuInfoCollector.h"
#include "engine/platforms/hardware/StorageInfoCollector.h"
#include "engine/platforms/hardware/NetworkAdapterInfoCollector.h"

namespace zzz::engine
{
	class IMonitorProvider;

	/**
	 * @brief Сервис сбора полного среза аппаратной телеметрии платформы (CPU, RAM, GPU, Storage, Network, Motherboard).
	 * Ноль #ifdef в теле класса: каждая категория телеметрии инкапсулирована в своём коллекторе
	 * с собственным compile-time alias на платформенную реализацию (Правило 29).
	 * Список мониторов не собирается напрямую, а запрашивается у существующего IMonitorProvider.
	 */
	class HardwareManager final
	{
	public:
		HardwareManager() = delete;
		explicit HardwareManager(const IMonitorProvider& monitorProvider);

		[[nodiscard]] const HardwareState& GetHardwareState() const noexcept { return m_HardwareState; }

	private:
		[[nodiscard]] HardwareState Gather(const IMonitorProvider& monitorProvider) const;

		CpuInfoCollector m_CpuCollector;
		RamInfoCollector m_RamCollector;
		MotherboardInfoCollector m_MotherboardCollector;
		GpuInfoCollector m_GpuCollector;
		StorageInfoCollector m_StorageCollector;
		NetworkAdapterInfoCollector m_NetworkCollector;

		// m_HardwareState объявлен последним: Gather() использует уже сконструированные коллекторы выше
		// (порядок инициализации членов определяется порядком объявления, а не списком инициализации).
		HardwareState m_HardwareState;
	};
}
