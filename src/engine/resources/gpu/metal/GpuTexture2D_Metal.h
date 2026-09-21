#pragma once

#include <string>
#include <memory>
#include <cstdint>
#include "core/utils/Ensure.h"
#include "core/utils/MemoryUtils.h"
#include "engine/resources/ResourceRef.h"
#include "engine/resources/ResourceBase.h"
#include "engine/resources/cpu/CpuTexture2D.h"

using namespace zzz::core;

namespace zzz::engine
{
	class GpuTexture2D_Metal final : public ResourceBase
	{
	public:
		using CpuSource = CpuTexture2D;

		GpuTexture2D_Metal(const Guid& guid, std::string name, uint32_t width, uint32_t height)
			: ResourceBase(guid, eResourceType::Texture2D, std::move(name))
			, m_Width(width)
			, m_Height(height)
		{
		}

		~GpuTexture2D_Metal() override = default;

		[[nodiscard]] static std::shared_ptr<GpuTexture2D_Metal> CreateGpuResourceAndUploadFromCpu(ResourceRef<CpuTexture2D> cpuTexture)
		{
			ensure(cpuTexture != nullptr, "GpuTexture2D_Metal::CreateGpuResourceAndUploadFromCpu: cpuTexture не должен быть null");
			const auto& guid = cpuTexture->GetGuid();
			std::string name(cpuTexture->GetName());
			const uint32_t width = cpuTexture->GetWidth();
			const uint32_t height = cpuTexture->GetHeight();

			return safe_make_shared<GpuTexture2D_Metal>(guid, std::move(name), width, height);
		}

		[[nodiscard]] uint32_t GetWidth() const noexcept { return m_Width; }
		[[nodiscard]] uint32_t GetHeight() const noexcept { return m_Height; }

	private:
		uint32_t m_Width{ 0 };
		uint32_t m_Height{ 0 };
	};
}
