#include "ResourceManager.h"
#include "Mesh.h"
#include "Material.h"
#include "Shader.h"
#include "engine/package/PackageManager.h"
#include "core/io/package/DataAssetsManager.h"
#include "core/io/package/MeshData.h"
#include "core/io/ResourceStorageTraits.h"
#include "core/utils/Ensure.h"
#include "core/utils/MemoryUtils.h"
Z_SET_LOG_CATEGORY(::zzz::core::LogEngine);

using namespace zzz::core;
using namespace zzz::templates;

namespace zzz::engine
{
	ResourceManager::ResourceManager(
		std::shared_ptr<PackageManager> packageManager,
		std::shared_ptr<DataAssetsManager> dataAssetsManager,
		std::shared_ptr<FileSystem> fileSystem,
		std::shared_ptr<GAPI> gapi)
		: m_PackageManager(std::move(packageManager))
		, m_DataAssetsManager(std::move(dataAssetsManager))
		, m_FileSystem(std::move(fileSystem))
		, m_GAPI(std::move(gapi))
	{
		ensure(m_PackageManager != nullptr, "PackageManager не должен быть null в ResourceManager.");
	}

	ResourceManager::~ResourceManager()
	{
		Stop();
	}

	void ResourceManager::Start()
	{
		if (m_IoThread.joinable())
			return;

		m_IoThread = std::jthread([this](std::stop_token st) {
			IoWorkerLoop(std::move(st));
		});
	}

	void ResourceManager::Stop()
	{
		if (m_IoThread.joinable())
		{
			m_IoThread.request_stop();
			m_IoCv.notify_all();
			m_IoThread.join();
		}
	}

	std::expected<SceneData, std::string> ResourceManager::LoadSceneData(const Guid& sceneGuid)
	{
		std::shared_lock lock(m_Mutex);
		auto entryOpt = m_PackageManager->GetEntry(ePackage::Scene, sceneGuid);
		if (!entryOpt.has_value())
		{
			return std::unexpected(std::format("Сцена с GUID '{}' не найдена в package.dat.", sceneGuid.ToString()));
		}

		auto sceneDataRes = m_PackageManager->LoadAsset<SceneData>(sceneGuid);
		if (!sceneDataRes.has_value())
		{
			return std::unexpected(std::format("Ошибка загрузки данных сцены '{}' ({}): {}",
				entryOpt->GetName(), sceneGuid.ToString(), sceneDataRes.error()));
		}

		return std::move(*sceneDataRes);
	}

	std::expected<SceneData, std::string> ResourceManager::LoadSceneData(std::string_view sceneName)
	{
		std::shared_lock lock(m_Mutex);
		auto entryOpt = m_PackageManager->GetEntry(ePackage::Scene, sceneName);
		if (!entryOpt.has_value())
		{
			return std::unexpected(std::format("Сцена с именем '{}' не найдена в package.dat.", sceneName));
		}

		return LoadSceneData(entryOpt->GetGuid());
	}

	void ResourceManager::RegisterLoader(std::unique_ptr<IResourceLoader> loader)
	{
		if (!loader) return;
		std::unique_lock lock(m_Mutex);
		m_Loaders[loader->GetSupportedType()] = std::move(loader);
	}

	bool ResourceManager::HasLoader(eResourceType type) const noexcept
	{
		std::shared_lock lock(m_Mutex);
		return m_Loaders.contains(type);
	}

	void ResourceManager::Update()
	{
		m_MainThreadQueue.ExecuteAll();
	}

	void ResourceManager::AddMesh(std::shared_ptr<Mesh> mesh)
	{
		if (!mesh) return;
		std::unique_lock lock(m_Mutex);
		m_Meshes[mesh->GetGuid()] = mesh;
		if (!mesh->GetName().empty())
		{
			m_MeshNames[std::string(mesh->GetName())] = mesh->GetGuid();
		}
	}

	void ResourceManager::AddTexture(std::shared_ptr<Texture2D> texture)
	{
		if (!texture) return;
		// Метод-заглушка: будет наполняться при реализации класса Texture2D на шаге 12
	}

	void ResourceManager::AddShader(std::shared_ptr<Shader> shader)
	{
		if (!shader) return;
		// Метод-заглушка: будет наполняться при реализации класса Shader на шаге 13
	}

	void ResourceManager::AddMaterial(std::shared_ptr<Material> material)
	{
		if (!material) return;
		// Метод-заглушка: будет наполняться при реализации класса Material на шаге 14
	}

	std::shared_ptr<Mesh> ResourceManager::GetMesh(const Guid& guid) const
	{
		std::shared_lock lock(m_Mutex);
		auto it = m_Meshes.find(guid);
		return (it != m_Meshes.end()) ? it->second : nullptr;
	}

	std::shared_ptr<Texture2D> ResourceManager::GetTexture(const Guid& guid) const
	{
		std::shared_lock lock(m_Mutex);
		auto it = m_Textures.find(guid);
		return (it != m_Textures.end()) ? it->second : nullptr;
	}

	std::shared_ptr<Shader> ResourceManager::GetShader(const Guid& guid) const
	{
		std::shared_lock lock(m_Mutex);
		auto it = m_Shaders.find(guid);
		return (it != m_Shaders.end()) ? it->second : nullptr;
	}

	std::shared_ptr<Material> ResourceManager::GetMaterial(const Guid& guid) const
	{
		std::shared_lock lock(m_Mutex);
		auto it = m_Materials.find(guid);
		return (it != m_Materials.end()) ? it->second : nullptr;
	}

	std::shared_ptr<Mesh> ResourceManager::GetMesh(std::string_view name) const
	{
		std::shared_lock lock(m_Mutex);
		auto itName = m_MeshNames.find(std::string(name));
		if (itName == m_MeshNames.end()) return nullptr;
		auto it = m_Meshes.find(itName->second);
		return (it != m_Meshes.end()) ? it->second : nullptr;
	}

	std::shared_ptr<Texture2D> ResourceManager::GetTexture(std::string_view name) const
	{
		std::shared_lock lock(m_Mutex);
		auto itName = m_TextureNames.find(std::string(name));
		if (itName == m_TextureNames.end()) return nullptr;
		auto it = m_Textures.find(itName->second);
		return (it != m_Textures.end()) ? it->second : nullptr;
	}

	std::shared_ptr<Shader> ResourceManager::GetShader(std::string_view name) const
	{
		std::shared_lock lock(m_Mutex);
		auto itName = m_ShaderNames.find(std::string(name));
		if (itName == m_ShaderNames.end()) return nullptr;
		auto it = m_Shaders.find(itName->second);
		return (it != m_Shaders.end()) ? it->second : nullptr;
	}

	std::shared_ptr<Material> ResourceManager::GetMaterial(std::string_view name) const
	{
		std::shared_lock lock(m_Mutex);
		auto itName = m_MaterialNames.find(std::string(name));
		if (itName == m_MaterialNames.end()) return nullptr;
		auto it = m_Materials.find(itName->second);
		return (it != m_Materials.end()) ? it->second : nullptr;
	}

	bool ResourceManager::HasMesh(const Guid& guid) const noexcept
	{
		std::shared_lock lock(m_Mutex);
		return m_Meshes.contains(guid);
	}

	bool ResourceManager::HasTexture(const Guid& guid) const noexcept
	{
		std::shared_lock lock(m_Mutex);
		return m_Textures.contains(guid);
	}

	bool ResourceManager::HasShader(const Guid& guid) const noexcept
	{
		std::shared_lock lock(m_Mutex);
		return m_Shaders.contains(guid);
	}

	bool ResourceManager::HasMaterial(const Guid& guid) const noexcept
	{
		std::shared_lock lock(m_Mutex);
		return m_Materials.contains(guid);
	}

	void ResourceManager::EnqueueLoadRequest(
		const Guid& guid,
		eResourceType type,
		std::string name,
		AnyResourceCallback onLoaded)
	{
		bool isFirstRequest = false;
		{
			std::unique_lock lock(m_Mutex);
			auto it = m_InFlightCallbacks.find(guid);
			if (it != m_InFlightCallbacks.end())
			{
				if (onLoaded)
				{
					it->second.push_back(std::move(onLoaded));
				}
				return;
			}

			isFirstRequest = true;
			if (onLoaded)
			{
				m_InFlightCallbacks[guid].push_back(std::move(onLoaded));
			}
			else
			{
				m_InFlightCallbacks[guid] = {};
			}
		}

		if (isFirstRequest)
		{
			m_ActiveRequests.fetch_add(1, std::memory_order_release);
			m_RequestQueue.Emplace(ResourceLoadRequest{
				.guid = guid,
				.type = type,
				.name = std::move(name),
				.onLoaded = nullptr
			});
			m_IoCv.notify_one();
		}
	}

	void ResourceManager::Flush()
	{
		std::unique_lock lock(m_FlushMutex);
		m_FlushCv.wait(lock, [this]() {
			return m_ActiveRequests.load(std::memory_order_acquire) == 0;
		});
		lock.unlock();

		m_MainThreadQueue.ExecuteAll();
	}

	void ResourceManager::PublishResource(const Guid& guid, const std::shared_ptr<IResource>& resource)
	{
		if (!resource) return;

		std::unique_lock lock(m_Mutex);
		switch (resource->GetResourceType())
		{
		case eResourceType::Mesh:
		{
			auto mesh = std::static_pointer_cast<Mesh>(resource);
			m_Meshes[guid] = mesh;
			if (!mesh->GetName().empty())
			{
				m_MeshNames[std::string(mesh->GetName())] = guid;
			}
			break;
		}
		case eResourceType::Material:
		{
			auto mat = std::static_pointer_cast<Material>(resource);
			m_Materials[guid] = mat;
			if (!mat->GetName().empty())
			{
				m_MaterialNames[std::string(mat->GetName())] = guid;
			}
			break;
		}
		case eResourceType::Shader:
		{
			auto shader = std::static_pointer_cast<Shader>(resource);
			m_Shaders[guid] = shader;
			if (!shader->GetName().empty())
			{
				m_ShaderNames[std::string(shader->GetName())] = guid;
			}
			break;
		}
		default:
			// Ресурсы Texture2D подключаются на этапе 18
			break;
		}
	}

	void ResourceManager::IoWorkerLoop(std::stop_token stopToken)
	{
		while (!stopToken.stop_requested())
		{
			{
				std::unique_lock lock(m_IoMutex);
				m_IoCv.wait(lock, stopToken, [this]() {
					return m_RequestQueue.HasPendingWrites();
				});
			}

			if (stopToken.stop_requested())
				break;

			auto& batch = m_RequestQueue.SwapAndGetReadBuffer();
			for (auto& req : batch)
			{
				AnyResourceResult result = std::unexpected("Неизвестная ошибка загрузки ресурса");

				IResourceLoader* loader = nullptr;
				{
					std::shared_lock lock(m_Mutex);
					auto loaderIt = m_Loaders.find(req.type);
					if (loaderIt != m_Loaders.end())
					{
						loader = loaderIt->second.get();
					}
				}

				if (!loader)
				{
					result = std::unexpected(std::format("Не найден загрузчик для ресурса типа {}", ToString(req.type)));
					DOutError("[ResourceManager::IoWorkerLoop] {}", result.error());
				}
				else if (!m_PackageManager || !m_DataAssetsManager || !m_FileSystem || !m_GAPI)
				{
					result = std::unexpected(std::format("Подсистемы движка не инициализированы для загрузки ресурса {}", req.guid.ToString()));
					DOutError("[ResourceManager::IoWorkerLoop] {}", result.error());
				}
				else
				{
					const auto storageKind = GetResourceStorageTraits(req.type).storageKind;
					std::optional<PackageEntry> entryOpt;

					if (storageKind == eResourceStorageKind::DataArchive)
					{
						entryOpt = m_DataAssetsManager->GetEntry(req.type, req.guid);
					}
					else if (storageKind == eResourceStorageKind::PackageArchive)
					{
						entryOpt = m_PackageManager->GetEntry(req.guid);
					}

					if (entryOpt.has_value())
					{
						try
						{
							auto loadRes = loader->Load(*entryOpt, *m_PackageManager, *m_DataAssetsManager, *m_FileSystem, *m_GAPI);
							if (loadRes.has_value())
							{
								PublishResource(req.guid, *loadRes);
								result = *loadRes;
							}
							else
							{
								result = std::unexpected(loadRes.error());
								DOutError("[ResourceManager::IoWorkerLoop] Ошибка загрузки ресурса '{}' ({}): {}",
									entryOpt->GetName(), req.guid.ToString(), loadRes.error());
							}
						}
						catch (const std::exception& ex)
						{
							result = std::unexpected(std::format("Исключение при загрузке ресурса '{}' ({}): {}",
								entryOpt->GetName(), req.guid.ToString(), ex.what()));
							DOutError("[ResourceManager::IoWorkerLoop] {}", result.error());
						}
						catch (...)
						{
							result = std::unexpected(std::format("Неизвестное исключение при загрузке ресурса '{}' ({})",
								entryOpt->GetName(), req.guid.ToString()));
							DOutError("[ResourceManager::IoWorkerLoop] {}", result.error());
						}
					}
					else
					{
						try
						{
							THROW_RUNTIME("Запись ресурса с GUID '{}' не найдена в хранилище", req.guid.ToString());
						}
						catch (const std::exception& ex)
						{
							result = std::unexpected(ex.what());
							DOutError("[ResourceManager::IoWorkerLoop] {}", result.error());
						}
					}
				}

				// Извлекаем колбэки из in-flight таблицы
				std::vector<AnyResourceCallback> callbacks;
				{
					std::unique_lock lock(m_Mutex);
					auto it = m_InFlightCallbacks.find(req.guid);
					if (it != m_InFlightCallbacks.end())
					{
						callbacks = std::move(it->second);
						m_InFlightCallbacks.erase(it);
					}
				}

				// Передаем колбэки в очередь главного потока m_MainThreadQueue
				for (auto& cb : callbacks)
				{
					if (cb)
					{
						m_MainThreadQueue.Push([cb = std::move(cb), result]() mutable {
							cb(result);
						});
					}
				}

				{
					// m_FlushCv.notify_all() должен выполняться под тем же m_FlushMutex, что и
					// проверка предиката в Flush(), иначе возможна потеря пробуждения: поток Flush()
					// может проверить m_ActiveRequests > 0 и начать входить в wait() ровно в момент,
					// когда здесь декрементируется последний активный запрос и вызывается notify_all() -
					// тогда уведомление некому будет доставить, а других уведомлений уже не будет,
					// и Flush()/LoadSync() зависнут навсегда.
					std::lock_guard flushLock(m_FlushMutex);
					if (m_ActiveRequests.fetch_sub(1, std::memory_order_acq_rel) == 1)
					{
						m_FlushCv.notify_all();
					}
				}
			}
		}
	}

	void ResourceManager::UnloadSceneResources()
	{
		std::unique_lock lock(m_Mutex);
		m_Meshes.clear();
		m_MeshNames.clear();
		m_Textures.clear();
		m_TextureNames.clear();
		m_Materials.clear();
		m_MaterialNames.clear();
		// Шейдеры m_Shaders сохраняются, так как они глобальны для приложения
	}

	void ResourceManager::UnloadUnused(std::function<bool()> shouldStop)
	{
		std::unique_lock lock(m_Mutex);

		auto eraseUnused = [&shouldStop]<typename T>(std::unordered_map<Guid, std::shared_ptr<T>>& map, std::unordered_map<std::string, Guid>& names) {
			for (auto it = map.begin(); it != map.end(); )
			{
				if (shouldStop && shouldStop())
					return;

				if (it->second.use_count() == 1)
				{
					std::erase_if(names, [&it](const auto& pair) { return pair.second == it->first; });
					it = map.erase(it);
				}
				else
				{
					++it;
				}
			}
		};

		eraseUnused(m_Meshes, m_MeshNames);
		eraseUnused(m_Textures, m_TextureNames);
		eraseUnused(m_Materials, m_MaterialNames);
		eraseUnused(m_Shaders, m_ShaderNames);
	}

	void ResourceManager::UnloadMeshes()
	{
		std::unique_lock lock(m_Mutex);
		m_Meshes.clear();
		m_MeshNames.clear();
	}

	void ResourceManager::UnloadTextures()
	{
		std::unique_lock lock(m_Mutex);
		m_Textures.clear();
		m_TextureNames.clear();
	}

	void ResourceManager::UnloadMaterials()
	{
		std::unique_lock lock(m_Mutex);
		m_Materials.clear();
		m_MaterialNames.clear();
	}

	void ResourceManager::UnloadShaders()
	{
		std::unique_lock lock(m_Mutex);
		m_Shaders.clear();
		m_ShaderNames.clear();
	}

	void ResourceManager::UnloadAll()
	{
		std::unique_lock lock(m_Mutex);
		m_Meshes.clear();
		m_MeshNames.clear();
		m_Textures.clear();
		m_TextureNames.clear();
		m_Materials.clear();
		m_MaterialNames.clear();
		m_Shaders.clear();
		m_ShaderNames.clear();
	}

	size_t ResourceManager::GetLoadedMeshCount() const noexcept
	{
		std::shared_lock lock(m_Mutex);
		return m_Meshes.size();
	}

	size_t ResourceManager::GetLoadedTextureCount() const noexcept
	{
		std::shared_lock lock(m_Mutex);
		return m_Textures.size();
	}

	size_t ResourceManager::GetLoadedShaderCount() const noexcept
	{
		std::shared_lock lock(m_Mutex);
		return m_Shaders.size();
	}

	size_t ResourceManager::GetLoadedMaterialCount() const noexcept
	{
		std::shared_lock lock(m_Mutex);
		return m_Materials.size();
	}

	void ResourceManager::LoadMeshAsync(
		const Guid& meshGuid,
		std::function<void(std::expected<Guid, std::string>)> onCompleted,
		OwnerToken ownerToken)
	{
		LoadAsync<Mesh>(meshGuid, ResourceCallback<Mesh>([onCompleted = std::move(onCompleted), meshGuid, ownerToken](ResourceResult<Mesh> res) {
			if (!IsOwnerAlive(ownerToken)) return;
			if (!onCompleted) return;

			if (!res)
			{
				onCompleted(std::unexpected(res.error()));
			}
			else
			{
				onCompleted(meshGuid);
			}
		}), ownerToken);
	}

	void ResourceManager::LoadMaterialAsync(
		const Guid& materialGuid,
		std::function<void(std::expected<Guid, std::string>)> onCompleted,
		OwnerToken ownerToken)
	{
		LoadAsync<Material>(materialGuid, ResourceCallback<Material>([this, onCompleted = std::move(onCompleted), materialGuid, ownerToken](ResourceResult<Material> res) {
			if (!IsOwnerAlive(ownerToken)) return;
			if (!onCompleted) return;

			if (!res)
			{
				onCompleted(std::unexpected(res.error()));
			}
			else
			{
				auto mat = *res;
				if (mat && mat->GetShaderGuid().IsValid())
				{
					LoadShaderAsync(mat->GetShaderGuid(), [onCompleted, materialGuid, ownerToken](std::expected<Guid, std::string> shaderRes) {
						if (!IsOwnerAlive(ownerToken)) return;
						if (!onCompleted) return;

						if (!shaderRes)
						{
							onCompleted(std::unexpected(shaderRes.error()));
						}
						else
						{
							onCompleted(materialGuid);
						}
					}, ownerToken);
				}
				else
				{
					onCompleted(materialGuid);
				}
			}
		}), ownerToken);
	}

	void ResourceManager::LoadShaderAsync(
		const Guid& shaderGuid,
		std::function<void(std::expected<Guid, std::string>)> onCompleted,
		OwnerToken ownerToken)
	{
		LoadAsync<Shader>(shaderGuid, ResourceCallback<Shader>([onCompleted = std::move(onCompleted), shaderGuid, ownerToken](ResourceResult<Shader> res) {
			if (!IsOwnerAlive(ownerToken)) return;
			if (!onCompleted) return;

			if (!res)
			{
				onCompleted(std::unexpected(res.error()));
			}
			else
			{
				onCompleted(shaderGuid);
			}
		}), ownerToken);
	}
}
