#pragma once

#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <shared_mutex>
#include <functional>
#include <expected>

#include "core/utils/Guid.h"
#include "core/utils/Defines.h"
#include "core/templates/CallbackQueue.h"
#include "engine/resources/ResourceRecord.h"
#include "engine/resources/gpu/GpuMesh.h"
#include "engine/resources/gpu/GpuMaterial.h"
#include "engine/resources/gpu/GpuTexture2D.h"
#include "engine/resources/gpu/GpuShader.h"

#include "engine/gapi/GAPI.h"

namespace zzz::engine
{
	class CpuResourceManager;

	template <typename T>
	using GpuResourceResult = std::expected<std::shared_ptr<T>, std::string>;

	template <typename T>
	using GpuResourceCallback = std::function<void(GpuResourceResult<T>)>;

	/**
	 * @class GpuResourceManager
	 * @brief Менеджер видеопамяти и GPU-ресурсов.
	 * @details Связан через внедрение зависимостей (DI) с GAPI и CpuResourceManager.
	 * При запросе GPU-ресурса прозрачно подгружает CPU-ресурс, формирует GPU-буферы и разрешает OneShotEvent.
	 */
	class GpuResourceManager final : public std::enable_shared_from_this<GpuResourceManager>
	{
		Z_NO_COPY_MOVE(GpuResourceManager);

	public:
		GpuResourceManager() = delete;
		explicit GpuResourceManager(
			std::shared_ptr<GAPI> gapi,
			std::shared_ptr<CpuResourceManager> cpuResourceManager);
		~GpuResourceManager() = default;

		// --- Обновление и выполнение колбэков на главном потоке ---
		void Update();

		// --- Методы фабрикации GPU-ресурсов (на этапе 22 - обёртки, на этапе 23 - GAPI staging) ---
		[[nodiscard]] std::shared_ptr<GpuMesh>      CreateGpuMesh(std::shared_ptr<CpuMesh> cpuMesh);
		[[nodiscard]] std::shared_ptr<GpuMaterial>  CreateGpuMaterial(std::shared_ptr<CpuMaterial> cpuMaterial);
		[[nodiscard]] std::shared_ptr<GpuTexture2D> CreateGpuTexture(std::shared_ptr<CpuTexture2D> cpuTexture);
		[[nodiscard]] std::shared_ptr<GpuShader>    CreateGpuShader(std::shared_ptr<CpuShader> cpuShader);

		// --- Асинхронные методы загрузки GPU-ресурсов ---
		// --- Унифицированная шаблонная асинхронная загрузка LoadAsync<T> ---
		template<typename T, typename ContextType>
		void LoadAsync(
			const ::zzz::core::Guid& guid,
			std::weak_ptr<ContextType> context,
			GpuResourceCallback<T> onLoaded)
		{
			if constexpr (std::is_same_v<T, GpuMesh>)
			{
				LoadGpuResourceInternal<GpuMesh, CpuMesh>(
					guid, m_GpuMeshes, std::move(context), std::move(onLoaded),
					[this](const ::zzz::core::Guid& g, auto ctx, auto cb) { RequestCpuMesh(g, std::move(ctx), std::move(cb)); },
					[this](std::shared_ptr<CpuMesh> cpuRes) { return CreateGpuMesh(std::move(cpuRes)); });
			}
			else if constexpr (std::is_same_v<T, GpuMaterial>)
			{
				LoadGpuResourceInternal<GpuMaterial, CpuMaterial>(
					guid, m_GpuMaterials, std::move(context), std::move(onLoaded),
					[this](const ::zzz::core::Guid& g, auto ctx, auto cb) { RequestCpuMaterial(g, std::move(ctx), std::move(cb)); },
					[this](std::shared_ptr<CpuMaterial> cpuRes) { return CreateGpuMaterial(std::move(cpuRes)); });
			}
			else if constexpr (std::is_same_v<T, GpuTexture2D>)
			{
				LoadGpuResourceInternal<GpuTexture2D, CpuTexture2D>(
					guid, m_GpuTextures, std::move(context), std::move(onLoaded),
					[this](const ::zzz::core::Guid& g, auto ctx, auto cb) { RequestCpuTexture(g, std::move(ctx), std::move(cb)); },
					[this](std::shared_ptr<CpuTexture2D> cpuRes) { return CreateGpuTexture(std::move(cpuRes)); });
			}
			else if constexpr (std::is_same_v<T, GpuShader>)
			{
				LoadGpuResourceInternal<GpuShader, CpuShader>(
					guid, m_GpuShaders, std::move(context), std::move(onLoaded),
					[this](const ::zzz::core::Guid& g, auto ctx, auto cb) { RequestCpuShader(g, std::move(ctx), std::move(cb)); },
					[this](std::shared_ptr<CpuShader> cpuRes) { return CreateGpuShader(std::move(cpuRes)); });
			}
			else
			{
				static_assert(sizeof(T) == 0, "Неподдерживаемый тип ресурса для LoadAsync<T>");
			}
		}

		template<typename T>
		void LoadAsync(
			const ::zzz::core::Guid& guid,
			GpuResourceCallback<T> onLoaded)
		{
			if constexpr (std::is_same_v<T, GpuMesh>)
			{
				LoadGpuResourceInternal<GpuMesh, CpuMesh>(
					guid, m_GpuMeshes, std::move(onLoaded),
					[this](const ::zzz::core::Guid& g, auto ctx, auto cb) { RequestCpuMesh(g, std::move(ctx), std::move(cb)); },
					[this](std::shared_ptr<CpuMesh> cpuRes) { return CreateGpuMesh(std::move(cpuRes)); });
			}
			else if constexpr (std::is_same_v<T, GpuMaterial>)
			{
				LoadGpuResourceInternal<GpuMaterial, CpuMaterial>(
					guid, m_GpuMaterials, std::move(onLoaded),
					[this](const ::zzz::core::Guid& g, auto ctx, auto cb) { RequestCpuMaterial(g, std::move(ctx), std::move(cb)); },
					[this](std::shared_ptr<CpuMaterial> cpuRes) { return CreateGpuMaterial(std::move(cpuRes)); });
			}
			else if constexpr (std::is_same_v<T, GpuTexture2D>)
			{
				LoadGpuResourceInternal<GpuTexture2D, CpuTexture2D>(
					guid, m_GpuTextures, std::move(onLoaded),
					[this](const ::zzz::core::Guid& g, auto ctx, auto cb) { RequestCpuTexture(g, std::move(ctx), std::move(cb)); },
					[this](std::shared_ptr<CpuTexture2D> cpuRes) { return CreateGpuTexture(std::move(cpuRes)); });
			}
			else if constexpr (std::is_same_v<T, GpuShader>)
			{
				LoadGpuResourceInternal<GpuShader, CpuShader>(
					guid, m_GpuShaders, std::move(onLoaded),
					[this](const ::zzz::core::Guid& g, auto ctx, auto cb) { RequestCpuShader(g, std::move(ctx), std::move(cb)); },
					[this](std::shared_ptr<CpuShader> cpuRes) { return CreateGpuShader(std::move(cpuRes)); });
			}
			else
			{
				static_assert(sizeof(T) == 0, "Неподдерживаемый тип ресурса для LoadAsync<T>");
			}
		}

		// --- Именованные методы асинхронной загрузки (делегируют в LoadAsync<T>) ---
		template<typename ContextType>
		void LoadGpuMeshAsync(const ::zzz::core::Guid& guid, std::weak_ptr<ContextType> context, GpuResourceCallback<GpuMesh> onLoaded)
		{
			LoadAsync<GpuMesh>(guid, std::move(context), std::move(onLoaded));
		}

		void LoadGpuMeshAsync(const ::zzz::core::Guid& guid, GpuResourceCallback<GpuMesh> onLoaded)
		{
			LoadAsync<GpuMesh>(guid, std::move(onLoaded));
		}

		template<typename ContextType>
		void LoadGpuMaterialAsync(const ::zzz::core::Guid& guid, std::weak_ptr<ContextType> context, GpuResourceCallback<GpuMaterial> onLoaded)
		{
			LoadAsync<GpuMaterial>(guid, std::move(context), std::move(onLoaded));
		}

		void LoadGpuMaterialAsync(const ::zzz::core::Guid& guid, GpuResourceCallback<GpuMaterial> onLoaded)
		{
			LoadAsync<GpuMaterial>(guid, std::move(onLoaded));
		}

		template<typename ContextType>
		void LoadGpuTextureAsync(const ::zzz::core::Guid& guid, std::weak_ptr<ContextType> context, GpuResourceCallback<GpuTexture2D> onLoaded)
		{
			LoadAsync<GpuTexture2D>(guid, std::move(context), std::move(onLoaded));
		}

		void LoadGpuTextureAsync(const ::zzz::core::Guid& guid, GpuResourceCallback<GpuTexture2D> onLoaded)
		{
			LoadAsync<GpuTexture2D>(guid, std::move(onLoaded));
		}

		template<typename ContextType>
		void LoadGpuShaderAsync(const ::zzz::core::Guid& guid, std::weak_ptr<ContextType> context, GpuResourceCallback<GpuShader> onLoaded)
		{
			LoadAsync<GpuShader>(guid, std::move(context), std::move(onLoaded));
		}

		void LoadGpuShaderAsync(const ::zzz::core::Guid& guid, GpuResourceCallback<GpuShader> onLoaded)
		{
			LoadAsync<GpuShader>(guid, std::move(onLoaded));
		}

		// --- Шаблонный быстрый доступ к кэшу GPU ---
		template<typename T>
		[[nodiscard]] std::shared_ptr<T> Get(const ::zzz::core::Guid& guid) const
		{
			std::shared_lock lock(m_Mutex);
			const auto& table = GetTable<T>();
			auto it = table.find(guid);
			return it != table.end() ? it->second.resource : nullptr;
		}

		template<typename T>
		[[nodiscard]] bool Has(const ::zzz::core::Guid& guid) const noexcept
		{
			std::shared_lock lock(m_Mutex);
			const auto& table = GetTable<T>();
			auto it = table.find(guid);
			return it != table.end() && it->second.resource != nullptr;
		}

	private:
		template<typename T>
		[[nodiscard]] auto& GetTable() noexcept
		{
			if constexpr (std::is_same_v<T, GpuMesh>)           return m_GpuMeshes;
			else if constexpr (std::is_same_v<T, GpuMaterial>)  return m_GpuMaterials;
			else if constexpr (std::is_same_v<T, GpuTexture2D>) return m_GpuTextures;
			else if constexpr (std::is_same_v<T, GpuShader>)    return m_GpuShaders;
		}

		template<typename T>
		[[nodiscard]] const auto& GetTable() const noexcept
		{
			if constexpr (std::is_same_v<T, GpuMesh>)           return m_GpuMeshes;
			else if constexpr (std::is_same_v<T, GpuMaterial>)  return m_GpuMaterials;
			else if constexpr (std::is_same_v<T, GpuTexture2D>) return m_GpuTextures;
			else if constexpr (std::is_same_v<T, GpuShader>)    return m_GpuShaders;
		}

		template<typename GpuT, typename CpuT, typename ContextType, typename RequestFunc, typename CreateFunc>
		void LoadGpuResourceInternal(
			const ::zzz::core::Guid& guid,
			std::unordered_map<::zzz::core::Guid, ResourceRecord<GpuT>>& table,
			std::weak_ptr<ContextType> context,
			GpuResourceCallback<GpuT> onLoaded,
			RequestFunc&& requestFunc,
			CreateFunc&& createFunc)
		{
			std::unique_lock lock(m_Mutex);
			auto [it, inserted] = table.try_emplace(guid, [this](auto task) { m_MainThreadQueue.Push(std::move(task)); });
			it->second.readyEvent.Subscribe(std::move(context), std::move(onLoaded));
			if (!inserted)
			{
				return;
			}

			requestFunc(guid, weak_from_this(), [this, &table, guid, createFunc = std::forward<CreateFunc>(createFunc)](std::expected<std::shared_ptr<CpuT>, std::string> cpuRes)
			{
				if (!cpuRes)
				{
					std::unique_lock l(m_Mutex);
					auto it = table.find(guid);
					if (it != table.end())
					{
						it->second.readyEvent.Resolve(std::unexpected(cpuRes.error()));
					}
					return;
				}

				auto gpuObj = createFunc(*cpuRes);
				std::unique_lock l(m_Mutex);
				auto it = table.find(guid);
				if (it != table.end())
				{
					it->second.resource = gpuObj;
					it->second.readyEvent.Resolve(gpuObj);
				}
			});
		}

		template<typename GpuT, typename CpuT, typename RequestFunc, typename CreateFunc>
		void LoadGpuResourceInternal(
			const ::zzz::core::Guid& guid,
			std::unordered_map<::zzz::core::Guid, ResourceRecord<GpuT>>& table,
			GpuResourceCallback<GpuT> onLoaded,
			RequestFunc&& requestFunc,
			CreateFunc&& createFunc)
		{
			std::unique_lock lock(m_Mutex);
			auto [it, inserted] = table.try_emplace(guid, [this](auto task) { m_MainThreadQueue.Push(std::move(task)); });
			it->second.readyEvent.Subscribe(std::move(onLoaded));
			if (!inserted)
			{
				return;
			}

			requestFunc(guid, weak_from_this(), [this, &table, guid, createFunc = std::forward<CreateFunc>(createFunc)](std::expected<std::shared_ptr<CpuT>, std::string> cpuRes)
			{
				if (!cpuRes)
				{
					std::unique_lock l(m_Mutex);
					auto it = table.find(guid);
					if (it != table.end())
					{
						it->second.readyEvent.Resolve(std::unexpected(cpuRes.error()));
					}
					return;
				}

				auto gpuObj = createFunc(*cpuRes);
				std::unique_lock l(m_Mutex);
				auto it = table.find(guid);
				if (it != table.end())
				{
					it->second.resource = gpuObj;
					it->second.readyEvent.Resolve(gpuObj);
				}
			});
		}

		void RequestCpuMesh(const ::zzz::core::Guid& guid, std::weak_ptr<void> ctx, std::function<void(std::expected<std::shared_ptr<CpuMesh>, std::string>)> cb);
		void RequestCpuMaterial(const ::zzz::core::Guid& guid, std::weak_ptr<void> ctx, std::function<void(std::expected<std::shared_ptr<CpuMaterial>, std::string>)> cb);
		void RequestCpuTexture(const ::zzz::core::Guid& guid, std::weak_ptr<void> ctx, std::function<void(std::expected<std::shared_ptr<CpuTexture2D>, std::string>)> cb);
		void RequestCpuShader(const ::zzz::core::Guid& guid, std::weak_ptr<void> ctx, std::function<void(std::expected<std::shared_ptr<CpuShader>, std::string>)> cb);

		std::shared_ptr<GAPI> m_GAPI;
		std::shared_ptr<CpuResourceManager> m_CpuManager;

		mutable std::shared_mutex m_Mutex;

		std::unordered_map<::zzz::core::Guid, ResourceRecord<GpuMesh>>      m_GpuMeshes;
		std::unordered_map<::zzz::core::Guid, ResourceRecord<GpuMaterial>>  m_GpuMaterials;
		std::unordered_map<::zzz::core::Guid, ResourceRecord<GpuTexture2D>> m_GpuTextures;
		std::unordered_map<::zzz::core::Guid, ResourceRecord<GpuShader>>    m_GpuShaders;

		::zzz::templates::CallbackQueue<> m_MainThreadQueue;
	};
}
