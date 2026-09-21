#pragma once

#include <string>
#include <memory>

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

		GpuMaterial_DX(const Guid& guid, std::string name, ResourceRef<CpuMaterial> cpuMaterial)
			: ResourceBase(guid, eResourceType::Material, std::move(name)), m_CpuMaterial(std::move(cpuMaterial)) {}
		~GpuMaterial_DX() override = default;

		[[nodiscard]] static std::shared_ptr<GpuMaterial_DX> CreateFromCpu(ResourceRef<CpuMaterial> cpuMaterial)
		{
			return safe_make_shared<GpuMaterial_DX>(cpuMaterial->GetGuid(), std::string(cpuMaterial->GetName()), std::move(cpuMaterial));
		}

		[[nodiscard]] const ResourceRef<CpuMaterial>& GetCpuMaterial() const noexcept { return m_CpuMaterial; }

	private:
		ResourceRef<CpuMaterial> m_CpuMaterial;
	};
}
