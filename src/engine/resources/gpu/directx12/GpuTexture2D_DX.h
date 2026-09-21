#pragma once

#include <string>
#include <memory>

#include "engine/resources/ResourceRef.h"
#include "engine/resources/ResourceBase.h"
#include "engine/resources/cpu/CpuTexture2D.h"

using namespace zzz::core;

namespace zzz::engine
{
	class GpuTexture2D_DX final : public ResourceBase
	{
	public:
		using CpuSource = CpuTexture2D;

		GpuTexture2D_DX(const Guid& guid, std::string name, ResourceRef<CpuTexture2D> cpuTexture)
			: ResourceBase(guid, eResourceType::Texture2D, std::move(name)), m_CpuTexture(std::move(cpuTexture)) {}
		~GpuTexture2D_DX() override = default;

		[[nodiscard]] static std::shared_ptr<GpuTexture2D_DX> CreateFromCpu(ResourceRef<CpuTexture2D> cpuTexture)
		{
			return safe_make_shared<GpuTexture2D_DX>(cpuTexture->GetGuid(), std::string(cpuTexture->GetName()), std::move(cpuTexture));
		}

		[[nodiscard]] const ResourceRef<CpuTexture2D>& GetCpuTexture() const noexcept { return m_CpuTexture; }
		[[nodiscard]] uint32_t GetWidth() const noexcept { return m_CpuTexture ? m_CpuTexture->GetWidth() : 0; }
		[[nodiscard]] uint32_t GetHeight() const noexcept { return m_CpuTexture ? m_CpuTexture->GetHeight() : 0; }

	private:
		ResourceRef<CpuTexture2D> m_CpuTexture;
	};
}
