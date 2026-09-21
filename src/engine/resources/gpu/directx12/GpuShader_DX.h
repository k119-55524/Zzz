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
	class GpuShader_DX final : public ResourceBase
	{
	public:
		using CpuSource = CpuShader;

		GpuShader_DX(const Guid& guid, std::string name)
			: ResourceBase(guid, eResourceType::Shader, std::move(name))
		{
		}

		~GpuShader_DX() override = default;

		[[nodiscard]] static std::shared_ptr<GpuShader_DX> CreateGpuResourceAndUploadFromCpu(ResourceRef<CpuShader> cpuShader)
		{
			ensure(cpuShader != nullptr, "GpuShader_DX::CreateGpuResourceAndUploadFromCpu: cpuShader не должен быть null");
			const auto& guid = cpuShader->GetGuid();
			std::string name(cpuShader->GetName());

			return safe_make_shared<GpuShader_DX>(guid, std::move(name));
		}
	};
}
