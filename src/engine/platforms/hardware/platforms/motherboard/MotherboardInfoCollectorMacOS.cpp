#include "MotherboardInfoCollectorMacOS.h"

#if defined(Z_MACOS)

#include <IOKit/IOKitLib.h>
#include <CoreFoundation/CoreFoundation.h>

using namespace zzz::engine;
using namespace zzz::core;

MotherboardInfo MotherboardInfoCollectorMacOS::Collect() const
{
	std::string vendor = "Apple Inc.";
	std::string model = "Unknown";
	std::string uuid;

	io_service_t service = IOServiceGetMatchingService(kIOMasterPortDefault, IOServiceMatching("IOPlatformExpertDevice"));
	if (service)
	{
		if (CFTypeRef modelRef = IORegistryEntryCreateCFProperty(service, CFSTR("model"), kCFAllocatorDefault, 0))
		{
			if (CFGetTypeID(modelRef) == CFDataGetTypeID())
			{
				auto* data = static_cast<CFDataRef>(modelRef);
				CFIndex length = CFDataGetLength(data);
				if (length > 0)
				{
					model = std::string(reinterpret_cast<const char*>(CFDataGetBytePtr(data)), static_cast<std::size_t>(length));
					while (!model.empty() && model.back() == '\0')
						model.pop_back();
				}
			}
			CFRelease(modelRef);
		}

		if (CFTypeRef uuidRef = IORegistryEntryCreateCFProperty(service, CFSTR(kIOPlatformUUIDKey), kCFAllocatorDefault, 0))
		{
			if (CFGetTypeID(uuidRef) == CFStringGetTypeID())
			{
				char buffer[64] = {};
				if (CFStringGetCString(static_cast<CFStringRef>(uuidRef), buffer, sizeof(buffer), kCFStringEncodingUTF8))
					uuid = buffer;
			}
			CFRelease(uuidRef);
		}

		IOObjectRelease(service);
	}

	return MotherboardInfo(vendor, model, uuid);
}

#endif // defined(Z_MACOS)
