#include "core/io/package/LayerData.h"

namespace zzz::core
{
	std::expected<void, std::string> LayerData::Serialize(std::vector<std::byte>& buffer, const Serializer& serializer) const
	{
		return serializer.Serialize(buffer, m_Name)
			.and_then([&]() { return serializer.Serialize(buffer, static_cast<uint8_t>(m_Type)); })
			.and_then([&]() -> std::expected<void, std::string> {
				const uint32_t objectsCount = static_cast<uint32_t>(m_Objects.size());
				auto res = serializer.Serialize(buffer, objectsCount);
				if (!res) return res;

				for (const auto& objData : m_Objects)
				{
					res = serializer.Serialize(buffer, objData);
					if (!res) return res;
				}
				return {};
			});
	}

	std::expected<void, std::string> LayerData::Deserialize(std::span<const std::byte> buffer, std::size_t& offset, const Serializer& serializer)
	{
		uint8_t typeRaw = 0;
		uint32_t objectsCount = 0;

		auto res = serializer.Deserialize(buffer, offset, m_Name)
			.and_then([&]() { return serializer.Deserialize(buffer, offset, typeRaw); })
			.and_then([&]() { return serializer.Deserialize(buffer, offset, objectsCount); });

		if (!res)
		{
			return res;
		}

		m_Type = static_cast<eLayerType>(typeRaw);

		m_Objects.clear();
		m_Objects.reserve(objectsCount);
		for (uint32_t i = 0; i < objectsCount; ++i)
		{
			GameObjectData objData{};
			res = serializer.Deserialize(buffer, offset, objData);
			if (!res)
			{
				return res;
			}
			m_Objects.push_back(std::move(objData));
		}

		return {};
	}
}
