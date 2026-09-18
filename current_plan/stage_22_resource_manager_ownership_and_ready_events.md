# Этап 22: Разделение ResourceManager (CPU/GPU), прямая типизация без IResourceLoader, снятие OwnerToken, события OneShotEvent и ResourceRecord

## 1. Контекст, статус и цель

- **Статус:** ⏳ Не начато
- **Откуда взялся этот этап:** при проектировании асинхронной загрузки и защите от use-after-free (ранее `OwnerToken`/`IsOwnerAlive`) проведена архитектурная ревизия подсистемы ресурсов:
  1. Ликвидирован оверинжиниринг в виде полиморфного `IResourceLoader`, мапы `m_Loaders`, стирания типов до `IResource` и внешней ручной регистрации загрузчиков в `Engine`. Загрузка переведена на прямое строго типизированное API с вызовом статических парсеров (`CpuMeshLoader::Load`, `CpuMaterialLoader::Load` и т.д.).
  2. Механизм владельческих токенов заменён на подписку через одноразовые события `OneShotEvent<Args...>` с проверкой `std::weak_ptr<void>` контекста подписчика.
  3. Подсистема ресурсов разделена на два независимых менеджера: `CpuResourceManager` (I/O, пакеты, VFS, десериализация) и `GpuResourceManager` (GAPI, видеопамять, staging, fence-готовность).
  4. Зафиксирован сквозной конвейер запросов: `GameObject` запрашивает `GpuResourceManager` (для рендера) -> `GpuResourceManager` при необходимости запрашивает сырые данные у `CpuResourceManager` -> `CpuResourceManager` асинхронно считывает данные с диска через `m_IoThread` -> по получении CPU-ресурса `GpuResourceManager` вызывает метод создания GPU-ресурса (`CreateGpuMesh` и др.) -> разрешает событие готовности `readyEvent` для `GameObject`.
  5. Проверена и зафиксирована очередь дискового ввода-вывода: переиспользуется готовый потокобезопасный шаблон ядра `DoubleBufferedVector<T>` со свапом буферов (без блокировок на чтении).
  6. Унифицировано хранение и дедупликация ресурсов через единый шаблон `ResourceRecord<T>`.
  7. Введена строгая префиксация типов: `CpuMesh`, `CpuMaterial`, `CpuTexture2D`, `CpuShader` на CPU и `GpuMesh`, `GpuMaterial`, `GpuTexture2D`, `GpuShader` на GPU.
  8. Заменённые решения этапа 16 помечены явной ссылкой в `stage_16_mesh_and_gpu_upload.md` (Правило 3).
- **Цель:**
  1. Полностью удалить `IResourceLoader`, `RegisterLoader()`, `m_Loaders`, `AnyResourceCallback` и runtime-касты указателей. Публичные методы загрузки работают напрямую со своими типизированными таблицами `ResourceRecord<T>`.
  2. Полностью удалить `OwnerToken` (`std::weak_ptr<const void>`), `IsOwnerAlive()` и все промежуточные параметры из сигнатур `Scene`, `GameLayer`, `ILayer`, `GameObject`, `ResourceManager`.
  3. Ввести низкоуровневый примитив ядра `OneShotEvent<Args...>` в `src/core/events/`:
     - Подписка по `std::weak_ptr<ContextType>` контексту.
     - Безопасное конкурентное добавление подписчиков (`std::mutex`) во время фоновой загрузки.
     - Однократный переход `Pending -> Resolved`.
     - **Late subscribers:** если ресурс уже готов (`Resolved`), колбэк **категорически не вызывается синхронно**, а безусловно ставится в `m_MainThreadQueue` (защита от гонок потоков `TaskDispatcher`, дедлоков под мьютексом таблицы и преждевременного обнуления `CountdownTrigger`).
     - Отсутствие окна гонки между `Subscribe` и `Resolve` под одним мьютексом события.
  4. Ввести шаблонную структуру `ResourceRecord<T>` в `src/engine/resources/`:
     - Объединяет кэшированный `std::shared_ptr<T>` и `OneShotEvent<std::expected<std::shared_ptr<T>, std::string>>`.
     - Заменяет собой раздельные кэши и таблицы `m_InFlightCallbacks`.
  5. Разделить ресурсы и менеджеры по доменным подпапкам:
     - `src/engine/resources/cpu/`: `CpuResourceManager`, `CpuMesh`, `CpuMaterial`, `CpuTexture2D`, `CpuShader` и статические загрузчики `loaders/`.
     - `src/engine/resources/gpu/`: `GpuResourceManager` (со связью через DI с `CpuResourceManager` и `GAPI`), `GpuMesh`, `GpuMaterial`, `GpuTexture2D`, `GpuShader`.
  6. Реализовать методы создания GPU-ресурсов в `GpuResourceManager`:
     - `LoadGpuMeshAsync`: запрашивает `CpuMesh` через `CpuResourceManager` -> по получении вызывает метод создания `CreateGpuMesh(cpuMesh)` -> вызывает `readyEvent.Resolve(gpuMesh)`.
     - Аналогичные пары методов для Material, Texture2D, Shader.
  7. Связать менеджеры в классе `Engine`:
     - `Engine` инстанциирует `CpuResourceManager`, передавая ему `PackageManager`, `DataAssetsManager` и `FileSystem` (с обновлением `friend class CpuResourceManager;` внутри них).
     - `Engine` инстанциирует `GpuResourceManager(m_GAPI, m_CpuResourceManager)`.
     - Проброс `CpuResourceManager` в `SceneManager` и домен `IEntityDomain`.
     - Проброс `GpuResourceManager` в `SceneManager` (для передачи в `GameObject`).
     - Публичные геттеры в `Engine` (`GetCpuResourceManager()`, `GetGpuResourceManager()`) и вызовы `Update()` в главном цикле кадра.
- **Границы:**
  - **Сборка мусора (Garbage Collection):** фоновая сборка неиспользуемых ресурсов (как на CPU, так и на GPU) исключена из скоупа этапа 22 и отложена до появления полноценного рендера и боевых ресурсов.
  - Настоящее выделение аппаратных `GPUBuffer` (DirectX 12 / Vulkan Vertex/Index buffers), upload-инфраструктура, барьеры и fenceValue/VkFence реализуются на этапе 23. На этапе 22 методы `CreateGpuMesh` и др. создают валидные GPU-объекты со ссылками на дескрипторы/данные и переводят их `readyEvent` в статус готовности.
  - Каскад готовности `GameObject -> GameLayer -> Scene -> SceneManager` на базе `CountdownTrigger` сохраняется; `GameObject` подписывается на `GpuResourceManager::LoadGpuMeshAsync` и `LoadGpuMaterialAsync` через `weak_from_this()`.

---

## 2. Архитектура и согласованная логика

### 2.1. Прямая типизация без `IResourceLoader`
- Публичные методы `CpuResourceManager` строго типизированы:
  - `LoadMeshAsync(guid, context, onLoaded)`
  - `LoadMaterialAsync(guid, context, onLoaded)`
  - `LoadTextureAsync(guid, context, onLoaded)`
  - `LoadShaderAsync(guid, context, onLoaded)`
- Логика работы каждого метода:
  1. Метод обращается к своей типизированной таблице (например, `m_CpuMeshes: unordered_map<Guid, ResourceRecord<CpuMesh>>`).
  2. Защита словарей таблиц: обращение к `std::unordered_map` защищено `std::shared_mutex` (shared-lock для чтения/поиска, unique-lock для добавления новой записи `ResourceRecord<T>`).
  3. Если запись уже есть: просто регистрирует подписчика через `record.readyEvent.Subscribe(context, onLoaded)`.
  4. Если записи нет: создаёт `ResourceRecord<CpuMesh>`, регистрирует подписчика и пушит типизированную задачу в очередь `m_RequestQueue` (`DoubleBufferedVector`).
  5. Фоновый I/O-поток читает данные с диска, парсит их через статический метод загрузчика (`CpuMeshLoader::Load`) и вызывает `record.readyEvent.Resolve(...)`.
- **Очередь со свапом (`DoubleBufferedVector`):**
  - Писатели (любые потоки) вызывают `Push(...)` под мьютексом очереди.
  - I/O-поток вызывает `SwapAndGetReadBuffer()`, мгновенно меняя буферы местами, и считывает задачи без удержания блокировки накопителя.

### 2.2. Запросы ресурсов в `GpuResourceManager`
- `GpuResourceManager` хранит `std::shared_ptr<CpuResourceManager> m_CpuManager` (DI).
- Таблицы `m_GpuMeshes`, `m_GpuMaterials`, `m_GpuTextures`, `m_GpuShaders` защищены своим `std::shared_mutex`.
- При вызове `LoadGpuMeshAsync(guid, context, onLoaded)`:
  1. Проверяет таблицу `m_GpuMeshes`. Если есть — подписывает `context`.
  2. Если нет — создаёт запись `ResourceRecord<GpuMesh>`, подписывает `context` и запрашивает:
     ```cpp
     m_CpuManager->LoadMeshAsync(guid, m_SelfWeak, [this, guid](auto result)
     {
         if (!result)
         {
             std::unique_lock lock(m_Mutex);
             m_GpuMeshes[guid].readyEvent.Resolve(std::unexpected(result.error()));
             return;
         }
         // Создание GPU-ресурса (на этапе 22 - формирование структуры, на этапе 23 - GAPI staging)
         auto gpuMesh = CreateGpuMesh(result.value());
         std::unique_lock lock(m_Mutex);
         m_GpuMeshes[guid].readyEvent.Resolve(std::move(gpuMesh));
     });
     ```
  3. Аналогично разделены уровни API для остальных ресурсов:
     - Публичный `LoadGpuMaterialAsync(...)` делегирует в `CpuResourceManager::LoadMaterialAsync(...)` и вызывает внутренний `CreateGpuMaterial(cpuMaterial)`.
     - Публичный `LoadGpuTextureAsync(...)` делегирует в `CpuResourceManager::LoadTextureAsync(...)` и вызывает внутренний `CreateGpuTexture(cpuTexture)`.
     - Публичный `LoadGpuShaderAsync(...)` делегирует в `CpuResourceManager::LoadShaderAsync(...)` и вызывает внутренний `CreateGpuShader(cpuShader)`.

### 2.3. Обработка ошибок и проброс исключений
- Если при чтении с диска или парсинге произошёл сбой, загрузчик формирует детальное текстовое сообщение (какой файл, смещение, причина сбоя) и возвращает `std::unexpected(message)`.
- Сообщение доставляется в `GameObject` на главном потоке.
- В текущей архитектуре куба (MVP) ресурсы обязательны: при получении ошибки `GameObject` выбрасывает исключение `THROW_RUNTIME("[GameObject '{}'] Критическая ошибка ресурса: {}", GetName(), result.error())`.
- В будущем это место позволяет перехватывать некритичные ошибки и устанавливать fallback-ресурсы без выброса исключения.

### 2.4. Устранение `OwnerToken`, `enable_shared_from_this` и подписка по `weak_ptr`
- `GameObject` наследует `std::enable_shared_from_this<GameObject>` (`class GameObject final : public std::enable_shared_from_this<GameObject>`).
- `ObjectDomain` переходит на хранение `std::vector<std::shared_ptr<GameObject>> m_Objects;` (создание через `safe_make_shared<GameObject>`). Все методы поиска по-прежнему возвращают быстрый сырой указатель `GameObject*`.
- Параметры `ownerToken` удаляются из `Scene::Initialize`, `GameLayer::Populate`, `ILayer`, `GameObject::Initialize`.
- Подписчик (`GameObject`) передаёт нативный `weak_from_this()` в `OneShotEvent`.
- При вызове колбэка на главном потоке проверяется `context.expired()`. Если объект уже уничтожен — колбэк безопасно пропускается (use-after-free исключён).

### 2.5. Примитив `OneShotEvent<Args...>`
- Живёт в `src/core/events/OneShotEvent.h` рядом с `Event.h` и `EventBus.h`.
- Потокобезопасен (`std::mutex`): подписчики могут добавляться параллельно из разных потоков `TaskDispatcher`, пока I/O-поток готовит результат.
- Однократный `Resolve(Args... args)` сохраняет результат и атомарно забирает (`std::move`) список подписчиков.
- Все уведомления подписчиков отправляются в очередь главного потока `m_MainThreadQueue`.
- **Подписка после `Resolved`:** безусловно маршалится в `m_MainThreadQueue` — никакой синхронный вызов прямо из `Subscribe` не допускается!
- **Отсутствие окна гонки:** под `std::mutex` подписчик либо успевает попасть в список `Pending`, либо видит статус `Resolved` и планируется в очередь.

### 2.6. Интеграция в `Engine` и зависимости
```cpp
// Конструктор Engine:
m_CpuResourceManager = safe_make_shared<CpuResourceManager>(m_PackageManager, m_DataAssetsManager, m_FileSystem);
m_CpuResourceManager->Start();

m_GpuResourceManager = safe_make_shared<GpuResourceManager>(m_GAPI, m_CpuResourceManager);

m_SceneManager = safe_make_shared<SceneManager>(*m_TaskDispatcher, m_PackageManager, m_CpuResourceManager, m_GpuResourceManager, m_ScriptFactory);
```

---

## 3. Изменяемые компоненты и файлы

1. `src/core/events/OneShotEvent.h` *(NEW)* — шаблонный примитив одноразового события ядра.
2. `src/engine/resources/ResourceRecord.h` *(NEW)* — шаблонная запись ресурса с кэшем и `OneShotEvent`.
3. `src/engine/resources/cpu/` *(NEW подпапка)*:
   - `CpuResourceManager.h/.cpp` — менеджер ресурсов CPU: изоляция от GAPI, удаление `OwnerToken`/`IsOwnerAlive`, удаление `IResourceLoader`, переход на прямые методы и `ResourceRecord<T>`, очередь со свапом `DoubleBufferedVector`, защита таблиц `std::shared_mutex`.
   - `CpuMesh.h/.cpp` — ресурс CPU-сетки.
   - `CpuMaterial.h/.cpp` — ресурс CPU-материала.
   - `CpuTexture2D.h/.cpp` — ресурс CPU-текстуры.
   - `CpuShader.h/.cpp` — ресурс CPU-шейдера.
   - `loaders/` — статические загрузчики: `CpuMeshLoader`, `CpuMaterialLoader`, `CpuShaderLoader`.
4. `src/engine/resources/gpu/` *(NEW подпапка)*:
   - `GpuResourceManager.h/.cpp` — менеджер GPU-ресурсов с DI (`GAPI` + `CpuResourceManager`), методами `CreateGpuMesh`, `CreateGpuMaterial`, `CreateGpuTexture`, `CreateGpuShader`, защита таблиц `std::shared_mutex`.
   - `GpuMesh.h/.cpp` — ресурс GPU-сетки.
   - `GpuMaterial.h/.cpp` — ресурс GPU-материала.
   - `GpuTexture2D.h/.cpp` — ресурс GPU-текстуры.
   - `GpuShader.h/.cpp` — ресурс GPU-шейдера.
5. `src/engine/resources/IResourceLoader.h` *(DELETE)* — полный демонтаж интерфейса.
6. `src/core/io/package/DataAssetsManager.h` — обновление `friend class ::zzz::engine::CpuResourceManager;`.
7. `src/engine/package/PackageManager.h` — обновление `friend class CpuResourceManager;`.
8. `src/engine/scene/domain/IEntityDomain.h`, `EntityDomain.h/.cpp` — замена параметра `ResourceManager&` на строго `CpuResourceManager&`.
9. `src/engine/engine.h/.cpp` — создание `CpuResourceManager` и `GpuResourceManager`, проброс зависимостей в `SceneManager`, вызовы `Update()` и геттеры.
10. `src/engine/scene/SceneManager.h/.cpp` — использование `CpuResourceManager` и `GpuResourceManager`, снятие токенов.
11. `src/engine/scene/Scene.h/.cpp` — удаление параметра `ownerToken` из `Initialize` и вызовов слоёв.
12. `src/engine/scene/layer/ILayer.h`, `GameLayer.h/.cpp`, `LayerMVVM.h/.cpp` — удаление `ownerToken` из `Populate`.
13. `src/engine/scene/gameobject/GameObject.h/.cpp` — наследование от `std::enable_shared_from_this<GameObject>`, запрос меша/материала через `GpuResourceManager`, подписка через `weak_from_this()`, снятие токена, выброс `THROW_RUNTIME` при ошибке.
14. `src/engine/scene/domain/ObjectDomain.h/.cpp` — переход на хранение `std::vector<std::shared_ptr<GameObject>> m_Objects;` (создание через `safe_make_shared<GameObject>`), поддержка `enable_shared_from_this`.
15. `current_plan/stage_16_mesh_and_gpu_upload.md` — пометка о замене решений этапа 16 решениями этапа 22 (Правило 3).
16. `src/engine/CMakeLists.txt` — обновление путей к исходным файлам подсистемы ресурсов.

---

## 4. Критерии приёмки

1. Проект компилируется через `run_build.bat` чисто, без ошибок и предупреждений компилятора (`MSVC /W4 /WX`).
2. `OwnerToken`, `IsOwnerAlive` и все их упоминания полностью удалены из кодовой базы ZzzEngine (Zero Technical Debt).
3. `IResourceLoader`, `RegisterLoader` и стирание типов до `IResource` удалены; загрузка происходит строго типизировано.
4. `CpuResourceManager` полностью изолирован от GAPI и платформенных графических зависимостей (Zero Platform Leaks).
5. `GpuResourceManager` создан с внедрением зависимостей (`GAPI` + `CpuResourceManager`), типами `GpuMesh`, `GpuMaterial`, `GpuTexture2D`, `GpuShader` и методами создания GPU-ресурсов (`CreateGpuMesh` и др.).
6. Таблицы ресурсов используют единый шаблон `ResourceRecord<T>` и примитив `OneShotEvent`, защищены `std::shared_mutex`.
7. Поздняя подписка (`Subscribe` после `Resolved`) гарантированно маршалится в `m_MainThreadQueue` без синхронного выполнения.
8. Очередь фонового ввода-вывода использует существующий шаблон ядра `DoubleBufferedVector` со свапом буферов.
9. `GameObject` запрашивает ресурсы в `GpuResourceManager`, а `GpuResourceManager` прозрачно подтягивает `CpuResourceManager`.
10. Каскадная инициализация сцены на `CountdownTrigger` и отложенный запуск `InvokeStart()` функционируют без регрессий.
11. Пользователь явно подтвердил приёмку («Шаг принят») перед обновлением статуса в `general_plan.md`.
