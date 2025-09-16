#include "pch.h"
export module EngineFactory;

import IGAPI;
import result;
import Settings;
import StrConvert;

#if defined(RENDER_API_D3D12)
import DXAPI;
using namespace zzz::platforms::directx;
#else
#error ">>>>> [Compile error]. This branch requires implementation for the current platform"
#endif

using namespace zzz::platforms;

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

		[[nodiscard]] result<std::shared_ptr<IGAPI>> CreateGAPI(std::shared_ptr<Settings> setting);
		[[nodiscard]] inline std::shared_ptr<IGAPI> GetGAPI() const noexcept { return m_GAPI; }

	private:
		std::shared_ptr<IGAPI> m_GAPI;
	};

	result<std::shared_ptr<IGAPI>> EngineFactory::CreateGAPI(std::shared_ptr<Settings> setting)
	{
		if (m_GAPI)
			return Unexpected(eResult::already_created, L">>>>> [EngineFactories::CreateGAPI()]. GAPI already created.");

		ensure(setting, ">>>>> [EngineFactories::CreateGAPI()]. Settings cannot be null.");

		try
		{
			// TODO
			//eGAPIType type = setting.GetGapyType();
			eGAPIType type = eGAPIType::DirectX;
			std::shared_ptr<IGAPI> igapi;
			switch (type)
			{
#if defined(RENDER_API_D3D12)
			case eGAPIType::DirectX:
				igapi = safe_make_shared<DXAPI>();
				break;
#endif // defined(RENDER_API_D3D12)
			default:
				throw_runtime_error(std::format(">>>>> [EngineFactories::CreateGAPI()]. Unsupported GAPI type: {}.", static_cast<uint8_t>(type)));
			}

			auto res = igapi->Initialize()
				.and_then([&]() { m_GAPI = std::move(igapi); })
				.or_else([&](const Unexpected& error) { throw_runtime_error(std::format(">>>>> [EngineFactory::CreateGAPI( ... )]. {}.", wstring_to_string(error.getMessage()))); });

			return m_GAPI;
		}
		catch (const std::exception& e)
		{
			throw_runtime_error(std::format(">>>>> [EngineFactories::CreateGAPI()]. Failed to create GAPI: {}.", std::string(e.what())));
		}
		catch (...)
		{
			throw_runtime_error(">>>>>> [EngineFactories::CreateGAPI()]. Unknown exception occurred while creating GAPI.");
		}
	}
}