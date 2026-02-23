
export module EngineFactory;

import IGAPI;
import Ensure;
import Result;
import GAPIConfig;
import StrConvert;

#if defined(ZRENDER_API_D3D12)
import DXAPI;
using namespace zzz::dx;
#elif defined(ZRENDER_API_VULKAN)
import VKAPI;
using namespace zzz::vk;
#else
#error ">>>>> [Compile error]. This branch requires implementation for the current platform"
#endif

using namespace zzz::core;

namespace zzz
{
	export class EngineFactory final
	{
	public:
		EngineFactory() = default;
		EngineFactory(const EngineFactory&) = delete;
		EngineFactory(EngineFactory&&) = delete;
		EngineFactory& operator=(const EngineFactory&) = delete;
		EngineFactory& operator=(EngineFactory&&) = delete;
		~EngineFactory() = default;

		[[nodiscard]] Result<std::shared_ptr<IGAPI>> CreateGAPI(const std::shared_ptr<GAPIConfig>& config);
	};

	Result<std::shared_ptr<IGAPI>> EngineFactory::CreateGAPI(const std::shared_ptr<GAPIConfig>& config)
	{
		static bool isCreate = false;
		ensure(!isCreate, "GAPI can only be created once.");
		isCreate = true;
		ensure(config, "GAPIConfig cannot be null.");

		try
		{
			std::shared_ptr<IGAPI> igapi;
#ifdef ZRENDER_API_D3D12
			igapi = safe_make_shared<DXAPI>(config);
#elif defined(ZRENDER_API_VULKAN)
			igapi = safe_make_shared<VKAPI>(config);
#else
#error ">>>>> [EngineFactory::CreateGAPI]. Compile error. This branch requires implementation for the current platform"
#endif

			if (auto res = igapi->Initialize(); !res)
				throw_runtime_error(std::format("{}", wstring_to_string(res.error().getMessage())));

			return igapi;
		}
		catch (const std::exception& e)
		{
			throw_runtime_error(std::format("Failed to create GAPI: {}.", std::string(e.what())));
		}
		catch (...)
		{
			throw_runtime_error("Unknown exception occurred while creating GAPI.");
		}
	}
}