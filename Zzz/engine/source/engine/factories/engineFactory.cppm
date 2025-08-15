#include "pch.h"
export module engineFactory;

import IGAPI;

#if defined(_WIN64)
import DXAPI;
using namespace zzz::platforms::directx;
#else
#error ">>>>> [Compile error]. This branch requires implementation for the current platform"
#endif

using namespace zzz::platforms;

export namespace zzz
{
	export class engineFactory final
	{
	public:
		engineFactory() = default;
		engineFactory(const engineFactory&) = delete;
		engineFactory(engineFactory&&) = delete;
		engineFactory& operator=(const engineFactory&) = delete;
		engineFactory& operator=(engineFactory&&) = delete;
		~engineFactory() = default;

		[[nodiscard]] std::shared_ptr<IGAPI> CreateGAPI(eGAPIType type) const;
	};

	std::shared_ptr<IGAPI> engineFactory::CreateGAPI(eGAPIType type) const
	{
		try
		{
			switch (type)
			{
#if defined(_WIN64)
			case eGAPIType::DirectX:
				return safe_make_shared<DXAPI>();
#endif // defined(_WIN64)
			default:
				throw_runtime_error(std::format(">>>>> [engineFactories::CreateGAPI()]. Unsupported GAPI type: {}.", static_cast<uint8_t>(type)));
			}
		}
		catch (const std::exception& e)
		{
			throw_runtime_error(std::format(">>>>> [engineFactories::CreateGAPI()]. Failed to create GAPI: {}.", std::string(e.what())));
		}
		catch (...)
		{
			throw_runtime_error(">>>>>> [engineFactories::CreateGAPI()]. Unknown exception occurred while creating GAPI.");
		}
	}
}