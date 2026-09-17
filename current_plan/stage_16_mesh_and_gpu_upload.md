# Этап 16: ResourceManager, Mesh и подготовка к загрузке в GPU

## 1. Статус и границы

- **Статус:** ✅ Выполнено. Реализовано и проверено сборкой `engine_lib` и `engine_lib_editor` 2026-09-10. Реализация (`ResourceManager.cpp`) дополнительно вычитана по факту написания кода 2026-09-10: устранена гонка потери пробуждения в `Flush()`/`LoadSync()` (декремент `m_ActiveRequests` + `notify_all()` в `IoWorkerLoop` теперь под тем же `m_FlushMutex`, что и `wait()` в `Flush()`); исправлена мёртвая ветка обработки ошибки "запись не найдена" (`else if (result.has_value())` не могло быть true, ошибка глушилась без лога); `m_DataAssetsManager` добавлен в общую проверку готовности подсистем в `IoWorkerLoop`, чтобы не разыменовывать его вслепую для `PackageArchive`-ресурсов.
- **Цель:** Получить надёжную асинхронную загрузку ресурсов через `ResourceManager` с правильной маршрутизацией по хранилищам, дедупликацией in-flight запросов, прокидыванием результатов/ошибок на главный поток через `CallbackQueue` и подготовкой ресурса `Mesh` до состояния готовности к отправке на GPU (`eResourceState::Ready` на CPU-уровне).
- **Сквозной путь этапа 16:**
  1. Вызов `ResourceManager::LoadAsync<Mesh>(guid, callback)` из логики сцены/главного потока.
  2. Проверка кэша и реестра in-flight: если ресурс уже загружается, колбэк сохраняется без дублирования I/O-задачи.
  3. Маршрутизация запроса в `IoWorkerLoop` по `ResourceStorageTraits`: для `Mesh` (`DataArchive`) — чтение записи из `data.dat` через `DataAssetsManager::GetEntry`.
  4. Однопроходная десериализация `MeshData` через `DataAssetsManager::DeserializeAsset<MeshData>(entry)` без повторного поиска по таблице.
  5. Создание CPU-ресурса `Mesh` через `MeshLoader`, валидация данных геометрии, установка состояния `eResourceState::Ready`.
  6. Публикация ресурса в кэш `m_Meshes[guid]` и `m_MeshNames[resource->GetName()]`.
  7. Заворачивание вызовов колбэков в задачи и отправка в межпоточную очередь `CallbackQueue<> m_MainThreadQueue`.
  8. Исполнение колбэков на главном потоке кадра внутри `ResourceManager::Update()`, вызываемого из `Engine::OnUpdateSystem()`.
- **Граница этапа:** Никаких `fenceValue`, `VkFence`, staging-буферов, `RetireQueue`, командных списков копирования и отправки на GPU в этом этапе **нет**. Всё GPU-взаимодействие (создание `GPUBuffer`, выделение staging-памяти, запись команд копирования, `VkFence`/`SignalFence`, барьеры ресурсов и отложенное освобождение staging) — это **этап 19** после исправлений сцен этапов 17–18.
- **Тесты:** Модульные тесты в `src/qa/tests/` не создаются по решению пользователя. Проверка — сборка `engine_lib` и `engine_lib_editor`.

## 2. Сверка с кодовой базой и исправление замечаний

1. **Замечание L (`eResourceState`):**
   - В кодовой базе (`src/core/enums/eResourceState.h`) определены состояния:
     `Unloaded` (0), `Loading` (1), `Ready` (2), `Failed` (3).
   - Состояния `Loaded` не существует.
   - **Решение:** Используются исключительно валидные состояния: `Loading` при постановке/чтении, `Ready` при успешной подготовке геометрии на CPU, `Failed` при ошибке.
2. **Замечание M (Пространства имён):**
   - `GAPI` объявлен в `namespace zzz::engine` (`src/engine/gapi/GAPI.h`).
   - `PackageManager` объявлен в `namespace zzz::engine` (`src/engine/package/PackageManager.h`).
   - Внутри `namespace zzz::engine` оба типа используются без префиксов (`PackageManager&`, `GAPI&`).
   - Типы ядра квалифицируются: `::zzz::core::DataAssetsManager&`, `::zzz::core::FileSystem&`, `::zzz::core::PackageEntry&`, `::zzz::core::IResource`.
3. **Замечание N (Возврат ошибок из загрузчика):**
   - Метод `IResourceLoader::Load` возвращает `std::expected<std::shared_ptr<::zzz::core::IResource>, std::string>`.
   - Это гарантирует, что текст ошибки (не удалось прочитать файл, нарушена CRC, ошибка парсинга `MeshData`) не теряется, а логируется и передаётся в результат колбэка.
4. **Замечание O (Объявление типов колбэков):**
   - В `ResourceManager.h` вводятся явные псевдонимы типов для результатов и колбэков:
     ```cpp
     template <typename T>
     using ResourceResult = std::expected<std::shared_ptr<T>, std::string>;

     template <typename T>
     using ResourceCallback = std::function<void(ResourceResult<T>)>;

     using AnyResourceResult = std::expected<std::shared_ptr<::zzz::core::IResource>, std::string>;
     using AnyResourceCallback = std::function<void(AnyResourceResult)>;
     ```
5. **Асинхронность и прокидывание наверх (`CallbackQueue`):**
   - В движке уже есть проверенный потокобезопасный класс **`zzz::templates::CallbackQueue<>`** (`src/core/templates/CallbackQueue.h`), используемый в `SceneManager` (`m_MainThreadQueue`).
   - Фоновый поток I/O не должен вызывать пользовательские колбэки напрямую, так как это приводит к гонкам данных и проблемам многопоточности в логике сцены/UI.
   - I/O-поток отправляет результаты в `CallbackQueue<> m_MainThreadQueue` через `Push()`.
   - Главный поток движка вызывает `ResourceManager::Update()` внутри `Engine::OnUpdateSystem()`, где вызывается `m_MainThreadQueue.ExecuteAll()` без удержания блокировок.

## 3. Согласованные контракты

### 3.1. Маршрутизация по хранилищам

- Определение хранилища выполняется по типу ресурса через `ResourceStorageTraits`:
  - `eResourceStorageKind::DataArchive` (`Mesh`, `Shader`, `Material`, `Prefab`) → поиск записи в `data.dat` через `m_DataAssetsManager->GetEntry(req.type, req.guid)`.
  - `eResourceStorageKind::PackageArchive` (`Scene`, `View`) → поиск записи в `package.dat` через `m_PackageManager->GetEntry(req.guid)`.
- Чтобы `ResourceManager` мог вызывать приватный метод `DataAssetsManager::GetEntry()`, в `DataAssetsManager.h` добавляются:
  ```cpp
  namespace zzz::engine { class ResourceManager; } // Forward declaration
  // Внутри класса DataAssetsManager:
  friend class ::zzz::engine::ResourceManager;
  ```
- Для предотвращения повторного поиска GUID по хеш-таблице в `DataAssetsManager` добавляется метод однопроходного чтения по уже найденной записи:
  ```cpp
  template <typename T>
  [[nodiscard]] std::expected<T, std::string> DeserializeAsset(const PackageEntry& entry) const;
  ```

### 3.2. Асинхронность, доставка через CallbackQueue и дедупликация

1. **Реестр in-flight запросов:**
   - Член класса: `std::unordered_map<::zzz::core::Guid, std::vector<AnyResourceCallback>> m_InFlightCallbacks;` под защитой `m_Mutex`.
   - При вызове `LoadAsync`:
     - Если ресурс уже в кэше — колбэк немедленно ставится в `m_MainThreadQueue.Push(...)` с готовым результатом.
     - Если ресурс уже загружается (есть в `m_InFlightCallbacks`) — колбэк добавляется в вектор ожидающих слушателей этого GUID без повторной постановки задачи в I/O очередь.
     - Если запрос новый — создаётся запись в `m_InFlightCallbacks[guid]`, инкрементируется счётчик активных задач `m_ActiveRequests` и запрос добавляется в `m_RequestQueue`.
2. **Передача на главный поток через `CallbackQueue`:**
   - После выполнения `Load()` в `IoWorkerLoop`:
     - Под `m_Mutex` забирается список колбэков для данного GUID и удаляется запись из `m_InFlightCallbacks`.
     - Декрементируется `m_ActiveRequests`.
     - Для каждого слушателя формируется задача и отправляется в `m_MainThreadQueue.Push(...)`.
3. **Обработка кадров в `Engine`:**
   - В `ResourceManager` создаётся метод `void Update();`, выполняющий:
     ```cpp
     void ResourceManager::Update()
     {
         m_MainThreadQueue.ExecuteAll();
     }
     ```
   - В `Engine::OnUpdateSystem()` вызывается:
     ```cpp
     m_ResourceManager->Update();
     m_SceneManager->Update(*m_Time);
     ```
4. **Синхронизация в `Flush()` и `LoadSync()`:**
   - Метод `Flush()`:
     ```cpp
     void ResourceManager::Flush()
     {
         // 1. Ожидаем завершения всех фоновых I/O задач
         while (m_ActiveRequests.load(std::memory_order_acquire) > 0)
         {
             std::this_thread::yield();
         }
         // 2. Доставляем все накопленные колбэки на текущий поток
         m_MainThreadQueue.ExecuteAll();
     }
     ```
   - Метод `LoadSync<T>(guid)`:
     ```cpp
     template<typename T>
     [[nodiscard]] std::shared_ptr<T> LoadSync(const ::zzz::core::Guid& guid)
     {
         if (auto cached = Get<T>(guid))
             return cached;

         LoadAsync<T>(guid, ResourceCallback<T>{}); // явный тип, а не nullptr - иначе перегрузка LoadAsync неоднозначна между ResourceCallback<T> и std::function<void(shared_ptr<T>)>
         Flush();
         return Get<T>(guid);
     }
     ```
     Полностью исключает дедлоки на `future.get()`.

### 3.3. Состояния ресурса и обработка ошибок

- Конструктор `ResourceBase` (и, соответственно, `Mesh`) безусловно устанавливает `m_State = eResourceState::Ready` - промежуточного состояния `Loading` для самого объекта не существует и не нужно: объект `Mesh` физически создаётся только после успешной десериализации `MeshData` в `MeshLoader::Load()`, то есть уже готовым.
- При любой ошибке загрузчик не создаёт объект `Mesh` вовсе - `IResourceLoader::Load` возвращает `std::unexpected(err)`, и это значение доставляется в `CallbackQueue` как есть, без промежуточного перевода несуществующего объекта в `eResourceState::Failed`.
- Ошибка логируется в `IoWorkerLoop` через `DOutError(...)` рядом с точкой возникновения (не найден загрузчик, не инициализированы подсистемы, запись не найдена в хранилище, ошибка `loader->Load()`).

### 3.4. Кэширование и публикация

- `PublishResource` регистрирует ресурс:
  - В типизированную таблицу по GUID: `m_Meshes[guid] = mesh;`.
  - В индекс по имени: `m_MeshNames[resource->GetName()] = mesh;` (имя берётся из `resource->GetName()`, сформированного из `entry.GetName()`, а не из пустого `req.name`).
- Реализуется публичный метод добавления готового меша:
  ```cpp
  void AddMesh(std::shared_ptr<Mesh> mesh);
  ```

## 4. Порядок реализации

### Шаг 1. Ядро: доступ к DataAssetsManager и однопроходное чтение

- В `src/core/io/package/DataAssetsManager.h`:
  - Добавить forward declaration `namespace zzz::engine { class ResourceManager; }`.
  - Внутри класса `DataAssetsManager` добавить `friend class ::zzz::engine::ResourceManager;` (метод `GetEntry` остаётся приватным).
  - Реализовать публичный шаблонный метод `DeserializeAsset`, переиспользуя существующий паттерн чтения из приватного `DeserializeEntry<T>` (тот же `FileSystem::ReadBytes` + инстанс `Serializer::Deserialize`), но без повторного поиска записи по GUID — запись уже передана вызывающим кодом:
    ```cpp
    template <typename T>
    [[nodiscard]] std::expected<T, std::string> DeserializeAsset(const PackageEntry& entry) const
    {
        constexpr eResourceType expectedType = c_DataAssetResourceType<T>;
        if (entry.GetAssetType() != static_cast<zU32>(expectedType))
        {
            return UNEXPECTED("Несоответствие типа ассета. Ожидался: {}, в записи: {}",
                ToString(expectedType), entry.GetAssetType());
        }

        auto bufferRes = m_FileSystem->ReadBytes(eFileLocation::App, c_DataPackageRelativePath, entry.GetOffset(), entry.GetSize());
        if (!bufferRes)
            return UNEXPECTED("Не удалось прочитать блок данных '{}' из пакета '{}': {}",
                entry.GetName(), c_DataPackageRelativePath, bufferRes.error());

        const auto& buffer = *bufferRes;
        std::size_t offset = 0;
        Serializer serializer;
        T data{};
        auto res = serializer.Deserialize(buffer, offset, data);
        if (!res)
            return UNEXPECTED("Ошибка десериализации ресурса '{}': {}", entry.GetName(), res.error());

        return data;
    }
    ```
    Примечание: `DeserializeEntry<T>` (существующий приватный метод) можно оставить как есть или выразить через новый `DeserializeAsset<T>` после отдельного `GetEntry` — на усмотрение при реализации, план этого не фиксирует жёстко.

### Шаг 2. Контракт загрузчика IResourceLoader

- В `src/engine/resources/IResourceLoader.h`:
  - Исправить сигнатуру метода `Load`:
    ```cpp
    [[nodiscard]] virtual std::expected<std::shared_ptr<::zzz::core::IResource>, std::string> Load(
        const ::zzz::core::PackageEntry& entry,
        PackageManager& packageManager,
        ::zzz::core::DataAssetsManager& dataAssetsManager,
        ::zzz::core::FileSystem& fileSystem,
        GAPI& gapi) = 0;
    ```
  - Использовать правильные пространства имён: `PackageManager&` и `GAPI&` в `zzz::engine`, `::zzz::core::DataAssetsManager&`, `::zzz::core::FileSystem&`.

### Шаг 3. Ресурс Mesh и загрузчик MeshLoader

- В `src/engine/resources/Mesh.h` и `src/engine/resources/Mesh.cpp`:
  - Класс `Mesh`, наследующий `ResourceBase`.
  - Конструктор принимает `std::string name`, `core::MeshData meshData`.
  - Устанавливает состояние `eResourceState::Ready`.
  - Хранит геометрию на CPU в том же виде, что и источник `MeshData` (без домысливания несуществующих полей):
    - `m_VertexData: std::vector<std::byte>`, `m_VertexCount`, `m_VertexStride` — как в `MeshData`; интерпретация как `Vertex3D` возможна только при `vertexStride == sizeof(Vertex3D)` (проверяется `ensure`/`THROW_RUNTIME` при несовпадении, а не молчаливым reinterpret).
    - `m_IndexData: std::vector<std::byte>`, `m_IndexCount`, `m_IndexFormat: eIndexFormat` — индексы хранятся как есть, без безусловного расширения `UInt16` до `uint32_t`; конкретный тип индексного GPU-буфера (16 или 32 бита) выбирается в этапе 19 по `m_IndexFormat`.
  - Bounding box и сабмеши в `Mesh` не добавляются: bounding box — это этап 35 (AABB/Frustum Culling), а понятия "сабмеш" нет ни в `MeshData`, ни где-либо ещё в проекте — добавлять `MeshSubsetInfo` без формата-источника и потребителя нарушает правило 16 (YAGNI).
  - Предоставляет константные геттеры к этим данным для последующей отправки на GPU в этапе 19.
- В `src/engine/resources/MeshLoader.h` и `src/engine/resources/MeshLoader.cpp`:
  - Наследует `IResourceLoader`.
  - Метод `GetSupportedType()` возвращает `::zzz::core::eResourceType::Mesh`.
  - Метод `Load()`:
    - Вызывает `dataAssetsManager.DeserializeAsset<::zzz::core::MeshData>(entry)`.
    - При ошибке возвращает `std::unexpected(deserialized.error())`.
    - При успехе создает `auto mesh = safe_make_shared<Mesh>(entry.GetName(), std::move(*deserialized));` и возвращает `mesh`.

### Шаг 4. Доработка ResourceManager

- В `src/engine/resources/ResourceManager.h`:
  - Подключить `#include "core/templates/CallbackQueue.h"`.
  - Добавить псевдонимы `ResourceResult<T>`, `ResourceCallback<T>`, `AnyResourceResult`, `AnyResourceCallback`.
  - Добавить члены:
    ```cpp
    std::unordered_map<::zzz::core::Guid, std::vector<AnyResourceCallback>> m_InFlightCallbacks;
    ::zzz::templates::CallbackQueue<> m_MainThreadQueue;
    ```
  - Объявить метод `void Update();`.
  - Обновить сигнатуры `LoadAsync`:
    ```cpp
    template<typename T>
    void LoadAsync(const ::zzz::core::Guid& guid, ResourceCallback<T> onCompleted);

    template<typename T>
    void LoadAsync(const ::zzz::core::Guid& guid, std::function<void(std::shared_ptr<T>)> onLoaded);
    ```
- В `src/engine/resources/ResourceManager.cpp`:
  - В `EnqueueLoadRequest`: проверка кэша → регистрация в `m_InFlightCallbacks` → при первом запросе отправка в `m_RequestQueue`.
  - В `IoWorkerLoop`:
    - Проверка `ResourceStorageTraits`: если `DataArchive` — вызов `m_DataAssetsManager->GetEntry(req.type, req.guid)`, если `PackageArchive` — `m_PackageManager->GetEntry(req.guid)`.
    - Поиск подходящего `IResourceLoader` по типу.
    - Вызов `loader->Load(...)`.
    - При успехе: вызов `PublishResource(req.guid, resource)`.
    - Извлечение всех ожидающих колбэков из `m_InFlightCallbacks[req.guid]`.
    - Отправка каждого колбэка через `m_MainThreadQueue.Push([cb, result]() { cb(result); });`.
    - Декремент `m_ActiveRequests`.
  - В `PublishResource`: сохранение в `m_Meshes[guid]` и `m_MeshNames[resource->GetName()]`.
  - Реализация `AddMesh(std::shared_ptr<Mesh> mesh)`.
  - Реализация `Update()`: вызов `m_MainThreadQueue.ExecuteAll();`.
  - Реализация `Flush()`: цикл ожидания `m_ActiveRequests == 0` и вызов `m_MainThreadQueue.ExecuteAll();`.

### Шаг 5. Интеграция в Engine и регистрация в CMake

- В `src/engine/engine.cpp`:
  - В `Engine::Initialize()`: зарегистрировать `MeshLoader`:
    ```cpp
    m_ResourceManager->RegisterLoader(safe_make_unique<MeshLoader>());
    ```
  - В `Engine::OnUpdateSystem()`: добавить вызов обновления очереди ресурсов:
    ```cpp
    m_ResourceManager->Update();
    m_SceneManager->Update(*m_Time);
    ```
- В `src/engine/CMakeLists.txt`:
  - Добавить `resources/Mesh.h`, `resources/Mesh.cpp`, `resources/MeshLoader.h`, `resources/MeshLoader.cpp`.

## 5. Проверки и критерии готовности
 
- [x] `DataAssetsManager` компилируется с forward declaration `namespace zzz::engine { class ResourceManager; }` и `friend class ::zzz::engine::ResourceManager;`.
- [x] Метод `DataAssetsManager::DeserializeAsset<T>(entry)` компилируется и десериализует ассет за один проход по готовой записи.
- [x] Сигнатура `IResourceLoader::Load` использует точные пространства имён (`PackageManager&`, `GAPI&` в `zzz::engine`, `core::DataAssetsManager&`) и возвращает `std::expected`.
- [x] `IoWorkerLoop` маршрутизирует запросы на основе `ResourceStorageTraits` (`DataArchive` в `DataAssetsManager`, `PackageArchive` в `PackageManager`).
- [x] Реестр `m_InFlightCallbacks` предотвращает дублирование дисковых запросов при конкурентном вызове одного GUID.
- [x] Колбэки завершения загрузки безопасно пересылаются через `CallbackQueue<> m_MainThreadQueue` и исполняются на главном потоке кадра в `ResourceManager::Update()`.
- [x] Метод `Flush()` корректно дожидается завершения фоновых задач и доставляет колбэки через `ExecuteAll()`, устраняя риск дедлока в `LoadSync()`.
- [x] Используются только существующие состояния `eResourceState` (`Loading`, `Ready`, `Failed`).
- [x] Классы `Mesh` и `MeshLoader` компилируются; загрузчик восстанавливает `MeshData` из `data.dat` и создаёт объект `Mesh`.
- [x] `PublishResource` индексирует имя меша по `resource->GetName()`; метод `AddMesh` добавляет ресурс в кэш.
- [x] Вызов `m_ResourceManager->Update()` интегрирован в `Engine::OnUpdateSystem()`.
- [x] Таргеты `engine_lib` и `engine_lib_editor` успешно собираются без ошибок компилятора.

## 6. Не входит в этап

- Загрузка в GPU (создание `GPUBuffer`, выделение staging-буферов, запись команд копирования, `VkFence`/`SignalFence`, барьеры состояний, очередь отложенного освобождения staging-буферов) — **этап 19**.
- Текстуры (`Texture2D`), материалы (`Material`), шейдеры (`Shader`) — **этап 20**.
- Барьер готовности сцены и запуск скриптов — **этапы 20, 22**.
- Кадровые команды рендера и отрисовка (`DrawIndexed`) — **этап 23**.
- Модульные тесты в `src/qa/tests/` — не создаются по решению пользователя.
