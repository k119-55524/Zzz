#pragma once

#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <shared_mutex>
#include <functional>
#include <expected>
#include <concepts>
#include <thread>
#include <condition_variable>
#include <atomic>

#include "core/utils/Guid.h"
#include "core/utils/Defines.h"
#include "core/enums/eResourceType.h"
#include "core/io/package/SceneData.h"
#include "core/io/package/DataAssetsManager.h"
#include "core/io/package/PackageEntry.h"
#include "core/io/FileSystem.h"
#include "core/templates/DoubleBufferedVector.h"
#include "core/templates/CallbackQueue.h"
#include "engine/resources/ResourceRecord.h"
#include "engine/resources/cpu/CpuMesh.h"
#include "engine/resources/cpu/CpuMaterial.h"
#include "engine/resources/cpu/CpuTexture2D.h"
#include "engine/resources/cpu/CpuShader.h"

namespace zzz::engine
{
	class PackageManager;

	template <typename T>
	using CpuResourceResult = std::expected<std::shared_ptr<T>, std::string>;

	template <typename T>
	using CpuResourceCallback = std::function<void(CpuResourceResult<T>)>;

	/**
	 * @class CpuResourceManager
	 * @brief Потокобезопасный центральный сервис загрузки и кэширования CPU-ресурсов (диск, VFS, пакеты).
	 * @details Полностью изолирован от GAPI и видеопамяти (Zero Platform Leaks).
	 */
	class CpuResourceManager final
	{
		Z_NO_COPY_MOVE(CpuResourceManager);

	public:
		enum class eCpuResourceKind : uint8_t
		{
			Mesh,
			Material,
			Texture,
			Shader
		};

		struct CpuLoadRequest
		{
			::zzz::core::Guid guid;
			eCpuResourceKind kind{ eCpuResourceKind::Mesh };
			std::string name;
		};

		CpuResourceManager() = delete;
		explicit CpuResourceManager(
			std::shared_ptr<PackageManager> packageManager,
			std::shared_ptr<::zzz::core::DataAssetsManager> dataAssetsManager = nullptr,
			std::shared_ptr<::zzz::core::FileSystem> fileSystem = nullptr);
		~CpuResourceManager();

		// --- Обновление и доставка колбэков на главном потоке ---
		void Update();

		// --- Запуск и остановка I/O-потока ---
		void Start();
		void Stop();

		// --- Барьер загрузки ---
		void Flush();

		// --- Загрузка структуры сцены (SceneData) ---
		[[nodiscard]] std::expected<::zzz::core::SceneData, std::string> LoadSceneData(const ::zzz::core::Guid& sceneGuid);
		[[nodiscard]] std::expected<::zzz::core::SceneData, std::string> LoadSceneData(std::string_view sceneName);

		// --- Доступ к подсистемам пакетов и бинарных ассетов ---
		[[nodiscard]] std::shared_ptr<PackageManager> GetPackageManager() const noexcept;
		[[nodiscard]] std::shared_ptr<::zzz::core::DataAssetsManager> GetDataAssetsManager() const noexcept;
		[[nodiscard]] std::shared_ptr<::zzz::core::FileSystem> GetFileSystem() const noexcept;

		// --- Загрузка ассетов данных из data.dat ---
		template <typename T>
		[[nodiscard]] std::expected<T, std::string> LoadDataAsset(const ::zzz::core::Guid& guid)
		{
			if (!m_DataAssetsManager)
			{
				return std::unexpected("DataAssetsManager не инициализирован в CpuResourceManager");
			}
			return m_DataAssetsManager->LoadAsset<T>(guid);
		}

		// --- Регистрация рантайм/процедурных ресурсов ---
		void AddMesh(std::shared_ptr<CpuMesh> mesh);
		void AddTexture(std::shared_ptr<CpuTexture2D> texture);
		void AddShader(std::shared_ptr<CpuShader> shader);
		void AddMaterial(std::shared_ptr<CpuMaterial> material);

		// --- Быстрый доступ к кэшу (по GUID) ---
		[[nodiscard]] std::shared_ptr<CpuMesh>      GetMesh(const ::zzz::core::Guid& guid) const;
		[[nodiscard]] std::shared_ptr<CpuTexture2D> GetTexture(const ::zzz::core::Guid& guid) const;
		[[nodiscard]] std::shared_ptr<CpuShader>    GetShader(const ::zzz::core::Guid& guid) const;
		[[nodiscard]] std::shared_ptr<CpuMaterial>  GetMaterial(const ::zzz::core::Guid& guid) const;

		// --- Быстрый доступ к кэшу (по имени) ---
		[[nodiscard]] std::shared_ptr<CpuMesh>      GetMesh(std::string_view name) const;
		[[nodiscard]] std::shared_ptr<CpuTexture2D> GetTexture(std::string_view name) const;
		[[nodiscard]] std::shared_ptr<CpuShader>    GetShader(std::string_view name) const;
		[[nodiscard]] std::shared_ptr<CpuMaterial>  GetMaterial(std::string_view name) const;

		// --- Шаблонный фасад доступа Get<T> ---
		template<typename T>
		[[nodiscard]] std::shared_ptr<T> Get(const ::zzz::core::Guid& guid) const
		{
			if constexpr (std::is_same_v<T, CpuMesh>)           return GetMesh(guid);
			else if constexpr (std::is_same_v<T, CpuTexture2D>) return GetTexture(guid);
			else if constexpr (std::is_same_v<T, CpuShader>)    return GetShader(guid);
			else if constexpr (std::is_same_v<T, CpuMaterial>)  return GetMaterial(guid);
			else static_assert(sizeof(T) == 0, "Неподдерживаемый тип ресурса для Get<T>");
		}

		template<typename T>
		[[nodiscard]] std::shared_ptr<T> Get(std::string_view name) const
		{
			if constexpr (std::is_same_v<T, CpuMesh>)           return GetMesh(name);
			else if constexpr (std::is_same_v<T, CpuTexture2D>) return GetTexture(name);
			else if constexpr (std::is_same_v<T, CpuShader>)    return GetShader(name);
			else if constexpr (std::is_same_v<T, CpuMaterial>)  return GetMaterial(name);
			else static_assert(sizeof(T) == 0, "Неподдерживаемый тип ресурса для Get<T>");
		}

		// --- Проверка наличия в кэше ---
		[[nodiscard]] bool HasMesh(const ::zzz::core::Guid& guid) const noexcept;
		[[nodiscard]] bool HasTexture(const ::zzz::core::Guid& guid) const noexcept;
		[[nodiscard]] bool HasShader(const ::zzz::core::Guid& guid) const noexcept;
		[[nodiscard]] bool HasMaterial(const ::zzz::core::Guid& guid) const noexcept;

		// --- Строго типизированные методы асинхронной загрузки ---
		template<typename ContextType>
		void LoadMeshAsync(
			const ::zzz::core::Guid& guid,
			std::weak_ptr<ContextType> context,
			CpuResourceCallback<CpuMesh> onLoaded)
		{
			EnqueueTypedRequest<CpuMesh>(guid, eCpuResourceKind::Mesh, m_CpuMeshes, std::move(context), std::move(onLoaded));
		}

		void LoadMeshAsync(
			const ::zzz::core::Guid& guid,
			CpuResourceCallback<CpuMesh> onLoaded)
		{
			LoadMeshAsync<void>(guid, {}, std::move(onLoaded));
		}

		template<typename ContextType>
		void LoadMaterialAsync(
			const ::zzz::core::Guid& guid,
			std::weak_ptr<ContextType> context,
			CpuResourceCallback<CpuMaterial> onLoaded)
		{
			EnqueueTypedRequest<CpuMaterial>(guid, eCpuResourceKind::Material, m_CpuMaterials, std::move(context), std::move(onLoaded));
		}

		void LoadMaterialAsync(
			const ::zzz::core::Guid& guid,
			CpuResourceCallback<CpuMaterial> onLoaded)
		{
			LoadMaterialAsync<void>(guid, {}, std::move(onLoaded));
		}

		template<typename ContextType>
		void LoadTextureAsync(
			const ::zzz::core::Guid& guid,
			std::weak_ptr<ContextType> context,
			CpuResourceCallback<CpuTexture2D> onLoaded)
		{
			EnqueueTypedRequest<CpuTexture2D>(guid, eCpuResourceKind::Texture, m_CpuTextures, std::move(context), std::move(onLoaded));
		}

		void LoadTextureAsync(
			const ::zzz::core::Guid& guid,
			CpuResourceCallback<CpuTexture2D> onLoaded)
		{
			LoadTextureAsync<void>(guid, {}, std::move(onLoaded));
		}

		template<typename ContextType>
		void LoadShaderAsync(
			const ::zzz::core::Guid& guid,
			std::weak_ptr<ContextType> context,
			CpuResourceCallback<CpuShader> onLoaded)
		{
			EnqueueTypedRequest<CpuShader>(guid, eCpuResourceKind::Shader, m_CpuShaders, std::move(context), std::move(onLoaded));
		}

		void LoadShaderAsync(
			const ::zzz::core::Guid& guid,
			CpuResourceCallback<CpuShader> onLoaded)
		{
			LoadShaderAsync<void>(guid, {}, std::move(onLoaded));
		}

		// --- Очистка и выгрузка ---
		void UnloadSceneResources();
		void UnloadMeshes();
		void UnloadTextures();
		void UnloadMaterials();
		void UnloadShaders();
		void UnloadAll();

		[[nodiscard]] size_t GetLoadedMeshCount() const noexcept;
		[[nodiscard]] size_t GetLoadedTextureCount() const noexcept;
		[[nodiscard]] size_t GetLoadedShaderCount() const noexcept;
		[[nodiscard]] size_t GetLoadedMaterialCount() const noexcept;

	private:
		template<typename T, typename ContextType>
		void EnqueueTypedRequest(
			const ::zzz::core::Guid& guid,
			eCpuResourceKind kind,
			std::unordered_map<::zzz::core::Guid, ResourceRecord<T>>& table,
			std::weak_ptr<ContextType> context,
			CpuResourceCallback<T> onLoaded)
		{
			std::unique_lock lock(m_TablesMutex);
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

			m_ActiveRequests.fetch_add(1, std::memory_order_relaxed);
			m_RequestQueue.Push(CpuLoadRequest{ guid, kind, {} });
			m_IoCv.notify_one();
		}

		[[nodiscard]] std::expected<::zzz::core::PackageEntry, std::string> FindEntry(
			const ::zzz::core::Guid& guid,
			::zzz::core::eResourceType type) const;

		void IoWorkerLoop(std::stop_token stopToken);

		std::shared_ptr<PackageManager> m_PackageManager;
		std::shared_ptr<::zzz::core::DataAssetsManager> m_DataAssetsManager;
		std::shared_ptr<::zzz::core::FileSystem> m_FileSystem;

		mutable std::shared_mutex m_TablesMutex;

		std::unordered_map<::zzz::core::Guid, ResourceRecord<CpuMesh>>      m_CpuMeshes;
		std::unordered_map<::zzz::core::Guid, ResourceRecord<CpuTexture2D>> m_CpuTextures;
		std::unordered_map<::zzz::core::Guid, ResourceRecord<CpuShader>>    m_CpuShaders;
		std::unordered_map<::zzz::core::Guid, ResourceRecord<CpuMaterial>>  m_CpuMaterials;

		std::unordered_map<std::string, ::zzz::core::Guid> m_MeshNames;
		std::unordered_map<std::string, ::zzz::core::Guid> m_TextureNames;
		std::unordered_map<std::string, ::zzz::core::Guid> m_ShaderNames;
		std::unordered_map<std::string, ::zzz::core::Guid> m_MaterialNames;

		::zzz::templates::CallbackQueue<> m_MainThreadQueue;

		::zzz::core::DoubleBufferedVector<CpuLoadRequest> m_RequestQueue;
		std::jthread m_IoThread;
		std::mutex m_IoMutex;
		std::condition_variable_any m_IoCv;

		std::atomic<size_t> m_ActiveRequests{ 0 };
		std::mutex m_FlushMutex;
		std::condition_variable m_FlushCv;
	};
}
