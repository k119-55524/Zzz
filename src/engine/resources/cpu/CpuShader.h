#pragma once

#include <span>
#include <string>
#include <memory>
#include <expected>
#include <cstddef>
#include "engine/resources/ResourceBase.h"
#include "core/io/package/PackageEntry.h"
#include "core/enums/eResourceType.h"

namespace zzz::engine
{
	/**
	 * @class CpuShader
	 * @brief Ресурс шейдера в оперативной памяти (CPU).
	 */
	class CpuShader final : public ResourceBase
	{
	public:
		static constexpr ::zzz::core::eResourceType c_ResourceType = ::zzz::core::eResourceType::Shader;

		CpuShader(const ::zzz::core::Guid& guid, std::string name);
		~CpuShader() override = default;

		[[nodiscard]] static std::expected<std::shared_ptr<CpuShader>, std::string> CreateCpuResourceFromPackageBytes(
			const ::zzz::core::PackageEntry& entry,
			std::span<const std::byte> bytes);
	};
}
