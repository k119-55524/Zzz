#include "CpuResourceManager.h"
#include "engine/resources/cpu/loaders/CpuMeshLoader.h"
#include "engine/resources/cpu/loaders/CpuMaterialLoader.h"
#include "engine/resources/cpu/loaders/CpuShaderLoader.h"
#include "engine/package/PackageManager.h"
#include "core/io/package/DataAssetsManager.h"
#include "core/io/ResourceStorageTraits.h"
#include "core/utils/Ensure.h"
#include "core/utils/MemoryUtils.h"
#include <logger.h>

Z_SET_LOG_CATEGORY(::zzz::core::LogEngine);

using namespace zzz::core;
using namespace zzz::templates;

namespace zzz::engine
{
	CpuResourceManager::CpuResourceManager(
		std::shared_ptr<PackageManager> packageManager,
		std::shared_ptr<DataAssetsManager> dataAssetsManager,
		std::shared_ptr<FileSystem> fileSystem)
		: m_PackageManager(std::move(packageManager))
		, m_DataAssetsManager(std::move(dataAssetsManager))
		, m_FileSystem(std::move(fileSystem))
	{
		ensure(m_PackageManager.get() != nullptr, "PackageManager не должен быть null в CpuResourceManager.");
	}

	std::shared_ptr<PackageManager> CpuResourceManager::GetPackageManager() const noexcept
	{
		return m_PackageManager;
	}

	std::shared_ptr<DataAssetsManager> CpuResourceManager::GetDataAssetsManager() const noexcept
	{
		return m_DataAssetsManager;
	}

	std::shared_ptr<FileSystem> CpuResourceManager::GetFileSystem() const noexcept
	{
		return m_FileSystem;
	}

	CpuResourceManager::~CpuResourceManager()
	{
		Stop();
	}

	void CpuResourceManager::Start()
	{
		if (m_IoThread.joinable())
			return;

		m_IoThread = std::jthread([this](std::stop_token st) {
			IoWorkerLoop(std::move(st));
		});
	}

	void CpuResourceManager::Stop()
	{
		if (m_IoThread.joinable())
		{
			m_IoThread.request_stop();
			m_IoCv.notify_all();
			m_IoThread.join();
		}
	}

	void CpuResourceManager::Update()
	{
		m_MainThreadQueue.ExecuteAll();
	}

	void CpuResourceManager::Flush()
	{
		std::unique_lock lock(m_FlushMutex);
		m_FlushCv.wait(lock, [this] {
			return m_ActiveRequests.load(std::memory_order_acquire) == 0;
		});
	}

	std::expected<SceneData, std::string> CpuResourceManager::LoadSceneData(const Guid& sceneGuid)
	{
		std::shared_lock lock(m_TablesMutex);
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

	std::expected<SceneData, std::string> CpuResourceManager::LoadSceneData(std::string_view sceneName)
	{
		auto entryOpt = m_PackageManager->GetEntry(ePackage::Scene, sceneName);
		if (!entryOpt.has_value())
		{
			return std::unexpected(std::format("Сцена с именем '{}' не найдена в package.dat.", sceneName));
		}

		return LoadSceneData(entryOpt->GetGuid());
	}

	std::expected<PackageEntry, std::string> CpuResourceManager::FindEntry(
		const Guid& guid,
		eResourceType type) const
	{
		const auto storageKind = GetResourceStorageTraits(type).storageKind;
		if (storageKind == eResourceStorageKind::DataArchive)
		{
			if (!m_DataAssetsManager)
			{
				return std::unexpected("DataAssetsManager не инициализирован");
			}
			auto entryOpt = m_DataAssetsManager->GetEntry(type, guid);
			if (entryOpt.has_value())
			{
				return *entryOpt;
			}
		}
		else if (storageKind == eResourceStorageKind::PackageArchive)
		{
			if (!m_PackageManager)
			{
				return std::unexpected("PackageManager не инициализирован");
			}
			auto entryOpt = m_PackageManager->GetEntry(guid);
			if (entryOpt.has_value())
			{
				return *entryOpt;
			}
		}

		return std::unexpected(std::format("Запись ресурса с GUID '{}' не найдена в хранилище", guid.ToString()));
	}

	void CpuResourceManager::IoWorkerLoop(std::stop_token stopToken)
	{
		while (!stopToken.stop_requested())
		{
			{
				std::unique_lock lock(m_IoMutex);
				m_IoCv.wait(lock, stopToken, [this] {
					return m_RequestQueue.HasPendingWrites();
				});
			}

			if (stopToken.stop_requested())
				break;

			auto& requests = m_RequestQueue.SwapAndGetReadBuffer();
			struct RequestScopeGuard
			{
				std::atomic<size_t>& activeRequests;
				std::mutex& flushMutex;
				std::condition_variable& flushCv;

				~RequestScopeGuard()
				{
					std::lock_guard flushLock(flushMutex);
					if (activeRequests.fetch_sub(1, std::memory_order_acq_rel) == 1)
					{
						flushCv.notify_all();
					}
				}
			};

			for (const auto& req : requests)
			{
				RequestScopeGuard guard(m_ActiveRequests, m_FlushMutex, m_FlushCv);
				try
				{
					switch (req.kind)
					{
					case eCpuResourceKind::Mesh:
					{
						auto entryRes = FindEntry(req.guid, eResourceType::Mesh);
						if (!entryRes)
						{
							std::unique_lock lock(m_TablesMutex);
							auto [it, _] = m_CpuMeshes.try_emplace(req.guid, [this](auto task) { m_MainThreadQueue.Push(std::move(task)); });
							it->second.readyEvent.Resolve(std::unexpected(entryRes.error()));
						}
						else if (!m_DataAssetsManager)
						{
							std::unique_lock lock(m_TablesMutex);
							auto [it, _] = m_CpuMeshes.try_emplace(req.guid, [this](auto task) { m_MainThreadQueue.Push(std::move(task)); });
							it->second.readyEvent.Resolve(std::unexpected("DataAssetsManager не инициализирован"));
						}
						else
						{
							auto loadRes = CpuMeshLoader::Load(*entryRes, *m_DataAssetsManager);
							std::unique_lock lock(m_TablesMutex);
							auto [it, _] = m_CpuMeshes.try_emplace(req.guid, [this](auto task) { m_MainThreadQueue.Push(std::move(task)); });
							if (loadRes)
							{
								it->second.resource = *loadRes;
								if (!entryRes->GetName().empty())
								{
									m_MeshNames[std::string(entryRes->GetName())] = req.guid;
								}
								it->second.readyEvent.Resolve(*loadRes);
							}
							else
							{
								it->second.readyEvent.Resolve(std::unexpected(loadRes.error()));
							}
						}
						break;
					}
					case eCpuResourceKind::Material:
					{
						auto entryRes = FindEntry(req.guid, eResourceType::Material);
						if (!entryRes)
						{
							std::unique_lock lock(m_TablesMutex);
							auto [it, _] = m_CpuMaterials.try_emplace(req.guid, [this](auto task) { m_MainThreadQueue.Push(std::move(task)); });
							it->second.readyEvent.Resolve(std::unexpected(entryRes.error()));
						}
						else if (!m_DataAssetsManager)
						{
							std::unique_lock lock(m_TablesMutex);
							auto [it, _] = m_CpuMaterials.try_emplace(req.guid, [this](auto task) { m_MainThreadQueue.Push(std::move(task)); });
							it->second.readyEvent.Resolve(std::unexpected("DataAssetsManager не инициализирован"));
						}
						else
						{
							auto loadRes = CpuMaterialLoader::Load(*entryRes, *m_DataAssetsManager);
							std::unique_lock lock(m_TablesMutex);
							auto [it, _] = m_CpuMaterials.try_emplace(req.guid, [this](auto task) { m_MainThreadQueue.Push(std::move(task)); });
							if (loadRes)
							{
								it->second.resource = *loadRes;
								if (!entryRes->GetName().empty())
								{
									m_MaterialNames[std::string(entryRes->GetName())] = req.guid;
								}
								it->second.readyEvent.Resolve(*loadRes);
							}
							else
							{
								it->second.readyEvent.Resolve(std::unexpected(loadRes.error()));
							}
						}
						break;
					}
					case eCpuResourceKind::Shader:
					{
						auto entryRes = FindEntry(req.guid, eResourceType::Shader);
						if (!entryRes)
						{
							std::unique_lock lock(m_TablesMutex);
							auto [it, _] = m_CpuShaders.try_emplace(req.guid, [this](auto task) { m_MainThreadQueue.Push(std::move(task)); });
							it->second.readyEvent.Resolve(std::unexpected(entryRes.error()));
						}
						else if (!m_DataAssetsManager)
						{
							std::unique_lock lock(m_TablesMutex);
							auto [it, _] = m_CpuShaders.try_emplace(req.guid, [this](auto task) { m_MainThreadQueue.Push(std::move(task)); });
							it->second.readyEvent.Resolve(std::unexpected("DataAssetsManager не инициализирован"));
						}
						else
						{
							auto loadRes = CpuShaderLoader::Load(*entryRes, *m_DataAssetsManager);
							std::unique_lock lock(m_TablesMutex);
							auto [it, _] = m_CpuShaders.try_emplace(req.guid, [this](auto task) { m_MainThreadQueue.Push(std::move(task)); });
							if (loadRes)
							{
								it->second.resource = *loadRes;
								if (!entryRes->GetName().empty())
								{
									m_ShaderNames[std::string(entryRes->GetName())] = req.guid;
								}
								it->second.readyEvent.Resolve(*loadRes);
							}
							else
							{
								it->second.readyEvent.Resolve(std::unexpected(loadRes.error()));
							}
						}
						break;
					}
					case eCpuResourceKind::Texture:
					{
						auto entryRes = FindEntry(req.guid, eResourceType::Texture2D);
						if (!entryRes)
						{
							std::unique_lock lock(m_TablesMutex);
							auto [it, _] = m_CpuTextures.try_emplace(req.guid, [this](auto task) { m_MainThreadQueue.Push(std::move(task)); });
							it->second.readyEvent.Resolve(std::unexpected(entryRes.error()));
						}
						else
						{
							// Заглушка для загрузки текстуры (наполняется по мере необходимости)
							auto texture = safe_make_shared<CpuTexture2D>(entryRes->GetGuid(), std::string(entryRes->GetName()));
							std::unique_lock lock(m_TablesMutex);
							auto [it, _] = m_CpuTextures.try_emplace(req.guid, [this](auto task) { m_MainThreadQueue.Push(std::move(task)); });
							it->second.resource = texture;
							if (!entryRes->GetName().empty())
							{
								m_TextureNames[std::string(entryRes->GetName())] = req.guid;
							}
							it->second.readyEvent.Resolve(texture);
						}
						break;
					}
					}
				}
				catch (const std::exception& ex)
				{
					std::unique_lock lock(m_TablesMutex);
					const std::string err = std::format("Исключение при загрузке ресурса: {}", ex.what());
					switch (req.kind)
					{
					case eCpuResourceKind::Mesh:
					{
						auto [it, _] = m_CpuMeshes.try_emplace(req.guid, [this](auto task) { m_MainThreadQueue.Push(std::move(task)); });
						it->second.readyEvent.Resolve(std::unexpected(err));
						break;
					}
					case eCpuResourceKind::Material:
					{
						auto [it, _] = m_CpuMaterials.try_emplace(req.guid, [this](auto task) { m_MainThreadQueue.Push(std::move(task)); });
						it->second.readyEvent.Resolve(std::unexpected(err));
						break;
					}
					case eCpuResourceKind::Shader:
					{
						auto [it, _] = m_CpuShaders.try_emplace(req.guid, [this](auto task) { m_MainThreadQueue.Push(std::move(task)); });
						it->second.readyEvent.Resolve(std::unexpected(err));
						break;
					}
					case eCpuResourceKind::Texture:
					{
						auto [it, _] = m_CpuTextures.try_emplace(req.guid, [this](auto task) { m_MainThreadQueue.Push(std::move(task)); });
						it->second.readyEvent.Resolve(std::unexpected(err));
						break;
					}
					}
				}
				catch (...)
				{
					std::unique_lock lock(m_TablesMutex);
					const std::string err = "Неизвестное исключение при загрузке ресурса";
					switch (req.kind)
					{
					case eCpuResourceKind::Mesh:
					{
						auto [it, _] = m_CpuMeshes.try_emplace(req.guid, [this](auto task) { m_MainThreadQueue.Push(std::move(task)); });
						it->second.readyEvent.Resolve(std::unexpected(err));
						break;
					}
					case eCpuResourceKind::Material:
					{
						auto [it, _] = m_CpuMaterials.try_emplace(req.guid, [this](auto task) { m_MainThreadQueue.Push(std::move(task)); });
						it->second.readyEvent.Resolve(std::unexpected(err));
						break;
					}
					case eCpuResourceKind::Shader:
					{
						auto [it, _] = m_CpuShaders.try_emplace(req.guid, [this](auto task) { m_MainThreadQueue.Push(std::move(task)); });
						it->second.readyEvent.Resolve(std::unexpected(err));
						break;
					}
					case eCpuResourceKind::Texture:
					{
						auto [it, _] = m_CpuTextures.try_emplace(req.guid, [this](auto task) { m_MainThreadQueue.Push(std::move(task)); });
						it->second.readyEvent.Resolve(std::unexpected(err));
						break;
					}
					}
				}
			}
		}
	}
}
