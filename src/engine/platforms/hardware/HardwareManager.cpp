#include "HardwareManager.h"
#include "engine/platforms/monitor/IMonitorProvider.h"

using namespace zzz::engine;

HardwareManager::HardwareManager(const IMonitorProvider& monitorProvider)
	: m_HardwareState(Gather(monitorProvider))
{
}

HardwareState HardwareManager::Gather(const IMonitorProvider& monitorProvider) const
{
	return HardwareState(
		m_CpuCollector.Collect(),
		m_CpuCollector.CollectTopology(),
		m_RamCollector.Collect(),
		m_MotherboardCollector.Collect(),
		m_GpuCollector.Collect(),
		monitorProvider.GetMonitors(),
		m_StorageCollector.Collect(),
		m_NetworkCollector.Collect()
	);
}
