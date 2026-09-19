#pragma once

#include <string>
#include <vector>
#include <span>
#include <cstddef>
#include <cstdint>
#include "core/utils/Guid.h"
#include "core/serialize/Serializer.h"
#include "core/enums/eLayerType.h"
#include "core/io/package/GameObjectData.h"

namespace zzz::core
{

	/**
	 * @class LayerData
	 * @brief Сериализуемое представление слоя сцены: GUID, имя, тип и принадлежащие ему объекты.
	 *
	 * @details Объекты одного слоя хранятся внутри самого слоя, а не в плоском списке на уровне
	 * SceneData - это гарантирует, что при чтении сцены каждый ILayer создаётся ровно один раз
	 * и сразу наполняется своими объектами, без группировки по имени/типу на стороне движка.
	 */
	class LayerData final : public ISerializable
	{
	public:
		LayerData()
			: m_Guid()
			, m_Name()
			, m_Type(eLayerType::Layer3D)
			, m_Objects()
		{
		}

		LayerData(Guid guid, std::string name, eLayerType type, std::vector<GameObjectData> objects = {})
			: m_Guid(guid)
			, m_Name(std::move(name))
			, m_Type(type)
			, m_Objects(std::move(objects))
		{
		}

		[[nodiscard]] const Guid& GetGuid() const noexcept { return m_Guid; }
		[[nodiscard]] const std::string& GetName() const noexcept { return m_Name; }
		[[nodiscard]] eLayerType GetType() const noexcept { return m_Type; }

		[[nodiscard]] std::span<const GameObjectData> GetObjects() const noexcept { return m_Objects; }
		[[nodiscard]] std::vector<GameObjectData>& GetObjects() noexcept { return m_Objects; }
		void SetObjects(std::vector<GameObjectData> objects) noexcept { m_Objects = std::move(objects); }

		void LogFileBlock([[maybe_unused]] std::string_view indentation = {}) const;

	protected:
		[[nodiscard]] std::expected<void, std::string> Serialize(std::vector<std::byte>& buffer, const Serializer& serializer) const override;
		[[nodiscard]] std::expected<void, std::string> Deserialize(std::span<const std::byte> buffer, std::size_t& offset, const Serializer& serializer) override;

	private:
		Guid m_Guid;
		std::string m_Name;
		eLayerType m_Type;
		std::vector<GameObjectData> m_Objects;
	};
}
