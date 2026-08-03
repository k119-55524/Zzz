#pragma once

#include <string>
#include <filesystem>
#include <core/Types.h>
#include <core/Enums/eTargetPlatform.h>

namespace zzz::builder
{
	class PackagePacker final
	{
	public:
		// Упаковка манифеста project.json, сцен (*.zs) и вьюх (*.zv) в destinationDir/assets/package.dat
		static bool PackProject(
			const std::filesystem::path& sourceDir,
			const std::filesystem::path& destinationDir,
			zzz::common::eTargetPlatform targetPlatform);
	};
}
