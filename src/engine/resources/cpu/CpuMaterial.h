#pragma once

#include <span>
#include <string>
#include <memory>
#include <expected>
#include <cstddef>
#include "engine/resources/ResourceBase.h"
#include "core/utils/Guid.h"
#include "core/io/package/PackageEntry.h"

namespace zzz::engine
{
	/**
	 * @class CpuMaterial
	 * @brief Ресурс описания материала в оперативной памяти (CPU).
	 */
	class CpuMaterial final : public ResourceBase
	{
	public:
		CpuMaterial(const ::zzz::core::Guid& guid, std::string name, ::zzz::core::Guid shaderGuid = {});
		~CpuMaterial() override = default;

		[[nodiscard]] static std::expected<std::shared_ptr<CpuMaterial>, std::string> CreateFromMemory(
			const ::zzz::core::PackageEntry& entry,
			std::span<const std::byte> bytes);

		[[nodiscard]] const ::zzz::core::Guid& GetShaderGuid() const noexcept { return m_ShaderGuid; }

	private:
		::zzz::core::Guid m_ShaderGuid;
	};
}
