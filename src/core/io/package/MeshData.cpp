#include "core/io/package/MeshData.h"

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

		const std::size_t vertexByteSize = static_cast<std::size_t>(m_VertexCount) * m_VertexStride;
		m_VertexData.resize(vertexByteSize);
		res = serializer.Deserialize(buffer, offset, std::span<std::byte>(m_VertexData));
		if (!res) return res;

		res = serializer.Deserialize(buffer, offset, m_IndexCount);
		if (!res) return res;

		res = serializer.Deserialize(buffer, offset, m_IndexFormat);
		if (!res) return res;

		const std::size_t indexElemSize = (m_IndexFormat == eIndexFormat::UInt16) ? 2 : 4;
		const std::size_t indexByteSize = static_cast<std::size_t>(m_IndexCount) * indexElemSize;
		m_IndexData.resize(indexByteSize);
		res = serializer.Deserialize(buffer, offset, std::span<std::byte>(m_IndexData));
		if (!res) return res;

		return {};
	}
}
