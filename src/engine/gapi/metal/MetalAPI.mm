#include "engine/gapi/metal/MetalAPI.h"

#if defined(Z_METAL)

namespace zzz::engine
{
	MetalAPI::~MetalAPI()
	{
	}

	void MetalAPI::Initialize(std::shared_ptr<UserSettingsManager> userSettings)
	{
		Z_CHECK_ONCE_CALL();
	}

	void MetalAPI::WaitForGpu()
	{
	}
}

#endif // Z_METAL
