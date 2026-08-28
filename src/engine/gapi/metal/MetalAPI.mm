#include "engine/gapi/metal/MetalAPI.h"

#if defined(Z_METAL)

namespace zzz::engine
{
	MetalAPI::~MetalAPI()
	{
	}

	void MetalAPI::Initialize(std::shared_ptr<UserSettingsManager> userSettings)
	{
	}

	void MetalAPI::WaitForGpu()
	{
	}
}

#endif // Z_METAL
