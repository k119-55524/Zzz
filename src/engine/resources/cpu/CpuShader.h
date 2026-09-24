#pragma once

#include <span>
#include <string>
#include <memory>
#include <expected>
#include <cstddef>
#include "engine/resources/ResourceBase.h"
#include "core/io/package/PackageEntry.h"
#include "core/enums/eEngineResourceType.h"

namespace zzz::engine
{
	/**
	 * @class CpuShader
	 * @brief Ресурс шейдера в оперативной памяти (CPU).
	 */
	class CpuShader final : public ResourceBase
	{
	public:
		static constexpr ::zzz::core::eEngineResourceType c_ResourceType = ::zzz::core::eEngineResourceType::Shader;

		CpuShader(const ::zzz::core::Guid& guid, std::string name);
		~CpuShader() override = default;

		[[nodiscard]] static std::expected<std::shared_ptr<CpuShader>, std::string> CreateCpuResourceFromPackageBytes(
			const ::zzz::core::PackageEntry& entry,
			std::span<const std::byte> bytes);
	};
}
