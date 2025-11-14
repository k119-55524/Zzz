
#include "pch.h"

export module AppWinConfig_MSin;

import Result;
import Serializer;

using namespace zzz::core;

export namespace zzz
{
	export class AppWinConfig_MSin final : public ISerializable
	{
	public:
		AppWinConfig_MSin() = default;
		virtual ~AppWinConfig_MSin() = default;

	private:



		int i1;
		int i2;

		Result<> Serialize(std::vector<std::byte>& buffer, const zzz::core::Serializer& s) const override
		{
			Result<> res = s.Serialize(buffer, i1)
				.and_then([&]() { return s.Serialize(buffer, i2); });

			return res;
		}

		Result<> DeSerialize(std::span<const std::byte> buffer, std::size_t& offset, const zzz::core::Serializer& s) override
		{
			Result<> res = s.DeSerialize(buffer, offset, i1)
				.and_then([&]() { return s.DeSerialize(buffer, offset, i2); });

			return res;
		}
	};
}