#include "core/io/package/assets/GameObjectData.h"

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
		std::vector<RenderPairData> renderPairs,
		std::vector<Guid> scriptGuids,
		uint32_t parentIndex)
		: m_Guid(guid)
		, m_ParentIndex(parentIndex)
		, m_Name(std::move(name))
		, m_IsEntity(isEntity)
		, m_IsActive(isActive)
		, m_Position(position)
		, m_Rotation(rotation)
		, m_Scale(scale)
		, m_RenderPairs(std::move(renderPairs))
		, m_ScriptGuids(std::move(scriptGuids))
	{
	}

	std::expected<void, std::string> GameObjectData::Serialize(std::vector<std::byte>& buffer, const Serializer& serializer) const
	{
		const Guid legacyMeshGuid = m_RenderPairs.empty() ? Guid{} : m_RenderPairs[0].meshGuid;
		const Guid legacyMaterialGuid = m_RenderPairs.empty() ? Guid{} : m_RenderPairs[0].materialGuid;

		return serializer.Serialize(buffer, m_Guid)
			.and_then([&]() { return serializer.Serialize(buffer, m_ParentIndex); })
			.and_then([&]() { return serializer.Serialize(buffer, m_Name); })
			.and_then([&]() { return serializer.Serialize(buffer, m_IsEntity ? uint8_t{ 1 } : uint8_t{ 0 }); })
			.and_then([&]() { return serializer.Serialize(buffer, m_IsActive ? uint8_t{ 1 } : uint8_t{ 0 }); })
			.and_then([&]() { return serializer.Serialize(buffer, m_Position); })
			.and_then([&]() { return serializer.Serialize(buffer, m_Rotation); })
			.and_then([&]() { return serializer.Serialize(buffer, m_Scale); })
			.and_then([&]() { return serializer.Serialize(buffer, legacyMeshGuid); })
			.and_then([&]() { return serializer.Serialize(buffer, legacyMaterialGuid); })
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
				const uint32_t pairsCount = static_cast<uint32_t>(m_RenderPairs.size());
				auto res = serializer.Serialize(buffer, pairsCount);
				if (!res) return res;

				for (const auto& pair : m_RenderPairs)
				{
					res = serializer.Serialize(buffer, pair.meshGuid);
					if (!res) return res;

					res = serializer.Serialize(buffer, pair.materialGuid);
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
		Guid legacyMaterialGuid{};

		auto res = serializer.Deserialize(buffer, offset, m_Guid)
			.and_then([&]() { return serializer.Deserialize(buffer, offset, m_ParentIndex); })
			.and_then([&]() { return serializer.Deserialize(buffer, offset, m_Name); })
			.and_then([&]() { return serializer.Deserialize(buffer, offset, entityRaw); })
			.and_then([&]() { return serializer.Deserialize(buffer, offset, activeRaw); })
			.and_then([&]() { return serializer.Deserialize(buffer, offset, m_Position); })
			.and_then([&]() { return serializer.Deserialize(buffer, offset, m_Rotation); })
			.and_then([&]() { return serializer.Deserialize(buffer, offset, m_Scale); })
			.and_then([&]() { return serializer.Deserialize(buffer, offset, legacyMeshGuid); })
			.and_then([&]() { return serializer.Deserialize(buffer, offset, legacyMaterialGuid); })
			.and_then([&]() { return serializer.Deserialize(buffer, offset, scriptsCount); });

		if (!res)
		{
			return res;
		}

		m_IsEntity = (entityRaw != 0);
		m_IsActive = (activeRaw != 0);

		res = Serializer::ValidateElementCount(buffer, offset, scriptsCount, Guid::BinarySize());
		if (!res)
		{
			return res;
		}

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

		m_RenderPairs.clear();
		// Обратная совместимость: если буфер исчерпан, читаем из legacy-полей
		if (offset >= buffer.size())
		{
			if (legacyMeshGuid.IsValid())
			{
				m_RenderPairs.push_back(RenderPairData{ legacyMeshGuid, legacyMaterialGuid });
			}
			return {};
		}

		uint32_t pairsCount = 0;
		res = serializer.Deserialize(buffer, offset, pairsCount);
		if (!res) return res;

		res = Serializer::ValidateElementCount(buffer, offset, pairsCount, Guid::BinarySize() * 2);
		if (!res) return res;

		m_RenderPairs.reserve(pairsCount);
		for (uint32_t i = 0; i < pairsCount; ++i)
		{
			Guid smGuid;
			res = serializer.Deserialize(buffer, offset, smGuid);
			if (!res) return res;

			Guid matGuid;
			res = serializer.Deserialize(buffer, offset, matGuid);
			if (!res) return res;

			m_RenderPairs.push_back(RenderPairData{ smGuid, matGuid });
		}

		if (m_RenderPairs.empty() && legacyMeshGuid.IsValid())
		{
			m_RenderPairs.push_back(RenderPairData{ legacyMeshGuid, legacyMaterialGuid });
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
		if (!m_RenderPairs.empty())
		{
			DOut(Assets, "{}renderPairs({}):", nestedIndentation, m_RenderPairs.size());
			for (size_t i = 0; i < m_RenderPairs.size(); ++i)
			{
				DOut(Assets, "{}  pair [{}/{}]: mesh: {}, material: {}", nestedIndentation,
					i + 1, m_RenderPairs.size(),
					m_RenderPairs[i].meshGuid.ToString(),
					m_RenderPairs[i].materialGuid.ToString());
			}
		}
		if (!m_ScriptGuids.empty())
		{
			DOut(Assets, "{}scripts({}):", nestedIndentation, m_ScriptGuids.size());
			for (size_t i = 0; i < m_ScriptGuids.size(); ++i)
			{
				DOut(Assets, "{}  script [{}/{}]: {}", nestedIndentation, i + 1, m_ScriptGuids.size(), m_ScriptGuids[i].ToString());
			}
		}
#endif // Z_ADD_LOGGER
	}
}

