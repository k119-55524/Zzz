#include "StorageInfoCollectorMSWin.h"

#if defined(Z_WINDOWS)

#include "engine/platforms/hardware/platforms/common/WCharUtilsMSWin.h"

using namespace zzz::engine;
using namespace zzz::core;

std::vector<StorageInfo> StorageInfoCollectorMSWin::Collect() const
{
	std::vector<StorageInfo> storages;

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
					std::string mountPath = WCharToUtf8MSWin(drive);

					WCHAR volumeNameBuf[MAX_PATH]{};
					GetVolumeInformationW(drive, volumeNameBuf, MAX_PATH, nullptr, nullptr, nullptr, nullptr, 0);
					std::string volName = WCharToUtf8MSWin(volumeNameBuf);
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

					storages.emplace_back(volName, mountPath, static_cast<zU64>(totalBytes.QuadPart), static_cast<zU64>(freeBytes.QuadPart), isSysDrive, type);
				}
			}
			drive += wcslen(drive) + 1;
		}
	}

	return storages;
}

#endif // defined(Z_WINDOWS)
