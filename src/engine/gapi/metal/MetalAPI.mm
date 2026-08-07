#include "engine/gapi/metal/MetalAPI.h"

#if defined(Z_METAL)

namespace zzz::engine
{
	MetalAPI::MetalAPI(std::shared_ptr<UserSettingsManager> userSettings)
		: IGAPI(std::move(userSettings))
	{
	}

	MetalAPI::~MetalAPI()
	{
	}

	void MetalAPI::Initialize()
	{
	}

	void MetalAPI::SubmitCommandLists()
	{
	}

	void MetalAPI::BeginRender()
	{
	}

	void MetalAPI::EndRender()
	{
	}

	void MetalAPI::WaitForGpu()
	{
	}
}

#endif // Z_METAL
