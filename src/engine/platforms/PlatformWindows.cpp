
#include "Platform.h"
#include "window/WinMSWindows.h"
#include "monitor/IMonitorProvider.h"
#include "engine/utils/MonitorUtils.h"
#include "engine/utils/GpuUtils.h"

using namespace zzz::engine;

void Platform::ShutdownPlatformSpecific()
{
	const auto& windowClassName = m_PlatformData.GetWindowClassName();
	const BOOL result = UnregisterClass(windowClassName.c_str(), GetModuleHandle(nullptr));
	if (!result)
	{
		const DWORD error = GetLastError();
		DOutCritical("Не удалось отменить регистрацию класса окна '{}'. Код ошибки: {}.", windowClassName, error);
	}
}

void Platform::InitializePlatformSpecific()
{
	SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

	HICON iconHandle = (HICON)LoadImage(
		GetModuleHandle(NULL),
		c_IcoResourceName.data(),
		IMAGE_ICON,
		0,
		0,
		LR_DEFAULTSIZE | LR_SHARED);

	if (!iconHandle)
		DOutWarning("Не удалось загрузить иконку '{}'. Ошибка: {}", c_IcoResourceName.data(), GetLastError());

	WNDCLASS wc = { 0 };
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WinMSWindows::WindowProc;
	wc.hInstance = GetModuleHandle(NULL);
	wc.hIcon = iconHandle;
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpszClassName = m_PlatformData.GetWindowClassName().c_str();
	ATOM Result = RegisterClass(&wc);
	if (Result == 0)
		THROW_RUNTIME("Не удалось зарегистрировать класс окна. Код ошибки: {}.", GetLastError());
}

namespace
{
	BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData)
	{
		(void)hdcMonitor;
		(void)lprcMonitor;

		auto* monitors = reinterpret_cast<std::vector<MonitorInfo>*>(dwData);
		MONITORINFOEXW mi{};
		mi.cbSize = sizeof(MONITORINFOEXW);

		if (GetMonitorInfoW(hMonitor, &mi))
		{
			int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, mi.szDevice, -1, NULL, 0, NULL, NULL);
			std::string systemId(sizeNeeded > 1 ? sizeNeeded - 1 : 0, 0);
			if (sizeNeeded > 1)
			{
				WideCharToMultiByte(CP_UTF8, 0, mi.szDevice, -1, systemId.data(), sizeNeeded, NULL, NULL);
			}
			Size2D<zU32> resolution{
				static_cast<zU32>(mi.rcMonitor.right - mi.rcMonitor.left),
				static_cast<zU32>(mi.rcMonitor.bottom - mi.rcMonitor.top)
			};
			bool isPrimary = (mi.dwFlags & MONITORINFOF_PRIMARY) != 0;

			std::string platformMonitorId = MonitorUtils::MakeId(systemId);
			MonitorInfo info(platformMonitorId, systemId, resolution, mi.rcMonitor.left, mi.rcMonitor.top, isPrimary);
			monitors->push_back(info);
		}
		return TRUE;
	}
}

HardwareState Platform::GatherHardwareState() const
{
	std::vector<MonitorInfo> monitors;
	std::vector<GpuInfo> gpus;
	std::vector<CpuInfo> cpus;
	std::vector<StorageInfo> storages;
	std::vector<NetworkAdapterInfo> networkAdapters;

	// 1. Мониторы (из подсистемы IMonitorProvider)
	monitors = m_MonitorProvider ? m_MonitorProvider->GetMonitors() : std::vector<MonitorInfo>{};

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

			int nameSizeNeeded = WideCharToMultiByte(CP_UTF8, 0, desc.Description, -1, NULL, 0, NULL, NULL);
			std::string name(nameSizeNeeded > 1 ? nameSizeNeeded - 1 : 0, 0);
			if (nameSizeNeeded > 1)
			{
				WideCharToMultiByte(CP_UTF8, 0, desc.Description, -1, name.data(), nameSizeNeeded, NULL, NULL);
			}

			eGPUType type = eGPUType::Integrated;
			if (desc.DedicatedVideoMemory >= 256 * 1024 * 1024)
				type = eGPUType::Discrete;

			std::string platformGpuId = GpuUtils::MakeId(desc.VendorId, desc.DeviceId, desc.SubSysId, desc.Revision);

			GpuInfo gpu(platformGpuId, name, desc.VendorId, desc.DeviceId, type,
				desc.DedicatedVideoMemory, desc.SharedSystemMemory, desc.DedicatedSystemMemory);

			gpus.push_back(gpu);
		}
	}

	// 3. Процессоры
	SYSTEM_INFO sysInfo{};
	GetSystemInfo(&sysInfo);
	CpuInfo cpu("x86 Processor", "x86_64", sysInfo.dwNumberOfProcessors, sysInfo.dwNumberOfProcessors, 0);
	cpus.push_back(cpu);

	// 4. ОЗУ (SMBIOS & GlobalMemoryStatusEx)
	RamInfo ram{};
	MEMORYSTATUSEX memStatus{};
	memStatus.dwLength = sizeof(MEMORYSTATUSEX);
	zU64 totalPhys = 1024 * 1024 * 1024;
	zU64 availPhys = 512 * 1024 * 1024;
	if (GlobalMemoryStatusEx(&memStatus))
	{
		totalPhys = memStatus.ullTotalPhys;
		availPhys = memStatus.ullAvailPhys;
	}

	eRamType ramType = eRamType::Unknown;
	zU32 ramSpeed = 0;

	DWORD smbiosSize = GetSystemFirmwareTable('RSMB', 0, nullptr, 0);
	if (smbiosSize > 0)
	{
		std::vector<std::byte> smbiosData(smbiosSize);
		if (GetSystemFirmwareTable('RSMB', 0, smbiosData.data(), smbiosSize) == smbiosSize)
		{
			// Парсинг структуры SMBIOS (Type 17 - Memory Device)
			const std::byte* ptr = smbiosData.data() + 8; // Пропускаем заголовок RawSMBIOSData (8 байт)
			const std::byte* end = smbiosData.data() + smbiosSize;

			while (ptr + 4 <= end)
			{
				zU8 type = static_cast<zU8>(ptr[0]);
				zU8 length = static_cast<zU8>(ptr[1]);
				if (length < 4 || ptr + length > end) break;

				if (type == 17 && length >= 0x15) // Type 17: Memory Device
				{
					zU8 smbiosRamType = static_cast<zU8>(ptr[0x12]);
					zU16 configuredSpeed = *reinterpret_cast<const zU16*>(ptr + 0x15);
					if (length >= 0x22 && configuredSpeed == 0)
					{
						configuredSpeed = *reinterpret_cast<const zU16*>(ptr + 0x20); // ConfiguredClockSpeed
					}

					if (configuredSpeed > ramSpeed)
						ramSpeed = configuredSpeed;

					if (ramType == eRamType::Unknown)
					{
						switch (smbiosRamType)
						{
						case 0x18: ramType = eRamType::DDR3; break;
						case 0x1A: ramType = eRamType::DDR4; break;
						case 0x22: ramType = eRamType::DDR5; break;
						case 0x1E: ramType = eRamType::LPDDR4; break;
						case 0x23: ramType = eRamType::LPDDR5; break;
						}
					}
				}

				// Переход к следующей структуре в SMBIOS (пропускаем форматированную часть + текстовый блок с двумя нулевыми байтами)
				ptr += length;
				while (ptr + 1 < end && !(ptr[0] == std::byte{ 0 } && ptr[1] == std::byte{ 0 }))
				{
					ptr++;
				}
				ptr += 2;
			}
		}
	}

	ram = RamInfo(totalPhys, availPhys, ramType, ramSpeed);

	// 5. Материнская плата (Windows Registry SMBIOS)
	std::string mbVendor = "Unknown";
	std::string mbModel = "Unknown";
	std::string sysUuid = "00000000-0000-0000-0000-000000000000";

	auto WCharToUtf8 = [](const WCHAR* wstr) -> std::string {
		if (!wstr || !*wstr) return "";
		int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
		std::string result(sizeNeeded > 1 ? sizeNeeded - 1 : 0, 0);
		if (sizeNeeded > 1) {
			WideCharToMultiByte(CP_UTF8, 0, wstr, -1, result.data(), sizeNeeded, NULL, NULL);
		}
		return result;
	};

	HKEY hKey = nullptr;
	if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"HARDWARE\\DESCRIPTION\\System\\BIOS", 0, KEY_READ, &hKey) == ERROR_SUCCESS)
	{
		WCHAR buffer[256]{};
		DWORD bufSize = sizeof(buffer);

		if (RegQueryValueExW(hKey, L"BaseBoardManufacturer", nullptr, nullptr, reinterpret_cast<LPBYTE>(buffer), &bufSize) == ERROR_SUCCESS)
		{
			std::string s = WCharToUtf8(buffer);
			if (!s.empty()) mbVendor = std::move(s);
		}

		bufSize = sizeof(buffer);
		if (RegQueryValueExW(hKey, L"BaseBoardProduct", nullptr, nullptr, reinterpret_cast<LPBYTE>(buffer), &bufSize) == ERROR_SUCCESS)
		{
			std::string s = WCharToUtf8(buffer);
			if (!s.empty()) mbModel = std::move(s);
		}

		bufSize = sizeof(buffer);
		if (RegQueryValueExW(hKey, L"SystemSKU", nullptr, nullptr, reinterpret_cast<LPBYTE>(buffer), &bufSize) != ERROR_SUCCESS)
		{
			bufSize = sizeof(buffer);
			RegQueryValueExW(hKey, L"SystemProductName", nullptr, nullptr, reinterpret_cast<LPBYTE>(buffer), &bufSize);
		}

		RegCloseKey(hKey);
	}

	// UUID оборудования из SMBIOS Type 1 (System Information)
	if (smbiosSize > 0)
	{
		std::vector<std::byte> smbiosData(smbiosSize);
		if (GetSystemFirmwareTable('RSMB', 0, smbiosData.data(), smbiosSize) == smbiosSize)
		{
			const std::byte* ptr = smbiosData.data() + 8;
			const std::byte* end = smbiosData.data() + smbiosSize;

			while (ptr + 4 <= end)
			{
				zU8 type = static_cast<zU8>(ptr[0]);
				zU8 length = static_cast<zU8>(ptr[1]);
				if (length < 4 || ptr + length > end) break;

				if (type == 1 && length >= 0x18) // Type 1: System Information
				{
					const zU8* u = reinterpret_cast<const zU8*>(ptr + 0x08);
					// Форматирование UUID согласно спецификации SMBIOS (Little-Endian первые 3 группы)
					sysUuid = std::format("{:02x}{:02x}{:02x}{:02x}-{:02x}{:02x}-{:02x}{:02x}-{:02x}{:02x}-{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}",
						u[3], u[2], u[1], u[0], u[5], u[4], u[7], u[6],
						u[8], u[9], u[10], u[11], u[12], u[13], u[14], u[15]);
					break;
				}

				ptr += length;
				while (ptr + 1 < end && !(ptr[0] == std::byte{ 0 } && ptr[1] == std::byte{ 0 }))
				{
					ptr++;
				}
				ptr += 2;
			}
		}
	}

	MotherboardInfo motherboard(mbVendor, mbModel, sysUuid);

	// 6. Накопители (Storages)
	WCHAR sysDir[MAX_PATH]{};
	GetSystemDirectoryW(sysDir, MAX_PATH);
	WCHAR sysDriveLetter = sysDir[0];

	WCHAR driveBuffer[MAX_PATH]{};
	DWORD len = GetLogicalDriveStringsW(MAX_PATH, driveBuffer);
	if (len > 0 && len <= MAX_PATH)
	{
		WCHAR* drive = driveBuffer;
		while (*drive)
		{
			UINT driveType = GetDriveTypeW(drive);
			if (driveType == DRIVE_FIXED || driveType == DRIVE_REMOVABLE)
			{
				ULARGE_INTEGER freeBytes{}, totalBytes{}, totalFreeBytes{};
				if (GetDiskFreeSpaceExW(drive, &freeBytes, &totalBytes, &totalFreeBytes))
				{
						std::string mountPath = WCharToUtf8(drive);

					WCHAR volumeNameBuf[MAX_PATH]{};
					GetVolumeInformationW(drive, volumeNameBuf, MAX_PATH, nullptr, nullptr, nullptr, nullptr, 0);
					std::string volName = WCharToUtf8(volumeNameBuf);
					if (volName.empty())
						volName = mountPath;

					bool isSysDrive = (drive[0] == sysDriveLetter);

					eStorageType type = eStorageType::Unknown;
					if (driveType == DRIVE_REMOVABLE)
					{
						type = eStorageType::Removable;
					}
					else if (driveType == DRIVE_CDROM)
					{
						type = eStorageType::CDROM;
					}
					else if (driveType == DRIVE_REMOTE)
					{
						type = eStorageType::Network;
					}
					else if (driveType == DRIVE_RAMDISK)
					{
						type = eStorageType::RAMDisk;
					}
					else if (driveType == DRIVE_FIXED)
					{
						// Формируем имя физического или логического устройства для IOCTL
						std::wstring volumePath = L"\\\\.\\" + std::wstring(1, drive[0]) + L":";
						HANDLE hDevice = CreateFileW(volumePath.c_str(), 0, FILE_SHARE_READ | FILE_SHARE_WRITE,
							nullptr, OPEN_EXISTING, 0, nullptr);

						if (hDevice != INVALID_HANDLE_VALUE)
						{
							// 1. Проверка Seek Penalty (HDD vs SSD)
							STORAGE_PROPERTY_QUERY query{};
							query.PropertyId = StorageDeviceSeekPenaltyProperty;
							query.QueryType = PropertyStandardQuery;

							DEVICE_SEEK_PENALTY_DESCRIPTOR seekDesc{};
							DWORD bytesReturned = 0;

							if (DeviceIoControl(hDevice, IOCTL_STORAGE_QUERY_PROPERTY, &query, sizeof(query),
								&seekDesc, sizeof(seekDesc), &bytesReturned, nullptr))
							{
								type = seekDesc.IncursSeekPenalty ? eStorageType::HDD : eStorageType::SSD;
							}

							// 2. Проверка BusType (NVMe)
							query.PropertyId = StorageDeviceProperty;
							query.QueryType = PropertyStandardQuery;

							std::vector<std::byte> propBuffer(1024);
							if (DeviceIoControl(hDevice, IOCTL_STORAGE_QUERY_PROPERTY, &query, sizeof(query),
								propBuffer.data(), static_cast<DWORD>(propBuffer.size()), &bytesReturned, nullptr))
							{
								auto* devDesc = reinterpret_cast<STORAGE_DEVICE_DESCRIPTOR*>(propBuffer.data());
								if (devDesc->BusType == BusTypeNvme)
								{
									type = eStorageType::NVMe;
								}
							}

							CloseHandle(hDevice);
						}

						if (type == eStorageType::Unknown)
							type = eStorageType::SSD;
					}

					StorageInfo storage(volName, mountPath, totalBytes.QuadPart, freeBytes.QuadPart, isSysDrive, type);
					storages.push_back(storage);
				}
			}
			drive += wcslen(drive) + 1;
		}
	}

	// 7. Сетевые адаптеры (Network Adapters)
	ULONG outBufLen = 15000;
	std::vector<std::byte> adapterBuffer(outBufLen);
	PIP_ADAPTER_ADDRESSES pAddresses = reinterpret_cast<PIP_ADAPTER_ADDRESSES>(adapterBuffer.data());
	DWORD dwRetVal = GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_INCLUDE_PREFIX, nullptr, pAddresses, &outBufLen);

	if (dwRetVal == ERROR_BUFFER_OVERFLOW)
	{
		adapterBuffer.resize(outBufLen);
		pAddresses = reinterpret_cast<PIP_ADAPTER_ADDRESSES>(adapterBuffer.data());
		dwRetVal = GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_INCLUDE_PREFIX, nullptr, pAddresses, &outBufLen);
	}

	if (dwRetVal == NO_ERROR)
	{
		PIP_ADAPTER_ADDRESSES pCurrAddresses = pAddresses;
		while (pCurrAddresses != nullptr)
		{
			if (pCurrAddresses->IfType != IF_TYPE_SOFTWARE_LOOPBACK)
			{
				std::string adapterName = WCharToUtf8(pCurrAddresses->FriendlyName);

				std::string macAddress;
				if (pCurrAddresses->PhysicalAddressLength > 0)
				{
					for (DWORD i = 0; i < pCurrAddresses->PhysicalAddressLength; ++i)
					{
						if (i > 0) macAddress += "-";
						macAddress += std::format("{:02X}", static_cast<unsigned int>(pCurrAddresses->PhysicalAddress[i]));
					}
				}

				bool isActive = (pCurrAddresses->OperStatus == IfOperStatusUp);

				zU64 transmitSpeedMbps = 0;
				zU64 receiveSpeedMbps = 0;

				if (isActive && pCurrAddresses->TransmitLinkSpeed != 0 && pCurrAddresses->TransmitLinkSpeed != (zU64)-1)
				{
					transmitSpeedMbps = pCurrAddresses->TransmitLinkSpeed / (1000 * 1000);
				}

				if (isActive && pCurrAddresses->ReceiveLinkSpeed != 0 && pCurrAddresses->ReceiveLinkSpeed != (zU64)-1)
				{
					receiveSpeedMbps = pCurrAddresses->ReceiveLinkSpeed / (1000 * 1000);
				}

				NetworkAdapterInfo netAdapter(adapterName, macAddress, transmitSpeedMbps, receiveSpeedMbps, isActive);
				networkAdapters.push_back(netAdapter);
			}
			pCurrAddresses = pCurrAddresses->Next;
		}
	}

	return HardwareState(
		std::move(cpus),
		std::move(ram),
		std::move(motherboard),
		std::move(gpus),
		std::move(monitors),
		std::move(storages),
		std::move(networkAdapters)
	);
}
