#include "GpuInfoCollectorMSWin.h"

#if defined(Z_WINDOWS)

#include "engine/utils/GpuUtils.h"
#include "engine/platforms/hardware/platforms/common/WCharUtilsMSWin.h"

using namespace zzz::engine;
using namespace zzz::core;

std::vector<GpuInfo> GpuInfoCollectorMSWin::Collect() const
{
	std::vector<GpuInfo> gpus;

	// На Windows ВСЕГДА используем системный DXGI для перечисления GPU (и для DX12, и для Vulkan)
	Microsoft::WRL::ComPtr<IDXGIFactory1> factory;
	if (SUCCEEDED(CreateDXGIFactory1(IID_PPV_ARGS(&factory))))
	{
		Microsoft::WRL::ComPtr<IDXGIAdapter1> adapter;
		for (UINT i = 0; SUCCEEDED(factory->EnumAdapters1(i, &adapter)); ++i)
		{
			DXGI_ADAPTER_DESC1 desc{};
			adapter->GetDesc1(&desc);

			// Игнорируем программные адаптеры (WARP) и невалидные устройства
			if ((desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) || (desc.VendorId == 0 && desc.DeviceId == 0))
				continue;

			std::string name = WCharToUtf8MSWin(desc.Description);

			eGPUType type = eGPUType::Integrated;
			if (desc.DedicatedVideoMemory >= 256 * 1024 * 1024)
				type = eGPUType::Discrete;

			std::string platformGpuId = GpuUtils::MakeId(desc.VendorId, desc.DeviceId, desc.SubSysId, desc.Revision);

			gpus.emplace_back(platformGpuId, name, desc.VendorId, desc.DeviceId, type,
				desc.DedicatedVideoMemory, desc.SharedSystemMemory, desc.DedicatedSystemMemory);
		}
	}

	return gpus;
}

#endif // defined(Z_WINDOWS)
