#pragma once

#include <memory>
#include <string>
#include <format>
#include <expected>

#include "core/utils/Ensure.h"
#include "core/utils/MemoryUtils.h"
#include "engine/resources/ResourceRef.h"
#include "engine/resources/gpu/GpuMesh.h"
#include "engine/resources/gpu/GpuMaterial.h"
#include "engine/resources/gpu/GpuShader.h"
#include "engine/resources/gpu/GpuTexture2D.h"

namespace zzz::engine
{
	/**
	 * @class GpuResourceBuilder
	 * @brief Фабрика конструирования платформенно-нейтральных GPU-ресурсов (RAM-заглушек) на воркерах.
	 * @details На этапе 22 создаёт C++ объекты в оперативной памяти без прямых вызовов GAPI.
	 *          GpuMesh удерживает ResourceRef<CpuMesh> до этапа 23, где специализированный конвейер
	 *          (GpuUploadScheduler) перенесёт данные в видеопамять без блокировки CPU-воркеров.
	 */
	template<typename TGpu>
	class GpuResourceBuilder;

	template<>
	class GpuResourceBuilder<GpuMesh> final
	{
	public:
		GpuResourceBuilder() = delete;

		[[nodiscard]] static std::shared_ptr<GpuMesh> Build(ResourceRef<CpuMesh> cpuMesh)
		{
			ensure(cpuMesh != nullptr, "GpuResourceBuilder<GpuMesh>::Build: cpuMesh не должен быть null");
			const auto& guid = cpuMesh->GetGuid();
			std::string name(cpuMesh->GetName());
			// На этапе 22: создаём RAM-заглушку GpuMesh, удерживающую ResourceRef<CpuMesh>.
			// Реальный трансфер в видеопамять (GPU upload) реализуется на этапе 23.
			return core::safe_make_shared<GpuMesh>(guid, std::move(name), std::move(cpuMesh));
		}
	};

	template<>
	class GpuResourceBuilder<GpuMaterial> final
	{
	public:
		GpuResourceBuilder() = delete;

		[[nodiscard]] static std::shared_ptr<GpuMaterial> Build(ResourceRef<CpuMaterial> cpuMaterial)
		{
			ensure(cpuMaterial != nullptr, "GpuResourceBuilder<GpuMaterial>::Build: cpuMaterial не должен быть null");
			const auto& guid = cpuMaterial->GetGuid();
			std::string name(cpuMaterial->GetName());
			return core::safe_make_shared<GpuMaterial>(guid, std::move(name), std::move(cpuMaterial));
		}
	};

	template<>
	class GpuResourceBuilder<GpuShader> final
	{
	public:
		GpuResourceBuilder() = delete;

		[[nodiscard]] static std::shared_ptr<GpuShader> Build(ResourceRef<CpuShader> cpuShader)
		{
			ensure(cpuShader != nullptr, "GpuResourceBuilder<GpuShader>::Build: cpuShader не должен быть null");
			const auto& guid = cpuShader->GetGuid();
			std::string name(cpuShader->GetName());
			return core::safe_make_shared<GpuShader>(guid, std::move(name), std::move(cpuShader));
		}
	};

	template<>
	class GpuResourceBuilder<GpuTexture2D> final
	{
	public:
		GpuResourceBuilder() = delete;

		[[nodiscard]] static std::shared_ptr<GpuTexture2D> Build(ResourceRef<CpuTexture2D> cpuTexture)
		{
			ensure(cpuTexture != nullptr, "GpuResourceBuilder<GpuTexture2D>::Build: cpuTexture не должен быть null");
			const auto& guid = cpuTexture->GetGuid();
			std::string name(cpuTexture->GetName());
			return core::safe_make_shared<GpuTexture2D>(guid, std::move(name), std::move(cpuTexture));
		}
	};
}
