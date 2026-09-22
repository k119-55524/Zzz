#pragma once

#include <string>
#include <format>
#include <memory>
#include <expected>
#include <concepts>

#include "core/utils/Guid.h"
#include "engine/gapi/GAPI.h"
#include "engine/tasks/TaskPriority.h"
#include "engine/resources/gpu/GpuMesh.h"
#include "engine/resources/ResourceTable.h"
#include "engine/resources/gpu/GpuShader.h"
#include "engine/resources/gpu/GpuMaterial.h"
#include "engine/resources/gpu/GpuTexture2D.h"
#include "engine/resources/cpu/CpuResourceManager.h"

using namespace zzz::core;

namespace zzz::engine
{
	/**
	 * @concept GpuResource
	 * @brief Концепт валидного GPU-ресурса движка.
	 * @details Требует наличие ассоциированного CpuSource и статического метода CreateGpuResourceAndUploadFromCpu.
	 */
	template<typename T>
	concept GpuResource = requires(ResourceRef<typename T::CpuSource> cpuRef)
	{
		typename T::CpuSource;
		{ T::CreateGpuResourceAndUploadFromCpu(std::move(cpuRef)) } -> std::same_as<std::shared_ptr<T>>;
	};

	/**
	 * @class GpuResourceManager
	 * @brief Централизованный сервис управления видеопамятью и GPU-ресурсами.
	 * @details Хранит явные таблицы GPU-ресурсов. При запросе GetAsync<TGpu> запрашивает
	 *          соответствующий CPU-ресурс у CpuResourceManager и инстанциирует GPU-ресурс.
	 */
	class GpuResourceManager final : public std::enable_shared_from_this<GpuResourceManager>
	{
		Z_NO_COPY_MOVE(GpuResourceManager);

	public:
		GpuResourceManager() = delete;

		explicit GpuResourceManager(
			std::shared_ptr<GAPI> gapi,
			std::shared_ptr<CpuResourceManager> cpuResourceManager);

		~GpuResourceManager();

		template<GpuResource T, typename ContextType>
		void GetAsync(
			const Guid& guid,
			std::weak_ptr<ContextType> context,
			std::function<void(std::expected<ResourceRef<T>, std::string>)> onLoaded,
			eTaskPriority priority)
		{
			ensure(priority != eTaskPriority::Critical, "Приоритет Critical зарезервирован строго для кадровых задач движка!");

			GetTable<T>().GetOrRequest(
				guid,
				context,
				[cb = std::move(onLoaded)](std::expected<std::shared_ptr<T>, std::string> res)
				{
					if (!res)
						cb(std::unexpected(std::move(res.error())));
					else
						cb(ResourceRef<T>(std::move(*res)));
				},
				[this, priority](const Guid& guid)
				{
					RequestFromCpu<T>(guid, priority);
				});
		}

		template<GpuResource T>
		[[nodiscard]] ResourceRef<T> TryGet(const Guid& guid)
		{
			auto res = GetTable<T>().TryGet(guid);
			return res ? ResourceRef<T>(std::move(res)) : ResourceRef<T>{};
		}

	private:
		template<GpuResource T>
		[[nodiscard]] auto& GetTable() noexcept
		{
			if constexpr (std::is_same_v<T, GpuMesh>)           return m_Meshes;
			else if constexpr (std::is_same_v<T, GpuMaterial>)  return m_Materials;
			else if constexpr (std::is_same_v<T, GpuTexture2D>) return m_Textures;
			else if constexpr (std::is_same_v<T, GpuShader>)    return m_Shaders;
			else static_assert(sizeof(T) == 0, "Запрашиваемый тип ресурса не поддерживается GpuResourceManager!");
		}

		void EmergencyStop();

		template<GpuResource TGpu>
		void RequestFromCpu(const Guid& guid, eTaskPriority priority)
		{
			using TCpu = TGpu::CpuSource;

			m_CpuManager->GetAsync<TCpu>(guid, weak_from_this(), [this, guid](std::expected<ResourceRef<TCpu>, std::string> cpuRes)
			{
				if (!cpuRes)
				{
					GetTable<TGpu>().Resolve(guid, std::unexpected(cpuRes.error()));
					return;
				}

				try
				{
					auto gpuObj = TGpu::CreateGpuResourceAndUploadFromCpu(std::move(*cpuRes));
					GetTable<TGpu>().Resolve(guid, std::move(gpuObj));
				}
				catch (const std::exception& ex)
				{
					GetTable<TGpu>().Resolve(guid, std::unexpected(std::format("Ошибка создания GPU-ресурса: {}", ex.what())));
				}
				catch (...)
				{
					GetTable<TGpu>().Resolve(guid, std::unexpected(std::string("Неизвестная ошибка создания GPU-ресурса")));
				}
			}, priority);
		}

		std::shared_ptr<GAPI> m_GAPI;
		std::shared_ptr<CpuResourceManager> m_CpuManager;

		ResourceTable<GpuMesh>      m_Meshes;
		ResourceTable<GpuMaterial>  m_Materials;
		ResourceTable<GpuTexture2D> m_Textures;
		ResourceTable<GpuShader>    m_Shaders;
	};
}
