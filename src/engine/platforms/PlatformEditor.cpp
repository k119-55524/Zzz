#include "Platform.h"
#include "window/WinMSWindowEditor.h"

using namespace zzz::engine;

void Platform::ShutdownPlatformSpecific()
{
}

void Platform::InitializePlatformSpecific()
{
}

HardwareState Platform::GatherHardwareState() const
{
	std::vector<CpuInfo> cpus{ CpuInfo("Editor CPU", "x64", 1, 1, 1000) };
	RamInfo ram(1024 * 1024 * 1024, 512 * 1024 * 1024);
	MotherboardInfo motherboard("Editor Vendor", "Editor Model", "00000000-0000-0000-0000-000000000000");
	std::vector<GpuInfo> gpus{ GpuInfo("editor_gpu_id", "Editor GPU", 0, 0, eGPUType::Integrated, 1024 * 1024 * 1024, 0, 0) };
	std::vector<MonitorInfo> monitors{ MonitorInfo("editor_mon_id", "Editor Monitor", zzz::math::Size2D<zzz::zU32>(1920, 1080), zzz::math::Size2D<zzz::zU32>(1920, 1080), 0, 0, true) };

	return HardwareState(
		std::move(cpus),
		std::move(ram),
		std::move(motherboard),
		std::move(gpus),
		std::move(monitors));
}
