#pragma once

#include <memory>
#include <string>
#include "engine/resources/ResourceBase.h"
#include "engine/resources/ResourceRef.h"
#include "engine/resources/cpu/CpuTexture2D.h"

namespace zzz::engine
{
	/**
	 * @class GpuTexture2D
	 * @brief Ресурс 2D-текстуры в видеопамяти (GPU).
	 */
	class GpuTexture2D final : public ResourceBase
	{
	public:
		using CpuType = CpuTexture2D;

		GpuTexture2D(
			const ::zzz::core::Guid& guid,
			std::string name,
			ResourceRef<CpuTexture2D> cpuTexture);
		~GpuTexture2D() override = default;

		[[nodiscard]] const ResourceRef<CpuTexture2D>& GetCpuTexture() const noexcept { return m_CpuTexture; }
		[[nodiscard]] uint32_t GetWidth() const noexcept { return m_CpuTexture ? m_CpuTexture->GetWidth() : 0; }
		[[nodiscard]] uint32_t GetHeight() const noexcept { return m_CpuTexture ? m_CpuTexture->GetHeight() : 0; }

	private:
		ResourceRef<CpuTexture2D> m_CpuTexture;
	};
}
