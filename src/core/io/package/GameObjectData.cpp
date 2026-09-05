#include "core/io/package/GameObjectData.h"

namespace zzz::core
{
	GameObjectData::GameObjectData(
		Guid guid,
		std::string name,
		std::string layerName,
		eLayerType layerType,
		eObjectDomain domain,
		bool isActive,
		math::Vec3<zF32> position,
		math::Quat<zF32> rotation,
		math::Vec3<zF32> scale,
		Guid meshGuid,
		Guid materialGuid,
		std::vector<Guid> scriptGuids)
		: m_Guid(guid)
		, m_Name(std::move(name))
		, m_LayerName(std::move(layerName))
		, m_LayerType(layerType)
		, m_Domain(domain)
		, m_IsActive(isActive)
		, m_Position(position)
		, m_Rotation(rotation)
		, m_Scale(scale)
		, m_MeshGuid(meshGuid)
		, m_MaterialGuid(materialGuid)
		, m_ScriptGuids(std::move(scriptGuids))
	{
	}

	std::expected<void, std::string> GameObjectData::Serialize(std::vector<std::byte>& buffer, const Serializer& serializer) const
	{
		return serializer.Serialize(buffer, m_Guid)
			.and_then([&]() { return serializer.Serialize(buffer, m_Name); })
			.and_then([&]() { return serializer.Serialize(buffer, m_LayerName); })
			.and_then([&]() { return serializer.Serialize(buffer, static_cast<uint8_t>(m_LayerType)); })
			.and_then([&]() { return serializer.Serialize(buffer, static_cast<uint8_t>(m_Domain)); })
			.and_then([&]() { return serializer.Serialize(buffer, m_IsActive ? uint8_t{ 1 } : uint8_t{ 0 }); })
			.and_then([&]() { return serializer.Serialize(buffer, m_Position); })
			.and_then([&]() { return serializer.Serialize(buffer, m_Rotation); })
			.and_then([&]() { return serializer.Serialize(buffer, m_Scale); })
			.and_then([&]() { return serializer.Serialize(buffer, m_MeshGuid); })
			.and_then([&]() { return serializer.Serialize(buffer, m_MaterialGuid); })
			.and_then([&]() -> std::expected<void, std::string> {
				const uint32_t scriptsCount = static_cast<uint32_t>(m_ScriptGuids.size());
				auto res = serializer.Serialize(buffer, scriptsCount);
				if (!res) return res;

				for (const auto& sGuid : m_ScriptGuids)
				{
					res = serializer.Serialize(buffer, sGuid);
					if (!res) return res;
				}
				return {};
			});
	}

	std::expected<void, std::string> GameObjectData::Deserialize(std::span<const std::byte> buffer, std::size_t& offset, const Serializer& serializer)
	{
		uint8_t layerTypeRaw = 0;
		uint8_t domainRaw = 0;
		uint8_t activeRaw = 1;
		uint32_t scriptsCount = 0;

		auto res = serializer.Deserialize(buffer, offset, m_Guid)
			.and_then([&]() { return serializer.Deserialize(buffer, offset, m_Name); })
			.and_then([&]() { return serializer.Deserialize(buffer, offset, m_LayerName); })
			.and_then([&]() { return serializer.Deserialize(buffer, offset, layerTypeRaw); })
			.and_then([&]() { return serializer.Deserialize(buffer, offset, domainRaw); })
			.and_then([&]() { return serializer.Deserialize(buffer, offset, activeRaw); })
			.and_then([&]() { return serializer.Deserialize(buffer, offset, m_Position); })
			.and_then([&]() { return serializer.Deserialize(buffer, offset, m_Rotation); })
			.and_then([&]() { return serializer.Deserialize(buffer, offset, m_Scale); })
			.and_then([&]() { return serializer.Deserialize(buffer, offset, m_MeshGuid); })
			.and_then([&]() { return serializer.Deserialize(buffer, offset, m_MaterialGuid); })
			.and_then([&]() { return serializer.Deserialize(buffer, offset, scriptsCount); });

		if (!res)
		{
			return res;
		}

		m_LayerType = static_cast<eLayerType>(layerTypeRaw);
		m_Domain = static_cast<eObjectDomain>(domainRaw);
		m_IsActive = (activeRaw != 0);

		m_ScriptGuids.clear();
		m_ScriptGuids.reserve(scriptsCount);
		for (uint32_t i = 0; i < scriptsCount; ++i)
		{
			Guid sGuid;
			res = serializer.Deserialize(buffer, offset, sGuid);
			if (!res)
			{
				return res;
			}
			m_ScriptGuids.push_back(sGuid);
		}

		return {};
	}
}
