#include "MotherboardInfoCollectorLinux.h"

#if defined(Z_LINUX)

#include <fstream>

using namespace zzz::engine;
using namespace zzz::core;

namespace
{
	std::string ReadSysfsLine(const char* path)
	{
		std::ifstream file(path);
		if (!file.is_open())
			return {};

		std::string line;
		std::getline(file, line);
		while (!line.empty() && (line.back() == '\n' || line.back() == '\r' || line.back() == ' '))
			line.pop_back();

		return line;
	}
}

MotherboardInfo MotherboardInfoCollectorLinux::Collect() const
{
	// /sys/class/dmi/id/product_uuid обычно требует прав root - при недоступности возвращается пустая строка (честно).
	std::string vendor = ReadSysfsLine("/sys/class/dmi/id/board_vendor");
	std::string model = ReadSysfsLine("/sys/class/dmi/id/board_name");
	std::string uuid = ReadSysfsLine("/sys/class/dmi/id/product_uuid");

	if (vendor.empty()) vendor = "Unknown";
	if (model.empty()) model = "Unknown";

	return MotherboardInfo(vendor, model, uuid);
}

#endif // defined(Z_LINUX)
