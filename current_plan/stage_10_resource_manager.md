# Этап 10: Подсистема ресурсов (`ResourceManager`, `ResourceGarbageCollector`, `DoubleBufferedVector`)

## 1. Контекст и цели этапа
- **Номер пункта:** **Пункт 10** (Уровень 2: GAPI-ресурсы, содержимое куба и сквозной рендер).
- **Цель:** Создать расширяемую, типобезопасную и потокобезопасную подсистему управления жизненным циклом ресурсов движка. Внедрить базовый интерфейс ресурса (`IResource`), систему расширяемых загрузчиков по принципу открытости/закрытости (`IResourceLoader`), центральный менеджер кэширования и загрузки (`ResourceManager`) с интеграцией в `PackageManager` и ядро `Engine`.
- **Обоснование позиции в плане:** Размещение `ResourceManager` на шаге 9 (сразу после форматов ресурсов на шаге 8 и до создания GAPI-абстракций) позволяет всем последующим типам ресурсов (`Mesh`, `Texture2D`, `Material`, `Shader`, `Font`, `AudioClip`) создаваться и подключаться сразу в чистовую архитектуру через регистрацию собственных загрузчиков без необходимости рефакторинга и без временных костылей.
- **Статус:** `⏳ Не начато`.
- **Зависимости:** `src/core/enums/eResourceType.h`, `src/core/utils/Guid.h`, `src/engine/package/PackageManager.h`, `src/engine/engine.h`.

---

## 2. Архитектурные принципы подсистемы ресурсов

1. **Принцип открытости/закрытости (OCP — Open-Closed Principle):**
   - Загрузка каждого типа ресурса вынесена в специализированный `IResourceLoader`.
   - Регистрация загрузчика выполняется через `m_ResourceManager->RegisterLoader(std::make_unique<MeshLoader>())`.
2. **Отдельные типизированные таблицы ресурсов (Zero-Cast & Гранулярность):**
   - Вместо единой свалки ресурсов `ResourceManager` хранит 4 выделенные таблицы:
     - `m_Meshes`: 3D-сетки геометрии в VRAM;
     - `m_Textures`: 2D-текстуры в VRAM;
     - `m_Shaders`: скомпилированные шейдеры / PSO (глобальный кэш);
     - `m_Materials`: материалы (параметры, текстуры, шейдер).
   - Это гарантирует нулевой оверхед на приведение типов (`GetMesh()` сразу отдаёт `std::shared_ptr<Mesh>`) и легкий доступ к коллекциям для Студии/редактора.
3. **Раздельный жизненный цикл при смене сцен (`UnloadSceneResources`):**
   - При смене или выгрузке сцены `Mesh`, `Texture2D` и `Material` сцены полностью выгружаются из видеопамяти.
   - Скомпилированные `Shader` **сохраняются в памяти**, так как они глобальны для игры и не требуют повторной компиляции.
4. **Прямая асинхронная загрузка в GPU (без дублирования на CPU):**
   - Для GPU-ресурсов оперативная память CPU как кэш не используется: данные читаются фоновым потоком с диска / из `package.dat`, сразу транслируются в аппаратные ресурсы через `GAPI`, а временный буфер на CPU освобождается.
5. **Многопоточная безопасность горячего конвейера (Shared Mutex / Readers-Writer Lock):**
   - Обращение к кэшу ресурсов (`Get<T>(guid)` / `Get<T>(name)`) происходит из разных потоков кадра.
   - Используется `std::shared_mutex` (`shared_lock` на чтение, `unique_lock` на изменение списков).
6. **Интеграция с `Engine` и `SceneManager` при изоляции рендера:**
   - `std::shared_ptr<ResourceManager> m_ResourceManager` инициализируется в конструкторе `Engine` сразу после `PackageManager`, `FileSystem` и `GAPI`.
   - Прокидывается в `SceneManager` для разрешения ссылок на меши и материалы при парсинге `GameObject` сцены.
   - Рендеру (`RenderManager` / `SurfView`) объект `ResourceManager` **не передаётся** — рендер берёт всё необходимое из компонентов сцены.
7. **Барьер параллельной загрузки (`Flush`):**
   - При синхронной загрузке стартовой сцены все ассеты запускаются асинхронно параллельно на всех ядрах через `LoadAsync`, а в конце вызывается `m_ResourceManager->Flush()`. Это ускоряет старт в разы и гарантирует 100% готовность мира перед первым кадром.

---

## 3. Архитектурная спецификация

### 3.1. Перечисление состояний ресурса: `src/core/enums/eResourceState.h`

```cpp
#pragma once

#include <string_view>
#include "math/utils/Types.h"
#include "core/utils/ThrowWrappers.h"

namespace zzz::core
{
	enum class eResourceState : zU8
	{
		Unloaded = 0,   ///< Ресурс не загружен
		Loading,        ///< В процессе чтения / парсинга
		Ready,          ///< Ресурс готов к использованию (в RAM / на GPU)
		Failed          ///< Ошибка загрузки
	};

	[[nodiscard]] constexpr std::string_view ToString(eResourceState state)
	{
		switch (state)
		{
		case eResourceState::Unloaded: return "Unloaded";
		case eResourceState::Loading:  return "Loading";
		case eResourceState::Ready:    return "Ready";
		case eResourceState::Failed:   return "Failed";
		}
		THROW_RUNTIME("Необработанный eResourceState");
	}
}
```

---

### 3.2. Базовый интерфейс и класс ресурса: `src/core/resources/IResource.h` и `ResourceBase.h`

```cpp
// src/core/resources/IResource.h
#pragma once

#include <string_view>
#include "core/utils/Guid.h"
#include "core/enums/eResourceType.h"
#include "core/enums/eResourceState.h"

namespace zzz::core
{
	/**
	 * @class IResource
	 * @brief Базовый абстрактный интерфейс любого ресурса движка.
	 */
	class IResource
	{
	public:
		virtual ~IResource() = default;

		[[nodiscard]] virtual const Guid& GetGuid() const noexcept = 0;
		[[nodiscard]] virtual eResourceType GetResourceType() const noexcept = 0;
		[[nodiscard]] virtual std::string_view GetName() const noexcept = 0;
		[[nodiscard]] virtual eResourceState GetState() const noexcept = 0;
		virtual void SetState(eResourceState state) noexcept = 0;
	};
}
```

```cpp
// src/core/resources/ResourceBase.h
#pragma once

#include <string>
#include <atomic>
#include "core/resources/IResource.h"

namespace zzz::core
{
	/**
	 * @class ResourceBase
	 * @brief Каноническая базовая реализация метаданных ресурса.
	 */
	class ResourceBase : public IResource
	{
	public:
		ResourceBase(const Guid& guid, eResourceType type, std::string name)
			: m_Guid(guid)
			, m_Type(type)
			, m_Name(std::move(name))
			, m_State(eResourceState::Ready)
		{
		}

		~ResourceBase() override = default;

		[[nodiscard]] const Guid& GetGuid() const noexcept override { return m_Guid; }
		[[nodiscard]] eResourceType GetResourceType() const noexcept override { return m_Type; }
		[[nodiscard]] std::string_view GetName() const noexcept override { return m_Name; }
		[[nodiscard]] eResourceState GetState() const noexcept override { return m_State.load(std::memory_order_relaxed); }
		void SetState(eResourceState state) noexcept override { m_State.store(state, std::memory_order_relaxed); }

	protected:
		Guid m_Guid;
		eResourceType m_Type;
		std::string m_Name;
		std::atomic<eResourceState> m_State;
	};
}
```

---

### 3.3. Интерфейс расширяемого загрузчика: `src/engine/resources/IResourceLoader.h`

```cpp
#pragma once

#include <memory>
#include <string>
#include <expected>
#include "core/enums/eResourceType.h"
#include "core/resources/IResource.h"
#include "core/io/package/PackageEntry.h"
#include "core/io/FileSystem.h"

namespace zzz::engine
{
	class GAPI;
	class PackageManager;

	/**
	 * @class IResourceLoader
	 * @brief Полиморфный интерфейс загрузчика конкретного типа ассетов (из пакета или напрямую с диска).
	 */
	class IResourceLoader
	{
	public:
		virtual ~IResourceLoader() = default;

		[[nodiscard]] virtual ::zzz::core::eResourceType GetSupportedType() const noexcept = 0;

		/// @brief Загрузка ресурса из записи пакета package.dat
		[[nodiscard]] virtual std::expected<std::shared_ptr<::zzz::core::IResource>, std::string> Load(
			const ::zzz::core::PackageEntry& entry,
			PackageManager& packageManager,
			::zzz::core::FileSystem& fileSystem,
			GAPI& gapi) = 0;
	};
}
```

---

### 3.4. Шлифовка пинг-понг буфера: `src/core/templates/DoubleBufferedVector.h`

В существующий класс [`DoubleBufferedVector.h`](file:///c:/Workspaces/ZzzTest/src/core/templates/DoubleBufferedVector.h) вносятся точечные доработки для безопасного использования в дисковом потоке I/O:
1. **Устранение Data Race в `IsEmpty()`:** разделение проверки буфера записи `HasPendingWrites()` от буфера чтения. Читающий поток сам владеет `m_ReadBuffer` и проверяет его без локов.
2. **Метод `HasPendingWrites()`:**
   ```cpp
   [[nodiscard]] bool HasPendingWrites() const noexcept
   {
       std::lock_guard lock(m_Mutex);
       return !m_WriteBuffer.empty();
   }
   ```
3. **`const`-корректность и `[[nodiscard]]`:** добавлены атрибуты к методам проверки.
4. **Макрос `Z_NO_COPY_MOVE(DoubleBufferedVector);`** для запрета некорректного копирования объекта с мьютексом.

---

### 3.5. Менеджер ресурсов: `src/engine/resources/ResourceManager.h` и `.cpp`

```cpp
// src/engine/resources/ResourceManager.h
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
#include "core/enums/eResourceType.h"
#include "core/resources/IResource.h"
#include "engine/resources/IResourceLoader.h"
#include "core/io/FileSystem.h"
#include "core/templates/DoubleBufferedVector.h"
#include "core/utils/Ensure.h"
#include "core/utils/ThrowWrappers.h"

namespace zzz::engine
{
	class GAPI;
	class PackageManager;

	// Предварительные объявления типов ресурсов (реализуются на шагах 8-15)
	class Mesh;
	class Texture2D;
	class Shader;
	class Material;

	/**
	 * @class ResourceManager
	 * @brief Потокобезопасный центральный сервис кэширования и асинхронной загрузки GPU-ресурсов.
	 *
	 * @details Использует выделенный поток дискового ввода-вывода (Dedicated I/O Thread)
	 * с пинг-понг очередью DoubleBufferedVector<ResourceLoadRequest>. Игровые потоки закидывают
	 * запросы в буфер записи без блокировок, а поток I/O последовательно читает данные из буфера чтения.
	 */
	class ResourceManager final
	{
	public:
		struct ResourceLoadRequest
		{
			::zzz::core::Guid guid;
			::zzz::core::eResourceType type{ ::zzz::core::eResourceType::Unknown };
			std::string name;
			std::function<void(std::shared_ptr<::zzz::core::IResource>)> onLoaded;
		};

		ResourceManager() = delete;
		explicit ResourceManager(
			std::shared_ptr<PackageManager> packageManager,
			std::shared_ptr<::zzz::core::FileSystem> fileSystem,
			std::shared_ptr<GAPI> gapi);
		~ResourceManager();

		// --- Запуск и остановка потока ввода-вывода ---
		void Start();
		void Stop();

		// --- Регистрация загрузчиков (OCP) ---
		void RegisterLoader(std::unique_ptr<IResourceLoader> loader);

		template<typename LoaderT, typename... Args>
			requires std::derived_from<LoaderT, IResourceLoader>
		void RegisterLoader(Args&&... args)
		{
			RegisterLoader(std::make_unique<LoaderT>(std::forward<Args>(args)...));
		}

		[[nodiscard]] bool HasLoader(::zzz::core::eResourceType type) const noexcept;

		// --- Регистрация процедурных / рантайм ресурсов в соответствующие списки ---
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

		// --- Проверка наличия в соответствующих списках ---
		[[nodiscard]] bool HasMesh(const ::zzz::core::Guid& guid) const noexcept;
		[[nodiscard]] bool HasTexture(const ::zzz::core::Guid& guid) const noexcept;
		[[nodiscard]] bool HasShader(const ::zzz::core::Guid& guid) const noexcept;
		[[nodiscard]] bool HasMaterial(const ::zzz::core::Guid& guid) const noexcept;

		// --- Асинхронная загрузка с диска через пинг-понг очередь ---
		template<typename T>
		void LoadAsync(const ::zzz::core::Guid& guid, std::function<void(std::shared_ptr<T>)> onLoaded)
		{
			if (auto cached = Get<T>(guid))
			{
				if (onLoaded) onLoaded(cached);
				return;
			}
			EnqueueLoadRequest(guid, GetTypeFor<T>(), {}, [onLoaded = std::move(onLoaded)](std::shared_ptr<::zzz::core::IResource> res) {
				if (onLoaded) onLoaded(std::static_pointer_cast<T>(res));
			});
		}

		// --- Синхронизация и барьеры загрузки (для первичной сцены) ---
		void Flush();

		template<typename T>
		[[nodiscard]] std::shared_ptr<T> LoadSync(const ::zzz::core::Guid& guid)
		{
			if (auto cached = Get<T>(guid))
				return cached;

			std::promise<std::shared_ptr<T>> promise;
			auto future = promise.get_future();

			LoadAsync<T>(guid, [&promise](std::shared_ptr<T> res) {
				promise.set_value(res);
			});

			return future.get();
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
			std::function<void(std::shared_ptr<::zzz::core::IResource>)> onLoaded);

		void IoWorkerLoop(std::stop_token stopToken);

		std::shared_ptr<PackageManager> m_PackageManager;
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
```

---

### 3.6. Фоновый сборщик мусора и RAII-приостановка: `ResourceGarbageCollector.h` и `.cpp`

```cpp
// src/engine/resources/ResourceGarbageCollector.h
#pragma once

#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <chrono>

#include "core/utils/Defines.h"
#include "core/utils/Ensure.h"

namespace zzz::engine
{
	class ResourceManager;

	/**
	 * @class ResourceGarbageCollector
	 * @brief Автономный сервис фоновой очистки неиспользуемых ресурсов (SRP).
	 *
	 * @details Работает в отдельном потоке (std::jthread), периодически вызывая
	 * ResourceManager::UnloadUnused(). Поддерживает мгновенную кооперативную паузу.
	 */
	class ResourceGarbageCollector final
	{
	public:
		ResourceGarbageCollector() = delete;
		explicit ResourceGarbageCollector(
			ResourceManager& resourceManager,
			std::chrono::milliseconds interval = std::chrono::milliseconds(10000));
		~ResourceGarbageCollector();

		void Start();
		void Stop();

		/// @brief Приостанавливает фоновую сборку (инкремент счетчика пауз)
		void Pause();

		/// @brief Возобновляет фоновую сборку (декремент счетчика пауз)
		void Resume();

		[[nodiscard]] bool IsPaused() const noexcept { return m_PauseCount.load(std::memory_order_relaxed) > 0; }
		[[nodiscard]] bool IsRunning() const noexcept { return m_IsRunning.load(std::memory_order_relaxed); }

	private:
		void WorkerLoop(std::stop_token stopToken);

		ResourceManager& m_ResourceManager;
		std::chrono::milliseconds m_Interval;

		std::jthread m_Thread;
		std::mutex m_Mutex;
		std::condition_variable_any m_Cv;

		std::atomic<uint32_t> m_PauseCount{ 0 };
		std::atomic<bool> m_IsRunning{ false };
	};

	/**
	 * @class ScopedGCSuspension
	 * @brief RAII-страж приостановки фонового сборщика мусора.
	 *
	 * @details При создании гарантированно ставит GC на паузу.
	 * При выходе из блока видимости (включая stack unwinding при исключениях)
	 * автоматически возобновляет сборку.
	 */
	class ScopedGCSuspension final
	{
	public:
		explicit ScopedGCSuspension(ResourceGarbageCollector& gc)
			: m_GC(gc)
		{
			m_GC.Pause();
		}

		~ScopedGCSuspension()
		{
			m_GC.Resume();
		}

		Z_NO_COPY_MOVE(ScopedGCSuspension);

	private:
		ResourceGarbageCollector& m_GC;
	};
}
```

---

### 3.6. Интеграция в движок и проброс зависимостей

1. **Создание в `Engine` ([`src/engine/engine.h`](file:///c:/Workspaces/ZzzTest/src/engine/engine.h), [`engine.cpp`](file:///c:/Workspaces/ZzzTest/src/engine/engine.cpp)):**
   - В `Engine.h`:
     ```cpp
     std::shared_ptr<ResourceManager> m_ResourceManager;
     std::unique_ptr<ResourceGarbageCollector> m_ResourceGC;

     [[nodiscard]] inline std::shared_ptr<ResourceManager> GetResourceManager() const noexcept { return m_ResourceManager; }
     [[nodiscard]] inline ResourceGarbageCollector* GetResourceGC() const noexcept { return m_ResourceGC.get(); }
     ```
   - В `Engine.cpp`:
     ```cpp
     m_ResourceManager = safe_make_shared<ResourceManager>(m_PackageManager, m_FileSystem, m_GAPI);
     m_ResourceGC = safe_make_unique<ResourceGarbageCollector>(*m_ResourceManager);
     m_ResourceGC->Start();
     ```
   - В `Engine::Shutdown()`:
     ```cpp
     if (m_ResourceGC)
         m_ResourceGC->Stop();
     ```

2. **Проброс в `SceneManager` ([`src/engine/scene/SceneManager.h`](file:///c:/Workspaces/ZzzTest/src/engine/scene/SceneManager.h)):**
   - `SceneManager` получает `m_ResourceManager` и указатель на `ResourceGarbageCollector`.
   - При вызове `LoadScene()` внутри объявляется:
     ```cpp
     ScopedGCSuspension gcLock(*m_ResourceGC);
     ```
     гарантируя, что во время сборки и линковки сцены ни один ресурс не будет удален фоновым сборщиком.

3. **Изоляция рендера (Least Privilege):**
   - Рендеру (`RenderManager` / `SurfView`) объекты `ResourceManager` и `ResourceGarbageCollector` **не передаются**.
   - Рендер получает всё необходимое исключительно из компонентов сцены (`MeshRenderer` содержит ссылки на конкретные `Mesh` и `Material`).

---

## 4. План верификации

1. **Компиляция под MSVC x64 + Ninja:**
   - Чистая компиляция целей `EngineTests` и `game_win` (0 ошибок, 0 предупреждений).
   - Проверка регистрации всех созданных файлов в `src/core/CMakeLists.txt` и `src/engine/CMakeLists.txt`.
2. **Сквозной запуск `game_win.exe`:**
   - Проверка успешной инициализации `m_ResourceManager` и запуска `m_ResourceGC->Start()` в жизненном цикле `Engine::Engine()`.
   - Проверка успешного создания `SceneManager` с проброшенными менеджером ресурсов и сборщиком мусора.
   - Проверка корректной остановки `m_ResourceGC->Stop()` и освобождения ресурсов при выходе `Engine::Shutdown()`.
   - Завершение процесса с кодом 0.

---

## 5. Чек-лист Definition of Done (DoD)

- [ ] Отшлифовать `src/core/templates/DoubleBufferedVector.h` (`HasPendingWrites()`, `const`, `[[nodiscard]]`, `Z_NO_COPY_MOVE`)
- [ ] Создать `src/core/enums/eResourceState.h` с `ToString(eResourceState)`
- [ ] Создать `src/core/resources/IResource.h` и `src/core/resources/ResourceBase.h`
- [ ] Создать `src/engine/resources/IResourceLoader.h` (с параметрами `PackageEntry`, `PackageManager`, `FileSystem`, `GAPI`)
- [ ] Создать `src/engine/resources/ResourceManager.h` и `src/engine/resources/ResourceManager.cpp` (с раздельными списками, выделенным I/O-потоком и `UnloadUnused`)
- [ ] Создать `src/engine/resources/ResourceGarbageCollector.h` и `ResourceGarbageCollector.cpp` (с RAII `ScopedGCSuspension`)
- [ ] Интегрировать `m_ResourceManager` и `m_ResourceGC` в `src/engine/engine.h` и `engine.cpp`
- [ ] Пробросить `m_ResourceManager` и `m_ResourceGC` в `src/engine/scene/SceneManager.h` и `SceneManager.cpp`
- [ ] Зарегистрировать новые файлы в `src/core/CMakeLists.txt` и `src/engine/CMakeLists.txt`
- [ ] Собрать `EngineTests.exe` и `game_win` под MSVC + Ninja (чистая сборка)
- [ ] Запустить `game_win.exe` (чистый запуск, работа фонового потока, чистый shutdown и код 0)
- [ ] Запросить утверждение у пользователя
- [ ] Зафиксировать Git-коммит: `feat(engine): completed stage 10 - ResourceManager and ResourceGarbageCollector architecture`
- [ ] Обновить статус Пункта 10 в `general_plan.md` на `✅ Выполнено`
