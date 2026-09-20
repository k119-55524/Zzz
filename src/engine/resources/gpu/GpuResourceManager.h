#pragma once

#include <memory>
#include <string>
#include <format>
#include <expected>

#include "core/utils/Guid.h"
#include "engine/gapi/GAPI.h"
#include "engine/tasks/TaskDispatcher.h"
#include "core/templates/CallbackQueue.h"
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
			TaskDispatcher& taskDispatcher,
			std::shared_ptr<GAPI> gapi,
			std::shared_ptr<CpuResourceManager> cpuResourceManager);

		~GpuResourceManager();

		inline void Update() { m_MainThreadQueue.ExecuteAll(); }

		template<typename T, typename ContextType>
		void GetAsync(const Guid& guid, std::weak_ptr<ContextType> context, std::function<void(std::expected<ResourceRef<T>, std::string>)> onLoaded)
		{
			GetTable<T>().GetOrRequest(
				guid,
				std::move(context),
				[cb = std::move(onLoaded)](typename ResourceTable<T>::ResultType res)
				{
					if (!res)
						cb(std::unexpected(std::move(res.error())));
					else
						cb(ResourceRef<T>(std::move(*res)));
				},
				GetDispatcher(),
				[this](const Guid& guid)
				{
					RequestFromCpu<T>(guid);
				});
		}

		template<typename T>
		void GetAsync(const Guid& guid, std::function<void(std::expected<ResourceRef<T>, std::string>)> onLoaded)
		{
			GetTable<T>().GetOrRequest(
				guid,
				[cb = std::move(onLoaded)](typename ResourceTable<T>::ResultType res)
				{
					if (!res)
						cb(std::unexpected(std::move(res.error())));
					else
						cb(ResourceRef<T>(std::move(*res)));
				},
				GetDispatcher(),
				[this](const Guid& guid)
				{
					RequestFromCpu<T>(guid);
				});
		}

		template<typename T>
		[[nodiscard]] ResourceRef<T> TryGet(const Guid& guid)
		{
			auto res = GetTable<T>().TryGet(guid);
			return res ? ResourceRef<T>(std::move(res)) : ResourceRef<T>{};
		}

	private:
		template<typename T>
		[[nodiscard]] auto& GetTable() noexcept
		{
			if constexpr (std::is_same_v<T, GpuMesh>)           return m_Meshes;
			else if constexpr (std::is_same_v<T, GpuMaterial>)  return m_Materials;
			else if constexpr (std::is_same_v<T, GpuTexture2D>) return m_Textures;
			else if constexpr (std::is_same_v<T, GpuShader>)    return m_Shaders;
			else static_assert(sizeof(T) == 0, "Запрашиваемый тип ресурса не поддерживается GpuResourceManager!");
		}

		void EmergencyStop();

		[[nodiscard]] auto GetDispatcher()
		{
			return [this](auto task) { m_MainThreadQueue.Push(std::move(task)); };
		}

		template<typename TGpu>
		void RequestFromCpu(const Guid& guid)
		{
			using TCpu = typename TGpu::CpuType;

			m_CpuManager->GetAsync<TCpu>(guid, weak_from_this(), [this, guid](std::expected<ResourceRef<TCpu>, std::string> cpuRes)
			{
				if (!cpuRes)
				{
					GetTable<TGpu>().Resolve(guid, std::unexpected(cpuRes.error()), GetDispatcher());
					return;
				}

				m_TaskDispatcher.Submit(eTaskPriority::Normal, [this, guid, cpuRef = std::move(*cpuRes)]() mutable
				{
					try
					{
						auto gpuObj = safe_make_shared<TGpu>(guid, std::string(cpuRef->GetName()), std::move(cpuRef));
						GetTable<TGpu>().Resolve(guid, std::move(gpuObj), GetDispatcher());
					}
					catch (const std::exception& ex)
					{
						GetTable<TGpu>().Resolve(guid, std::unexpected(std::format("Ошибка создания GPU-ресурса: {}", ex.what())), GetDispatcher());
					}
					catch (...)
					{
						GetTable<TGpu>().Resolve(guid, std::unexpected(std::string("Неизвестная ошибка создания GPU-ресурса")), GetDispatcher());
					}
				});
			});
		}

		TaskDispatcher& m_TaskDispatcher;
		std::shared_ptr<GAPI> m_GAPI;
		std::shared_ptr<CpuResourceManager> m_CpuManager;

		CallbackQueue<> m_MainThreadQueue;

		ResourceTable<GpuMesh>      m_Meshes;
		ResourceTable<GpuMaterial>  m_Materials;
		ResourceTable<GpuTexture2D> m_Textures;
		ResourceTable<GpuShader>    m_Shaders;
	};
}
