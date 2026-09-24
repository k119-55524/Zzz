#pragma once

#include <string>
#include <memory>
#include "core/utils/Ensure.h"
#include "core/utils/MemoryUtils.h"
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

		GpuShader_Metal(const Guid& guid, std::string name)
			: ResourceBase(guid, eEngineResourceType::Shader, std::move(name))
		{
		}

		~GpuShader_Metal() override = default;

		[[nodiscard]] static std::shared_ptr<GpuShader_Metal> CreateGpuResourceAndUploadFromCpu(ResourceRef<CpuShader> cpuShader)
		{
			ensure(cpuShader != nullptr, "GpuShader_Metal::CreateGpuResourceAndUploadFromCpu: cpuShader не должен быть null");
			const auto& guid = cpuShader->GetGuid();
			std::string name(cpuShader->GetName());

			return safe_make_shared<GpuShader_Metal>(guid, std::move(name));
		}
	};
}
