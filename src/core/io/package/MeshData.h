#pragma once

#include <vector>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <string>
#include <span>
#include "core/utils/Export.h"
#include "math/utils/Types.h"
#include "core/serialize/Serializer.h"
#include "core/enums/eIndexFormat.h"
#include "core/enums/eResourceType.h"

namespace zzz::core
{
	/**
	 * @class MeshData
	 * @brief Сериализуемый бинарный контейнер геометрии меша в data.dat.
	 */
	class Z_CORE_API MeshData final : public ISerializable
	{
	public:
		static constexpr eResourceType c_ResourceType = eResourceType::Mesh;

		MeshData() = default;
		MeshData(
			zU32 vertexCount,
			zU32 vertexStride,
			std::vector<std::byte> vertexData,
			zU32 indexCount,
			eIndexFormat indexFormat,
			std::vector<std::byte> indexData);

		[[nodiscard]] zU32 GetVertexCount() const noexcept { return m_VertexCount; }
		[[nodiscard]] zU32 GetVertexStride() const noexcept { return m_VertexStride; }
		[[nodiscard]] std::span<const std::byte> GetVertexData() const noexcept { return m_VertexData; }

		[[nodiscard]] zU32 GetIndexCount() const noexcept { return m_IndexCount; }
		[[nodiscard]] eIndexFormat GetIndexFormat() const noexcept { return m_IndexFormat; }
		[[nodiscard]] std::span<const std::byte> GetIndexData() const noexcept { return m_IndexData; }

	protected:
		[[nodiscard]] std::expected<void, std::string> Serialize(std::vector<std::byte>& buffer, const Serializer& serializer) const override;
		[[nodiscard]] std::expected<void, std::string> Deserialize(std::span<const std::byte> buffer, std::size_t& offset, const Serializer& serializer) override;

	private:
		zU32 m_VertexCount{ 0 };
		zU32 m_VertexStride{ 0 };
		std::vector<std::byte> m_VertexData;

		zU32 m_IndexCount{ 0 };
		eIndexFormat m_IndexFormat{ eIndexFormat::UInt16 };
		std::vector<std::byte> m_IndexData;
	};
}
