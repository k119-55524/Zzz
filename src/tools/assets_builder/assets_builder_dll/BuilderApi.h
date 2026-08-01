#pragma once

#include "BuilderExport.h"
#include <cstdint>

extern "C"
{
	BUILDER_API const char* GetBuilderEngineVersion();
	BUILDER_API const char* GetGamePackageFileName();
	BUILDER_API const uint8_t* GetGamePackageMagicBytes();
	BUILDER_API uint32_t GetGamePackageMajorVersion();
	BUILDER_API uint32_t GetGamePackageMinorVersion();
	BUILDER_API uint32_t GetGamePackagePatchVersion();

	BUILDER_API uint32_t GetAssetTypeProjectManifest();
	BUILDER_API uint32_t GetAssetTypeScene();
	BUILDER_API uint32_t GetAssetTypeView();
	BUILDER_API uint32_t GetAssetTypePrefab();
	BUILDER_API uint32_t GetAssetTypeBinaryAsset();
}
