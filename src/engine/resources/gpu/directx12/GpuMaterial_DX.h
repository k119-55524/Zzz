#pragma once

#include <string>
#include <memory>
#include "core/utils/Ensure.h"
#include "core/utils/MemoryUtils.h"
#include "engine/resources/ResourceRef.h"
#include "engine/resources/ResourceBase.h"
#include "engine/resources/cpu/CpuMaterial.h"

using namespace zzz::core;

namespace zzz::engine
{
	class GpuMaterial_DX final : public ResourceBase
	{
	public:
		using CpuSource = CpuMaterial;

		GpuMaterial_DX(const Guid& guid, std::string name, const Guid& shaderGuid)
			: ResourceBase(guid, eResourceType::Material, std::move(name))
			, m_ShaderGuid(shaderGuid)
		{
		}

		~GpuMaterial_DX() override = default;

		[[nodiscard]] static std::shared_ptr<GpuMaterial_DX> CreateGpuResourceAndUploadFromCpu(ResourceRef<CpuMaterial> cpuMaterial)
		{
			ensure(cpuMaterial != nullptr, "GpuMaterial_DX::CreateGpuResourceAndUploadFromCpu: cpuMaterial не должен быть null");
			const auto& guid = cpuMaterial->GetGuid();
			std::string name(cpuMaterial->GetName());
			const Guid shaderGuid = cpuMaterial->GetShaderGuid();

			return safe_make_shared<GpuMaterial_DX>(guid, std::move(name), shaderGuid);
		}

		[[nodiscard]] const Guid& GetShaderGuid() const noexcept { return m_ShaderGuid; }

	private:
		Guid m_ShaderGuid;
	};
}
