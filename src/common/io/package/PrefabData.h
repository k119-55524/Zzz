#pragma once

#include <string>
#include <common/serialize/Serializer.h>

using namespace zzz::common;

namespace zzz::core
{
	class PrefabData final : public ISerializable
	{
	public:
		PrefabData() = default;
		explicit PrefabData(std::string name)
			: name(std::move(name))
		{}

		std::string name;

		void LogFileBlock() const
		{
#if Z_ADD_LOGGER || Z_DEVELOPMENT_BUILD
			DOut("           [PrefabData] Префаб: '{}'", name);
#endif
		}

	protected:
		[[nodiscard]] std::expected<void, std::string> Serialize(std::vector<std::byte>& buffer, const Serializer& serializer) const override
		{
			return serializer.Serialize(buffer, name);
		}

		[[nodiscard]] std::expected<void, std::string> Deserialize(std::span<const std::byte> buffer, std::size_t& offset, const Serializer& serializer) override
		{
			return serializer.Deserialize(buffer, offset, name);
		}
	};
}
