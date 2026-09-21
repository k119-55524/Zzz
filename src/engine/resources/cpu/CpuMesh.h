#pragma once
#include <span>
#include <string>
#include <vector>
#include <memory>
#include <expected>
#include <cstddef>
#include "engine/resources/ResourceBase.h"
#include "core/io/package/MeshData.h"
#include "core/io/package/PackageEntry.h"
#include "core/enums/eIndexFormat.h"

namespace zzz::engine
{
	/**
	 * @class CpuMesh
	 * @brief Ресурс полигональной 3D-геометрии в оперативной памяти (CPU).
	 */
	class CpuMesh final : public ResourceBase
	{
	public:
		CpuMesh(const ::zzz::core::Guid& guid, std::string name, ::zzz::core::MeshData meshData);
		~CpuMesh() override = default;

		[[nodiscard]] static std::expected<std::shared_ptr<CpuMesh>, std::string> CreateCpuResourceFromPackageBytes(
			const ::zzz::core::PackageEntry& entry,
			std::span<const std::byte> bytes);

		[[nodiscard]] zU32 GetVertexCount() const noexcept { return m_MeshData.GetVertexCount(); }
		[[nodiscard]] zU32 GetVertexStride() const noexcept { return m_MeshData.GetVertexStride(); }
		[[nodiscard]] std::span<const std::byte> GetVertexData() const noexcept { return m_MeshData.GetVertexData(); }

		[[nodiscard]] zU32 GetIndexCount() const noexcept { return m_MeshData.GetIndexCount(); }
		[[nodiscard]] ::zzz::core::eIndexFormat GetIndexFormat() const noexcept { return m_MeshData.GetIndexFormat(); }
		[[nodiscard]] std::span<const std::byte> GetIndexData() const noexcept { return m_MeshData.GetIndexData(); }

		[[nodiscard]] const ::zzz::core::MeshData& GetMeshData() const noexcept { return m_MeshData; }

	private:
		::zzz::core::MeshData m_MeshData;
	};
}
