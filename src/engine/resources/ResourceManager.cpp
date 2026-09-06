#include "ResourceManager.h"
#include "engine/package/PackageManager.h"
#include "core/io/package/DataAssetsManager.h"
#include "core/utils/Ensure.h"
Z_SET_LOG_CATEGORY(::zzz::core::LogEngine);

using namespace zzz::core;

namespace zzz::engine
{
	ResourceManager::ResourceManager(
		std::shared_ptr<PackageManager> packageManager,
		std::shared_ptr<::zzz::core::DataAssetsManager> dataAssetsManager,
		std::shared_ptr<::zzz::core::FileSystem> fileSystem,
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

	void ResourceManager::AddMesh(std::shared_ptr<Mesh> mesh)
	{
		if (!mesh) return;
		// Метод-заглушка: будет наполняться при реализации класса Mesh на шаге 11
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
		std::function<void(std::shared_ptr<IResource>)> onLoaded)
	{
		m_ActiveRequests.fetch_add(1, std::memory_order_release);
		m_RequestQueue.Emplace(ResourceLoadRequest{
			.guid = guid,
			.type = type,
			.name = std::move(name),
			.onLoaded = std::move(onLoaded)
		});
		m_IoCv.notify_one();
	}

	void ResourceManager::Flush()
	{
		std::unique_lock lock(m_FlushMutex);
		m_FlushCv.wait(lock, [this]() {
			return m_ActiveRequests.load(std::memory_order_acquire) == 0;
		});
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
				std::shared_ptr<IResource> loadedResource = nullptr;
				{
					std::shared_lock lock(m_Mutex);
					auto loaderIt = m_Loaders.find(req.type);
					if (loaderIt != m_Loaders.end() && m_PackageManager && m_FileSystem && m_GAPI)
					{
						auto entryOpt = m_PackageManager->GetEntry(req.guid);
						if (entryOpt)
						{
							auto loadRes = loaderIt->second->Load(*entryOpt, *m_PackageManager, *m_FileSystem, *m_GAPI);
							if (loadRes)
							{
								loadedResource = *loadRes;
							}
							else
							{
								DOutError("ResourceManager: Ошибка загрузки ресурса {}: {}", req.guid.ToString(), loadRes.error());
							}
						}
					}
				}

				if (req.onLoaded)
				{
					req.onLoaded(loadedResource);
				}

				if (m_ActiveRequests.fetch_sub(1, std::memory_order_acq_rel) == 1)
				{
					m_FlushCv.notify_all();
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
}
