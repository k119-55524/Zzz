#pragma once

#include <memory>
#include <string>
#include <format>
#include <expected>

#include "core/utils/Guid.h"
#include "core/io/FileSystem.h"
#include "core/enums/eResourceType.h"
#include "core/io/package/PackageEntry.h"
#include "core/templates/CallbackQueue.h"
#include "engine/tasks/TaskDispatcher.h"
#include "engine/resources/ResourceTable.h"
#include "engine/resources/cpu/CpuMesh.h"
#include "engine/resources/cpu/CpuMaterial.h"
#include "engine/resources/cpu/CpuTexture2D.h"
#include "engine/resources/cpu/CpuShader.h"
#include "core/io/package/DataAssetsManager.h"

using namespace zzz::core;

namespace zzz::engine
{
	class PackageManager;

	/**
	 * @class CpuResourceManager
	 * @brief Централизованный сервис асинхронной загрузки и кэширования CPU-ресурсов.
	 * @details Хранит типизированные таблицы ресурсов m_Meshes, m_Materials, m_Textures, m_Shaders.
	 *          Предоставляет доступ через метод GetTable<T>() и единый фасад GetAsync<T>().
	 */
	class CpuResourceManager final
	{
		Z_NO_COPY_MOVE(CpuResourceManager);

	public:
		CpuResourceManager() = delete;

		explicit CpuResourceManager(
			TaskDispatcher& taskDispatcher,
			std::shared_ptr<PackageManager> packageManager,
			std::shared_ptr<DataAssetsManager> dataAssetsManager = nullptr,
			std::shared_ptr<FileSystem> fileSystem = nullptr);

		~CpuResourceManager();

		inline void Update() { m_MainThreadQueue.ExecuteAll(); }

		template<typename T, typename ContextType>
		void GetAsync(
			const Guid& guid,
			std::weak_ptr<ContextType> context,
			typename ResourceTable<T>::CallbackType onLoaded)
		{
			GetTable<T>().GetOrRequest(guid,
				std::move(context),
				std::move(onLoaded),
				GetDispatcher(),
				[this](const Guid& g)
				{
					DispatchLoad<T>(g);
				});
		}

	private:
		template<typename T>
		[[nodiscard]] auto& GetTable() noexcept
		{
			if constexpr (std::is_same_v<T, CpuMesh>)           return m_Meshes;
			else if constexpr (std::is_same_v<T, CpuMaterial>)  return m_Materials;
			else if constexpr (std::is_same_v<T, CpuTexture2D>) return m_Textures;
			else if constexpr (std::is_same_v<T, CpuShader>)    return m_Shaders;
			else static_assert(sizeof(T) == 0, "Запрашиваемый тип ресурса не поддерживается CpuResourceManager!");
		}

		void EmergencyStop();

		[[nodiscard]] auto GetDispatcher()
		{
			return [this](auto task) { m_MainThreadQueue.Push(std::move(task)); };
		}

		template<typename T>
		void DispatchLoad(const Guid& guid);

		template<typename T>
		std::expected<std::shared_ptr<T>, std::string> LoadResourceSync(const Guid& guid);

		[[nodiscard]] std::expected<PackageEntry, std::string> FindEntry(const Guid& guid, eResourceType type) const;

		TaskDispatcher& m_TaskDispatcher;
		std::shared_ptr<PackageManager> m_PackageManager;
		std::shared_ptr<DataAssetsManager> m_DataAssetsManager;
		std::shared_ptr<FileSystem> m_FileSystem;

		CallbackQueue<> m_MainThreadQueue;

		ResourceTable<CpuMesh>      m_Meshes;
		ResourceTable<CpuMaterial>  m_Materials;
		ResourceTable<CpuTexture2D> m_Textures;
		ResourceTable<CpuShader>    m_Shaders;
	};

	template<typename T>
	void CpuResourceManager::DispatchLoad(const Guid& guid)
	{
		m_TaskDispatcher.Submit(eTaskPriority::Normal, [this, guid]()
		{
			try
			{
				auto result = LoadResourceSync<T>(guid);
				GetTable<T>().Resolve(guid, std::move(result), GetDispatcher());
			}
			catch (const std::exception& ex)
			{
				GetTable<T>().Resolve(guid, std::unexpected(std::format("Исключение при загрузке: {}", ex.what())), GetDispatcher());
			}
			catch (...)
			{
				GetTable<T>().Resolve(guid, std::unexpected(std::string("Неизвестное исключение при загрузке")), GetDispatcher());
			}
		});
	}
}
