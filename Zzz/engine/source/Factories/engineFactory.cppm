
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

export namespace zzz
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

		[[nodiscard]] Result<std::shared_ptr<IGAPI>> CreateGAPI(const std::shared_ptr<GAPIConfig> config);
		[[nodiscard]] inline std::shared_ptr<IGAPI> GetGAPI() const noexcept { return m_GAPI; }

	private:
		std::shared_ptr<IGAPI> m_GAPI;
	};

	Result<std::shared_ptr<IGAPI>> EngineFactory::CreateGAPI(const std::shared_ptr<GAPIConfig> config)
	{
		if (m_GAPI)
			return Unexpected(eResult::already_created, L"GAPI already created.");

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

			auto res = igapi->Initialize()
				.and_then([&]() { m_GAPI = std::move(igapi); })
				.or_else([&](const Unexpected& error) { throw_runtime_error(std::format("{}", wstring_to_string(error.getMessage()))); });

			return m_GAPI;
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