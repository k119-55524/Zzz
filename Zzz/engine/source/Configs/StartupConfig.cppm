
#include "pch.h"

export module StartupConfig;

import Serializer;
import AppWinConfig_MSin;

using namespace zzz::core;

namespace zzz::core
{
	export class StartupConfig final : public ISerializable
	{
	public:

	protected:
#if defined(ZPLATFORM_MSWINDOWS)
		AppWinConfig_MSin m_AppWinConfig;
#elif defined(ZPLATFORM_LINUX)
#elif defined(ZPLATFORM_ANDROID)
#elif defined(ZPLATFORM_MACOS)
#elif defined(ZPLATFORM_IOS)
#else
#error ">>>>> [Compile error]. This branch requires implementation for the current platform"
#endif



		Result<> Serialize(std::vector<std::byte>& buffer, const zzz::core::Serializer& s) const override
		{
			Result<> res = s.Serialize(buffer, m_AppWinConfig);
				//.and_then([&]() { return s.Serialize(buffer, i2); });

			return res;
		}
		Result<> DeSerialize(std::span<const std::byte> buffer, std::size_t& offset, const zzz::core::Serializer& s) override
		{
			Result<> res = s.DeSerialize(buffer, offset, m_AppWinConfig);
				//.and_then([&]() { return s.DeSerialize(buffer, offset, i2); });

			return res;
		}
	};
}
