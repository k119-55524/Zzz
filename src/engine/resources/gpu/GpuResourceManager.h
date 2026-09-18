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
		template<typename ContextType>
		void LoadGpuMeshAsync(
			const ::zzz::core::Guid& guid,
			std::weak_ptr<ContextType> context,
			GpuResourceCallback<GpuMesh> onLoaded)
		{
			LoadGpuResourceInternal<GpuMesh, CpuMesh>(
				guid,
				m_GpuMeshes,
				std::move(context),
				std::move(onLoaded),
				[this](const ::zzz::core::Guid& g, auto ctx, auto cb) {
					RequestCpuMesh(g, std::move(ctx), std::move(cb));
				},
				[this](std::shared_ptr<CpuMesh> cpuRes) {
					return CreateGpuMesh(std::move(cpuRes));
				});
		}

		void LoadGpuMeshAsync(
			const ::zzz::core::Guid& guid,
			GpuResourceCallback<GpuMesh> onLoaded)
		{
			LoadGpuMeshAsync<void>(guid, {}, std::move(onLoaded));
		}

		template<typename ContextType>
		void LoadGpuMaterialAsync(
			const ::zzz::core::Guid& guid,
			std::weak_ptr<ContextType> context,
			GpuResourceCallback<GpuMaterial> onLoaded)
		{
			LoadGpuResourceInternal<GpuMaterial, CpuMaterial>(
				guid,
				m_GpuMaterials,
				std::move(context),
				std::move(onLoaded),
				[this](const ::zzz::core::Guid& g, auto ctx, auto cb) {
					RequestCpuMaterial(g, std::move(ctx), std::move(cb));
				},
				[this](std::shared_ptr<CpuMaterial> cpuRes) {
					return CreateGpuMaterial(std::move(cpuRes));
				});
		}

		void LoadGpuMaterialAsync(
			const ::zzz::core::Guid& guid,
			GpuResourceCallback<GpuMaterial> onLoaded)
		{
			LoadGpuMaterialAsync<void>(guid, {}, std::move(onLoaded));
		}

		template<typename ContextType>
		void LoadGpuTextureAsync(
			const ::zzz::core::Guid& guid,
			std::weak_ptr<ContextType> context,
			GpuResourceCallback<GpuTexture2D> onLoaded)
		{
			LoadGpuResourceInternal<GpuTexture2D, CpuTexture2D>(
				guid,
				m_GpuTextures,
				std::move(context),
				std::move(onLoaded),
				[this](const ::zzz::core::Guid& g, auto ctx, auto cb) {
					RequestCpuTexture(g, std::move(ctx), std::move(cb));
				},
				[this](std::shared_ptr<CpuTexture2D> cpuRes) {
					return CreateGpuTexture(std::move(cpuRes));
				});
		}

		void LoadGpuTextureAsync(
			const ::zzz::core::Guid& guid,
			GpuResourceCallback<GpuTexture2D> onLoaded)
		{
			LoadGpuTextureAsync<void>(guid, {}, std::move(onLoaded));
		}

		template<typename ContextType>
		void LoadGpuShaderAsync(
			const ::zzz::core::Guid& guid,
			std::weak_ptr<ContextType> context,
			GpuResourceCallback<GpuShader> onLoaded)
		{
			LoadGpuResourceInternal<GpuShader, CpuShader>(
				guid,
				m_GpuShaders,
				std::move(context),
				std::move(onLoaded),
				[this](const ::zzz::core::Guid& g, auto ctx, auto cb) {
					RequestCpuShader(g, std::move(ctx), std::move(cb));
				},
				[this](std::shared_ptr<CpuShader> cpuRes) {
					return CreateGpuShader(std::move(cpuRes));
				});
		}

		void LoadGpuShaderAsync(
			const ::zzz::core::Guid& guid,
			GpuResourceCallback<GpuShader> onLoaded)
		{
			LoadGpuShaderAsync<void>(guid, {}, std::move(onLoaded));
		}

		// --- Быстрый доступ к кэшу GPU ---
		[[nodiscard]] std::shared_ptr<GpuMesh>      GetGpuMesh(const ::zzz::core::Guid& guid) const;
		[[nodiscard]] std::shared_ptr<GpuMaterial>  GetGpuMaterial(const ::zzz::core::Guid& guid) const;
		[[nodiscard]] std::shared_ptr<GpuTexture2D> GetGpuTexture(const ::zzz::core::Guid& guid) const;
		[[nodiscard]] std::shared_ptr<GpuShader>    GetGpuShader(const ::zzz::core::Guid& guid) const;

		[[nodiscard]] bool HasGpuMesh(const ::zzz::core::Guid& guid) const noexcept;
		[[nodiscard]] bool HasGpuMaterial(const ::zzz::core::Guid& guid) const noexcept;
		[[nodiscard]] bool HasGpuTexture(const ::zzz::core::Guid& guid) const noexcept;
		[[nodiscard]] bool HasGpuShader(const ::zzz::core::Guid& guid) const noexcept;

		void UnloadAll();

	private:
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
			auto it = table.find(guid);
			if (it != table.end())
			{
				it->second.readyEvent.SetDispatcher([this](auto task) { m_MainThreadQueue.Push(std::move(task)); });
				it->second.readyEvent.Subscribe(std::move(context), std::move(onLoaded));
				return;
			}

			auto& record = table[guid];
			record.readyEvent.SetDispatcher([this](auto task) { m_MainThreadQueue.Push(std::move(task)); });
			record.readyEvent.Subscribe(std::move(context), std::move(onLoaded));

			requestFunc(guid, weak_from_this(), [this, &table, guid, createFunc = std::forward<CreateFunc>(createFunc)](std::expected<std::shared_ptr<CpuT>, std::string> cpuRes)
			{
				if (!cpuRes)
				{
					std::unique_lock l(m_Mutex);
					table[guid].readyEvent.Resolve(std::unexpected(cpuRes.error()));
					return;
				}

				auto gpuObj = createFunc(*cpuRes);
				std::unique_lock l(m_Mutex);
				table[guid].resource = gpuObj;
				table[guid].readyEvent.Resolve(gpuObj);
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
