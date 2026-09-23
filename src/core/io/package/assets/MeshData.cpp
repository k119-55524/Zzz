#include "core/io/package/assets/MeshData.h"

#include "core/utils/SafeRange.h"

namespace zzz::core
{
	MeshData::MeshData(
		zU32 vertexCount,
		zU32 vertexStride,
		std::vector<std::byte> vertexData,
		zU32 indexCount,
		eIndexFormat indexFormat,
		std::vector<std::byte> indexData)
		: m_VertexCount(vertexCount)
		, m_VertexStride(vertexStride)
		, m_VertexData(std::move(vertexData))
		, m_IndexCount(indexCount)
		, m_IndexFormat(indexFormat)
		, m_IndexData(std::move(indexData))
	{
	}

	std::expected<void, std::string> MeshData::Serialize(std::vector<std::byte>& buffer, const Serializer& serializer) const
	{
		auto res = serializer.Serialize(buffer, m_VertexCount);
		if (!res) return res;

		res = serializer.Serialize(buffer, m_VertexStride);
		if (!res) return res;

		res = serializer.Serialize(buffer, std::span<const std::byte>(m_VertexData));
		if (!res) return res;

		res = serializer.Serialize(buffer, m_IndexCount);
		if (!res) return res;

		res = serializer.Serialize(buffer, m_IndexFormat);
		if (!res) return res;

		res = serializer.Serialize(buffer, std::span<const std::byte>(m_IndexData));
		if (!res) return res;

		return {};
	}

	std::expected<void, std::string> MeshData::Deserialize(std::span<const std::byte> buffer, std::size_t& offset, const Serializer& serializer)
	{
		auto res = serializer.Deserialize(buffer, offset, m_VertexCount);
		if (!res) return res;

		res = serializer.Deserialize(buffer, offset, m_VertexStride);
		if (!res) return res;

		if (m_VertexCount > 0 && m_VertexStride == 0)
			return std::unexpected("MeshData: vertexStride не может быть равен 0 при ненулевом vertexCount.");

		const auto vertexCount = NarrowTo<std::size_t>(m_VertexCount);
		const auto vertexStride = NarrowTo<std::size_t>(m_VertexStride);
		const auto vertexByteSize = (vertexCount && vertexStride) ? CheckedMul(*vertexCount, *vertexStride) : std::nullopt;
		if (!vertexByteSize)
			return std::unexpected("MeshData: размер вершинных данных переполняет std::size_t.");

		if (!IsRangeInside(offset, *vertexByteSize, buffer.size()))
			return std::unexpected("MeshData: вершинные данные выходят за границы буфера.");

		m_VertexData.resize(*vertexByteSize);
		res = serializer.Deserialize(buffer, offset, std::span<std::byte>(m_VertexData));
		if (!res) return res;

		res = serializer.Deserialize(buffer, offset, m_IndexCount);
		if (!res) return res;

		res = serializer.Deserialize(buffer, offset, m_IndexFormat);
		if (!res) return res;

		std::size_t indexElemSize = 0;
		switch (m_IndexFormat)
		{
		case eIndexFormat::UInt16:
			indexElemSize = sizeof(zU16);
			break;
		case eIndexFormat::UInt32:
			indexElemSize = sizeof(zU32);
			break;
		default:
			return std::unexpected("MeshData: неизвестный формат индексов.");
		}

		if (m_IndexCount > 0 && m_VertexCount == 0)
			return std::unexpected("MeshData: индексы не могут существовать без вершин.");

		const auto indexCount = NarrowTo<std::size_t>(m_IndexCount);
		const auto indexByteSize = indexCount ? CheckedMul(*indexCount, indexElemSize) : std::nullopt;
		if (!indexByteSize)
			return std::unexpected("MeshData: размер индексных данных переполняет std::size_t.");

		if (!IsRangeInside(offset, *indexByteSize, buffer.size()))
			return std::unexpected("MeshData: индексные данные выходят за границы буфера.");

		m_IndexData.resize(*indexByteSize);
		res = serializer.Deserialize(buffer, offset, std::span<std::byte>(m_IndexData));
		if (!res) return res;

		return {};
	}
}
