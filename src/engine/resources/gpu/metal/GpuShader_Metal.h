#pragma once

#include <string>
#include <memory>

#include "engine/resources/ResourceRef.h"
#include "engine/resources/ResourceBase.h"
#include "engine/resources/cpu/CpuShader.h"

using namespace zzz::core;

namespace zzz::engine
{
	class GpuShader_Metal final : public ResourceBase
	{
	public:
		using CpuSource = CpuShader;

		GpuShader_Metal(const Guid& guid, std::string name, ResourceRef<CpuShader> cpuShader)
			: ResourceBase(guid, eResourceType::Shader, std::move(name)), m_CpuShader(std::move(cpuShader)) {}
		~GpuShader_Metal() override = default;

		[[nodiscard]] static std::shared_ptr<GpuShader_Metal> CreateFromCpu(ResourceRef<CpuShader> cpuShader)
		{
			return safe_make_shared<GpuShader_Metal>(cpuShader->GetGuid(), std::string(cpuShader->GetName()), std::move(cpuShader));
		}

		[[nodiscard]] const ResourceRef<CpuShader>& GetCpuShader() const noexcept { return m_CpuShader; }

	private:
		ResourceRef<CpuShader> m_CpuShader;
	};
}
