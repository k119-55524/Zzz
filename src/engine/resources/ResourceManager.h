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
#include <future>

#include "core/utils/Guid.h"
#include "core/utils/Defines.h"
#include "core/enums/eResourceType.h"
#include "core/resources/IResource.h"
#include "core/io/package/SceneData.h"
#include "core/io/package/DataAssetsManager.h"
#include "core/io/FileSystem.h"
#include "core/templates/DoubleBufferedVector.h"
#include "core/templates/CallbackQueue.h"
#include "engine/resources/IResourceLoader.h"
#include "engine/gapi/GAPI.h"
namespace zzz::engine
{
	class PackageManager;

	// Предварительные объявления конкретных типов ресурсов
	class Mesh;
	class Texture2D;
	class Shader;
	class Material;

	template <typename T>
	using ResourceResult = std::expected<std::shared_ptr<T>, std::string>;

	template <typename T>
	using ResourceCallback = std::function<void(ResourceResult<T>)>;

	using AnyResourceResult = std::expected<std::shared_ptr<::zzz::core::IResource>, std::string>;
	using AnyResourceCallback = std::function<void(AnyResourceResult)>;

	/**
	 * @class ResourceManager
	 * @brief Потокобезопасный центральный сервис кэширования и асинхронной загрузки ресурсов движка.
	 */
	class ResourceManager final
	{
		Z_NO_COPY_MOVE(ResourceManager);

	public:
		struct ResourceLoadRequest
		{
			::zzz::core::Guid guid;
			::zzz::core::eResourceType type{ ::zzz::core::eResourceType::Unknown };
			std::string name;
			AnyResourceCallback onLoaded;
		};

		ResourceManager() = delete;
		explicit ResourceManager(
			std::shared_ptr<PackageManager> packageManager,
			std::shared_ptr<::zzz::core::DataAssetsManager> dataAssetsManager = nullptr,
			std::shared_ptr<::zzz::core::FileSystem> fileSystem = nullptr,
			std::shared_ptr<GAPI> gapi = nullptr);
		~ResourceManager();

		// --- Обновление и доставка колбэков на главном потоке ---
		void Update();

		// --- Запуск и остановка I/O-потока ---
		void Start();
		void Stop();

		// --- Загрузка структуры сцены (SceneData) ---
		[[nodiscard]] std::expected<::zzz::core::SceneData, std::string> LoadSceneData(const ::zzz::core::Guid& sceneGuid);
		[[nodiscard]] std::expected<::zzz::core::SceneData, std::string> LoadSceneData(std::string_view sceneName);

		// --- Доступ к подсистемам пакетов и бинарных ассетов ---
		[[nodiscard]] std::shared_ptr<PackageManager> GetPackageManager() const noexcept { return m_PackageManager; }
		[[nodiscard]] std::shared_ptr<::zzz::core::DataAssetsManager> GetDataAssetsManager() const noexcept { return m_DataAssetsManager; }
		[[nodiscard]] std::shared_ptr<::zzz::core::FileSystem> GetFileSystem() const noexcept { return m_FileSystem; }
		[[nodiscard]] std::shared_ptr<GAPI> GetGAPI() const noexcept { return m_GAPI; }

		// --- Загрузка ассетов данных из data.dat ---
		template <typename T>
		[[nodiscard]] std::expected<T, std::string> LoadDataAsset(const ::zzz::core::Guid& guid)
		{
			if (!m_DataAssetsManager)
			{
				return std::unexpected("DataAssetsManager не инициализирован в ResourceManager");
			}
			return m_DataAssetsManager->LoadAsset<T>(guid);
		}

		// --- Регистрация загрузчиков (OCP) ---
		void RegisterLoader(std::unique_ptr<IResourceLoader> loader);

		template<typename LoaderT, typename... Args>
			requires std::derived_from<LoaderT, IResourceLoader>
		void RegisterLoader(Args&&... args)
		{
			RegisterLoader(std::make_unique<LoaderT>(std::forward<Args>(args)...));
		}

		[[nodiscard]] bool HasLoader(::zzz::core::eResourceType type) const noexcept;

		// --- Регистрация процедурных / рантайм ресурсов ---
		void AddMesh(std::shared_ptr<Mesh> mesh);
		void AddTexture(std::shared_ptr<Texture2D> texture);
		void AddShader(std::shared_ptr<Shader> shader);
		void AddMaterial(std::shared_ptr<Material> material);

		// --- Быстрый неблокирующий доступ к кэшу (по GUID) ---
		[[nodiscard]] std::shared_ptr<Mesh>      GetMesh(const ::zzz::core::Guid& guid) const;
		[[nodiscard]] std::shared_ptr<Texture2D> GetTexture(const ::zzz::core::Guid& guid) const;
		[[nodiscard]] std::shared_ptr<Shader>    GetShader(const ::zzz::core::Guid& guid) const;
		[[nodiscard]] std::shared_ptr<Material>  GetMaterial(const ::zzz::core::Guid& guid) const;

		// --- Быстрый неблокирующий доступ к кэшу (по имени) ---
		[[nodiscard]] std::shared_ptr<Mesh>      GetMesh(std::string_view name) const;
		[[nodiscard]] std::shared_ptr<Texture2D> GetTexture(std::string_view name) const;
		[[nodiscard]] std::shared_ptr<Shader>    GetShader(std::string_view name) const;
		[[nodiscard]] std::shared_ptr<Material>  GetMaterial(std::string_view name) const;

		// --- Шаблонный фасад доступа Get<T> ---
		template<typename T>
		[[nodiscard]] std::shared_ptr<T> Get(const ::zzz::core::Guid& guid) const
		{
			if constexpr (std::is_same_v<T, Mesh>)           return GetMesh(guid);
			else if constexpr (std::is_same_v<T, Texture2D>) return GetTexture(guid);
			else if constexpr (std::is_same_v<T, Shader>)    return GetShader(guid);
			else if constexpr (std::is_same_v<T, Material>)  return GetMaterial(guid);
			else static_assert(sizeof(T) == 0, "Неподдерживаемый тип ресурса для Get<T>");
		}

		template<typename T>
		[[nodiscard]] std::shared_ptr<T> Get(std::string_view name) const
		{
			if constexpr (std::is_same_v<T, Mesh>)           return GetMesh(name);
			else if constexpr (std::is_same_v<T, Texture2D>) return GetTexture(name);
			else if constexpr (std::is_same_v<T, Shader>)    return GetShader(name);
			else if constexpr (std::is_same_v<T, Material>)  return GetMaterial(name);
			else static_assert(sizeof(T) == 0, "Неподдерживаемый тип ресурса для Get<T>");
		}

		// --- Проверка наличия в кэше ---
		[[nodiscard]] bool HasMesh(const ::zzz::core::Guid& guid) const noexcept;
		[[nodiscard]] bool HasTexture(const ::zzz::core::Guid& guid) const noexcept;
		[[nodiscard]] bool HasShader(const ::zzz::core::Guid& guid) const noexcept;
		[[nodiscard]] bool HasMaterial(const ::zzz::core::Guid& guid) const noexcept;

		using OwnerToken = std::weak_ptr<const void>;

		[[nodiscard]] static bool IsOwnerAlive(const OwnerToken& token) noexcept
		{
			if (!token.owner_before(OwnerToken{}) && !OwnerToken{}.owner_before(token))
			{
				return true;
			}
			return !token.expired();
		}

		// --- Асинхронная загрузка с диска через пинг-понг очередь ---
		template<typename T>
		void LoadAsync(const ::zzz::core::Guid& guid, ResourceCallback<T> onCompleted, OwnerToken ownerToken = {})
		{
			if (auto cached = Get<T>(guid))
			{
				if (onCompleted)
				{
					m_MainThreadQueue.Push([onCompleted = std::move(onCompleted), cached = std::move(cached), ownerToken = std::move(ownerToken)]() mutable {
						if (IsOwnerAlive(ownerToken))
						{
							onCompleted(cached);
						}
					});
				}
				return;
			}

			EnqueueLoadRequest(guid, GetTypeFor<T>(), {}, [onCompleted = std::move(onCompleted), ownerToken = std::move(ownerToken)](AnyResourceResult res) {
				if (!onCompleted) return;
				if (!IsOwnerAlive(ownerToken)) return;

				if (!res)
				{
					onCompleted(std::unexpected(res.error()));
				}
				else
				{
					onCompleted(std::static_pointer_cast<T>(*res));
				}
			});
		}

		template<typename T>
		void LoadAsync(const ::zzz::core::Guid& guid, std::function<void(std::shared_ptr<T>)> onLoaded, OwnerToken ownerToken = {})
		{
			LoadAsync<T>(guid, [onLoaded = std::move(onLoaded)](ResourceResult<T> res) {
				if (!onLoaded) return;
				if (res)
					onLoaded(*res);
				else
					onLoaded(nullptr);
			}, std::move(ownerToken));
		}

		// --- Специализированная асинхронная загрузка Mesh ---
		void LoadMeshAsync(
			const ::zzz::core::Guid& meshGuid,
			std::function<void(std::expected<::zzz::core::Guid, std::string>)> onCompleted,
			OwnerToken ownerToken = {});

		void LoadMeshAsync(
			std::span<const ::zzz::core::Guid> meshGuids,
			std::function<void(std::expected<::zzz::core::Guid, std::string>)> onCompleted,
			OwnerToken ownerToken = {});

		// --- Синхронизация и барьеры загрузки ---
		void Flush();

		template<typename T>
		[[nodiscard]] std::shared_ptr<T> LoadSync(const ::zzz::core::Guid& guid)
		{
			if (auto cached = Get<T>(guid))
				return cached;

			LoadAsync<T>(guid, ResourceCallback<T>{});
			Flush();
			return Get<T>(guid);
		}

		// --- Раздельное управление жизненным циклом и выгрузкой памяти ---
		void UnloadSceneResources();
		void UnloadUnused(std::function<bool()> shouldStop = nullptr);

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
		template<typename T>
		static constexpr ::zzz::core::eResourceType GetTypeFor() noexcept
		{
			if constexpr (std::is_same_v<T, Mesh>)           return ::zzz::core::eResourceType::Mesh;
			else if constexpr (std::is_same_v<T, Texture2D>) return ::zzz::core::eResourceType::Texture2D;
			else if constexpr (std::is_same_v<T, Shader>)    return ::zzz::core::eResourceType::Shader;
			else if constexpr (std::is_same_v<T, Material>)  return ::zzz::core::eResourceType::Material;
			else return ::zzz::core::eResourceType::Unknown;
		}

		void EnqueueLoadRequest(
			const ::zzz::core::Guid& guid,
			::zzz::core::eResourceType type,
			std::string name,
			AnyResourceCallback onLoaded);

		void IoWorkerLoop(std::stop_token stopToken);

		std::shared_ptr<PackageManager> m_PackageManager;
		std::shared_ptr<::zzz::core::DataAssetsManager> m_DataAssetsManager;
		std::shared_ptr<::zzz::core::FileSystem> m_FileSystem;
		std::shared_ptr<GAPI> m_GAPI;

		mutable std::shared_mutex m_Mutex;

		// Отдельные типизированные таблицы ресурсов (кэш видеопамяти)
		std::unordered_map<::zzz::core::Guid, std::shared_ptr<Mesh>>      m_Meshes;
		std::unordered_map<::zzz::core::Guid, std::shared_ptr<Texture2D>> m_Textures;
		std::unordered_map<::zzz::core::Guid, std::shared_ptr<Shader>>    m_Shaders;
		std::unordered_map<::zzz::core::Guid, std::shared_ptr<Material>>  m_Materials;

		// Вторичные индексы по строковым именам
		std::unordered_map<std::string, ::zzz::core::Guid> m_MeshNames;
		std::unordered_map<std::string, ::zzz::core::Guid> m_TextureNames;
		std::unordered_map<std::string, ::zzz::core::Guid> m_ShaderNames;
		std::unordered_map<std::string, ::zzz::core::Guid> m_MaterialNames;

		// Реестр загрузчиков
		std::unordered_map<::zzz::core::eResourceType, std::unique_ptr<IResourceLoader>> m_Loaders;

		// Реестр in-flight запросов для дедупликации конкурентной загрузки одного GUID
		std::unordered_map<::zzz::core::Guid, std::vector<AnyResourceCallback>> m_InFlightCallbacks;

		// Очередь для передачи колбэков на главный поток
		::zzz::templates::CallbackQueue<> m_MainThreadQueue;

		// Публикация загруженного ресурса в типизированную таблицу кэша
		void PublishResource(const ::zzz::core::Guid& guid, const std::shared_ptr<::zzz::core::IResource>& resource);

		[[nodiscard]] std::expected<std::shared_ptr<Mesh>, std::string> CombineSubmeshes(
			const ::zzz::core::Guid& resultGuid,
			std::span<const std::shared_ptr<Mesh>> submeshes);

		// Выделенный I/O-поток и пинг-понг очередь
		::zzz::core::DoubleBufferedVector<ResourceLoadRequest> m_RequestQueue;
		std::jthread m_IoThread;
		std::mutex m_IoMutex;
		std::condition_variable_any m_IoCv;

		std::atomic<size_t> m_ActiveRequests{ 0 };
		std::mutex m_FlushMutex;
		std::condition_variable m_FlushCv;
	};
}
