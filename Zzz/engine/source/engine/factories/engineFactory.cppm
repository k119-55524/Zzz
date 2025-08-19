#include "pch.h"
export module engineFactory;

import IGAPI;
import result;
import settings;
import strConvert;

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

		[[nodiscard]] result<std::shared_ptr<IGAPI>> CreateGAPI(std::shared_ptr<settings> setting);
		[[nodiscard]] inline std::shared_ptr<IGAPI> GetGAPI() const noexcept { return m_GAPI; }

	private:
		std::shared_ptr<IGAPI> m_GAPI;
	};

	result<std::shared_ptr<IGAPI>> engineFactory::CreateGAPI(std::shared_ptr<settings> setting)
	{
		if (m_GAPI)
			return Unexpected(eResult::already_created, L">>>>> [engineFactories::CreateGAPI()]. GAPI already created.");

		ensure(setting, ">>>>> [engineFactories::CreateGAPI()]. Settings cannot be null.");

		try
		{
			// TODO
			//eGAPIType type = setting.GetGapyType();
			eGAPIType type = eGAPIType::DirectX;
			std::shared_ptr<IGAPI> igapi;
			switch (type)
			{
#if defined(_WIN64)
			case eGAPIType::DirectX:
				igapi = safe_make_shared<DXAPI>();
				break;
#endif // defined(_WIN64)
			default:
				throw_runtime_error(std::format(">>>>> [engineFactories::CreateGAPI()]. Unsupported GAPI type: {}.", static_cast<uint8_t>(type)));
			}

			auto res = igapi->Initialize()
				.and_then([&]() { m_GAPI = std::move(igapi); })
				.or_else([&](const Unexpected& error) { throw_runtime_error(std::format(">>>>> [engineFactory::CreateGAPI( ... )]. {}.", wstring_to_string(error.getMessage()))); });

			return m_GAPI;
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