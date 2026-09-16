#include "core/io/package/GameObjectData.h"

using namespace zzz::core;
using namespace zzz::math;

namespace zzz::core
{
	GameObjectData::GameObjectData(
		Guid guid,
		std::string name,
		bool isEntity,
		bool isActive,
		Vec3<zF32> position,
		Quat<zF32> rotation,
		Vec3<zF32> scale,
		std::vector<Guid> meshGuids,
		Guid materialGuid,
		std::vector<Guid> scriptGuids,
		uint32_t parentIndex,
		std::vector<Guid> materialGuids)
		: m_Guid(guid)
		, m_ParentIndex(parentIndex)
		, m_Name(std::move(name))
		, m_IsEntity(isEntity)
		, m_IsActive(isActive)
		, m_Position(position)
		, m_Rotation(rotation)
		, m_Scale(scale)
		, m_MeshGuids(std::move(meshGuids))
		, m_MaterialGuid(materialGuid)
		, m_ScriptGuids(std::move(scriptGuids))
		, m_MaterialGuids(std::move(materialGuids))
	{
	}

	std::expected<void, std::string> GameObjectData::Serialize(std::vector<std::byte>& buffer, const Serializer& serializer) const
	{
		const Guid legacyMeshGuid = m_MeshGuids.empty() ? Guid{} : m_MeshGuids[0];
		return serializer.Serialize(buffer, m_Guid)
			.and_then([&]() { return serializer.Serialize(buffer, m_ParentIndex); })
			.and_then([&]() { return serializer.Serialize(buffer, m_Name); })
			.and_then([&]() { return serializer.Serialize(buffer, m_IsEntity ? uint8_t{ 1 } : uint8_t{ 0 }); })
			.and_then([&]() { return serializer.Serialize(buffer, m_IsActive ? uint8_t{ 1 } : uint8_t{ 0 }); })
			.and_then([&]() { return serializer.Serialize(buffer, m_Position); })
			.and_then([&]() { return serializer.Serialize(buffer, m_Rotation); })
			.and_then([&]() { return serializer.Serialize(buffer, m_Scale); })
			.and_then([&]() { return serializer.Serialize(buffer, legacyMeshGuid); })
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
				const uint32_t submeshesCount = static_cast<uint32_t>(m_MeshGuids.size());
				auto res = serializer.Serialize(buffer, submeshesCount);
				if (!res) return res;

				for (const auto& smGuid : m_MeshGuids)
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
		Guid legacyMeshGuid{};

		auto res = serializer.Deserialize(buffer, offset, m_Guid)
			.and_then([&]() { return serializer.Deserialize(buffer, offset, m_ParentIndex); })
			.and_then([&]() { return serializer.Deserialize(buffer, offset, m_Name); })
			.and_then([&]() { return serializer.Deserialize(buffer, offset, entityRaw); })
			.and_then([&]() { return serializer.Deserialize(buffer, offset, activeRaw); })
			.and_then([&]() { return serializer.Deserialize(buffer, offset, m_Position); })
			.and_then([&]() { return serializer.Deserialize(buffer, offset, m_Rotation); })
			.and_then([&]() { return serializer.Deserialize(buffer, offset, m_Scale); })
			.and_then([&]() { return serializer.Deserialize(buffer, offset, legacyMeshGuid); })
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
		m_MeshGuids.clear();
		m_MaterialGuids.clear();
		if (offset >= buffer.size())
		{
			if (legacyMeshGuid.IsValid())
			{
				m_MeshGuids.push_back(legacyMeshGuid);
			}
			return {};
		}

		uint32_t submeshesCount = 0;
		res = serializer.Deserialize(buffer, offset, submeshesCount);
		if (!res) return res;

		m_MeshGuids.reserve(submeshesCount);
		for (uint32_t i = 0; i < submeshesCount; ++i)
		{
			Guid smGuid;
			res = serializer.Deserialize(buffer, offset, smGuid);
			if (!res) return res;
			m_MeshGuids.push_back(smGuid);
		}

		if (m_MeshGuids.empty() && legacyMeshGuid.IsValid())
		{
			m_MeshGuids.push_back(legacyMeshGuid);
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

	void GameObjectData::LogFileBlock([[maybe_unused]] std::string_view indentation) const
	{
#if Z_ADD_LOGGER
		const std::string nestedIndentation = std::string(indentation) + "  ";
		DOut(Assets, "{}[GameObjectData] '{}', guid: {}, isEntity: {}, isActive: {}, parentIndex: {}",
			indentation, m_Name, m_Guid.ToString(), m_IsEntity, m_IsActive, m_ParentIndex);
		DOut(Assets, "{}pos: ({:.2f}, {:.2f}, {:.2f}), rot: ({:.2f}, {:.2f}, {:.2f}, {:.2f}), scale: ({:.2f}, {:.2f}, {:.2f})",
			nestedIndentation, m_Position.x, m_Position.y, m_Position.z,
			m_Rotation.x, m_Rotation.y, m_Rotation.z, m_Rotation.w,
			m_Scale.x, m_Scale.y, m_Scale.z);
		if (HasMesh())
		{
			DOut(Assets, "{}meshGuids({}):", nestedIndentation, m_MeshGuids.size());
			for (size_t i = 0; i < m_MeshGuids.size(); ++i)
			{
				DOut(Assets, "{}  meshGuid #{}: {}", nestedIndentation, i, m_MeshGuids[i].ToString());
			}
		}
		if (HasMaterial())
		{
			DOut(Assets, "{}materialGuid: {}, materials({}):", nestedIndentation, m_MaterialGuid.ToString(), m_MaterialGuids.size());
			for (size_t i = 0; i < m_MaterialGuids.size(); ++i)
			{
				DOut(Assets, "{}  materialGuid #{}: {}", nestedIndentation, i, m_MaterialGuids[i].ToString());
			}
		}
		if (!m_ScriptGuids.empty())
		{
			DOut(Assets, "{}scripts({}):", nestedIndentation, m_ScriptGuids.size());
			for (size_t i = 0; i < m_ScriptGuids.size(); ++i)
			{
				DOut(Assets, "{}  scriptGuid #{}: {}", nestedIndentation, i, m_ScriptGuids[i].ToString());
			}
		}
#endif // Z_ADD_LOGGER
	}
}

