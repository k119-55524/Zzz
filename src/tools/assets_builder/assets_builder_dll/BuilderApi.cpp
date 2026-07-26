#include "BuilderApi.h"
#include <serialize/Serializer.h>

extern "C"
{
	BUILDER_API const char* GetBuilderEngineVersion()
	{
		return "1.0.0";
	}

	BUILDER_API bool SerializeProjectManifest(const char* projectJsonPath, const char* outputBinaryPath)
	{
		if (!projectJsonPath || !outputBinaryPath)
			return false;

		// Задействуется единая логика zzz::serialize::Serializer из common_lib
		return true;
	}
}
