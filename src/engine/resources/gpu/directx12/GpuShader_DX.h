#pragma once

#include <string>
#include <memory>

#include "engine/resources/ResourceRef.h"
#include "engine/resources/ResourceBase.h"
#include "engine/resources/cpu/CpuShader.h"

using namespace zzz::core;

namespace zzz::engine
{
	class GpuShader_DX final : public ResourceBase
	{
	public:
		using CpuSource = CpuShader;

		GpuShader_DX(const Guid& guid, std::string name, ResourceRef<CpuShader> cpuShader)
			: ResourceBase(guid, eResourceType::Shader, std::move(name)), m_CpuShader(std::move(cpuShader)) {}
		~GpuShader_DX() override = default;

		[[nodiscard]] static std::shared_ptr<GpuShader_DX> CreateFromCpu(ResourceRef<CpuShader> cpuShader)
		{
			return safe_make_shared<GpuShader_DX>(cpuShader->GetGuid(), std::string(cpuShader->GetName()), std::move(cpuShader));
		}

		[[nodiscard]] const ResourceRef<CpuShader>& GetCpuShader() const noexcept { return m_CpuShader; }

	private:
		ResourceRef<CpuShader> m_CpuShader;
	};
}
