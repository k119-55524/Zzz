#pragma once

#include <string>
#include <filesystem>
#include <core/Core.h>

namespace zzz::builder
{
	class PackagePacker final
	{
	public:
		// Упаковка манифеста project.json, сцен (*.zscene) и вьюх (*.zview) в destinationDir/assets/package.dat
		// и данных (мешей/материалов/шейдеров) в destinationDir/assets/data/data.dat. Оба архива получают
		// одно и то же время упаковки (см. DatFileHeader::GetBuildTime()); buildTimestamp - если задан (>0),
		// используется как единый timestamp (мс от unix epoch), иначе генерируется текущее время.
		// outBuildTimestamp - опциональный (может быть nullptr) выходной параметр для передачи того же значения вызывающей стороне.
		static bool PackProject(
			const std::filesystem::path& sourceDir,
			const std::filesystem::path& destinationDir,
			zzz::core::eTargetPlatform targetPlatform,
			const std::string& platformConfigFile = "",
			uint64_t buildTimestamp = 0,
			uint64_t* outBuildTimestamp = nullptr);
	};
}
