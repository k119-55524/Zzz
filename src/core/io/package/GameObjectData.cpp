#include "core/io/package/GameObjectData.h"

namespace zzz::core
{
	GameObjectData::GameObjectData(
		Guid guid,
		std::string name,
		bool isEntity,
		bool isActive,
		math::Vec3<zF32> position,
		math::Quat<zF32> rotation,
		math::Vec3<zF32> scale,
		Guid meshGuid,
		Guid materialGuid,
		std::vector<Guid> scriptGuids,
		uint32_t parentIndex,
		std::vector<Guid> submeshGuids,
		std::vector<Guid> materialGuids)
		: m_Guid(guid)
		, m_ParentIndex(parentIndex)
		, m_Name(std::move(name))
		, m_IsEntity(isEntity)
		, m_IsActive(isActive)
		, m_Position(position)
		, m_Rotation(rotation)
		, m_Scale(scale)
		, m_MeshGuid(meshGuid)
		, m_MaterialGuid(materialGuid)
		, m_ScriptGuids(std::move(scriptGuids))
		, m_SubmeshGuids(std::move(submeshGuids))
		, m_MaterialGuids(std::move(materialGuids))
	{
	}

	std::expected<void, std::string> GameObjectData::Serialize(std::vector<std::byte>& buffer, const Serializer& serializer) const
	{
		return serializer.Serialize(buffer, m_Guid)
			.and_then([&]() { return serializer.Serialize(buffer, m_ParentIndex); })
			.and_then([&]() { return serializer.Serialize(buffer, m_Name); })
			.and_then([&]() { return serializer.Serialize(buffer, m_IsEntity ? uint8_t{ 1 } : uint8_t{ 0 }); })
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
			})
			.and_then([&]() -> std::expected<void, std::string> {
				const uint32_t submeshesCount = static_cast<uint32_t>(m_SubmeshGuids.size());
				auto res = serializer.Serialize(buffer, submeshesCount);
				if (!res) return res;

				for (const auto& smGuid : m_SubmeshGuids)
				{
					res = serializer.Serialize(buffer, smGuid);
					if (!res) return res;
				}

				const uint32_t materialsCount = static_cast<uint32_t>(m_MaterialGuids.size());
				res = serializer.Serialize(buffer, materialsCount);
				if (!res) return res;

				for (const auto& matGuid : m_MaterialGuids)
				{
					res = serializer.Serialize(buffer, matGuid);
					if (!res) return res;
				}

				return {};
			});
	}

	std::expected<void, std::string> GameObjectData::Deserialize(std::span<const std::byte> buffer, std::size_t& offset, const Serializer& serializer)
	{
		uint8_t entityRaw = 0;
		uint8_t activeRaw = 1;
		uint32_t scriptsCount = 0;

		auto res = serializer.Deserialize(buffer, offset, m_Guid)
			.and_then([&]() { return serializer.Deserialize(buffer, offset, m_ParentIndex); })
			.and_then([&]() { return serializer.Deserialize(buffer, offset, m_Name); })
			.and_then([&]() { return serializer.Deserialize(buffer, offset, entityRaw); })
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

		m_IsEntity = (entityRaw != 0);
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

		// Обратная совместимость (Правило 17 / 31): если буфер исчерпан (старый формат), мультимеш пустой
		m_SubmeshGuids.clear();
		m_MaterialGuids.clear();
		if (offset >= buffer.size())
		{
			return {};
		}

		uint32_t submeshesCount = 0;
		res = serializer.Deserialize(buffer, offset, submeshesCount);
		if (!res) return res;

		m_SubmeshGuids.reserve(submeshesCount);
		for (uint32_t i = 0; i < submeshesCount; ++i)
		{
			Guid smGuid;
			res = serializer.Deserialize(buffer, offset, smGuid);
			if (!res) return res;
			m_SubmeshGuids.push_back(smGuid);
		}

		uint32_t materialsCount = 0;
		res = serializer.Deserialize(buffer, offset, materialsCount);
		if (!res) return res;

		m_MaterialGuids.reserve(materialsCount);
		for (uint32_t i = 0; i < materialsCount; ++i)
		{
			Guid matGuid;
			res = serializer.Deserialize(buffer, offset, matGuid);
			if (!res) return res;
			m_MaterialGuids.push_back(matGuid);
		}

		return {};
	}
}

