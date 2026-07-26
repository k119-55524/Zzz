#pragma once

#include "BuilderExport.h"

extern "C"
{
	BUILDER_API const char* GetBuilderEngineVersion();
	BUILDER_API const char* GetGamePackageFileName();
	BUILDER_API bool SerializeProjectManifest(const char* projectJsonPath, const char* outputBinaryPath);
}
