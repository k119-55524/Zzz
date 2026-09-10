#pragma once

#include <string>
#include <vector>
#include <cstddef>
#include "core/resources/ResourceBase.h"
#include "core/io/package/MeshData.h"
#include "core/enums/eIndexFormat.h"

namespace zzz::engine
{
	/**
	 * @class Mesh
	 * @brief Ресурс полигональной 3D геометрии.
	 * @details На этапе 16 хранит CPU-данные геометрии, подготовленные к отправке в GPU-буферы на этапе 17.
	 */
	class Mesh : public ::zzz::core::ResourceBase
	{
	public:
		Mesh(const ::zzz::core::Guid& guid, std::string name, ::zzz::core::MeshData meshData);
		~Mesh() override = default;

		[[nodiscard]] zU32 GetVertexCount() const noexcept { return m_MeshData.GetVertexCount(); }
		[[nodiscard]] zU32 GetVertexStride() const noexcept { return m_MeshData.GetVertexStride(); }
		[[nodiscard]] const std::vector<std::byte>& GetVertexData() const noexcept { return m_MeshData.GetVertexData(); }

		[[nodiscard]] zU32 GetIndexCount() const noexcept { return m_MeshData.GetIndexCount(); }
		[[nodiscard]] ::zzz::core::eIndexFormat GetIndexFormat() const noexcept { return m_MeshData.GetIndexFormat(); }
		[[nodiscard]] const std::vector<std::byte>& GetIndexData() const noexcept { return m_MeshData.GetIndexData(); }

		[[nodiscard]] const ::zzz::core::MeshData& GetMeshData() const noexcept { return m_MeshData; }

	private:
		::zzz::core::MeshData m_MeshData;
	};
}
