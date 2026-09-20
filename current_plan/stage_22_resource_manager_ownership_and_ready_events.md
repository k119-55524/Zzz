# Этап 22: Разделение ResourceManager (CPU/GPU), прямая типизация без IResourceLoader, снятие OwnerToken, события OneShotEvent, ResourceTable и трёхзвенный конвейер

## 1. Цель и скоуп этапа 22

- **Статус:** ⏳ В процессе (Активный этап разработки)
- **Цель этапа:** Реализовать строгую двухзвенную систему управления ресурсами (`CpuResourceManager` и `GpuResourceManager`), потокобезопасную дедупликацию через `ResourceTable<T>`, трёхзвенный асинхронный конвейер (дисковый I/O $\to$ десериализация на пуле $\to$ GPU RAM-заглушка $\to$ колбэк на главном потоке) и корректный жизненный цикл для гарантированной загрузки стартовой сцены (куб).

### 1.1. Ключевые архитектурные инварианты Этапа 22
1. **Строгая типизация без стирания типов:** Полное отсутствие полиморфного `IResourceLoader`, `m_Loaders`, `AnyResourceCallback` и runtime-кастов. Загрузка типизирована: `CpuMesh`, `CpuMaterial`, `CpuTexture2D`, `CpuShader` на CPU и `GpuMesh`, `GpuMaterial`, `GpuTexture2D`, `GpuShader` на GPU.
2. **Безопасная подписка по `weak_ptr`:** Механизм `OwnerToken` полностью удалён. `GameObject` наследует `std::enable_shared_from_this<GameObject>` и передаёт `weak_from_this()` в `OneShotEvent`. Если подписчик уничтожен — колбэк безопасно пропускается.
3. **Zero User Code Under Lock:** Никакой внешний код (подписчики `onLoaded`, инициирующий `onLoadRequest`, постановка задач в `IoScheduler`/диспетчеры) не исполняется под блокировками таблиц ресурсов. Замки $L1$ всегда снимаются до вызова внешних функций.
4. **Семантика `Ready` и честное время жизни памяти:** Сигнал готовности на этапе 22 означает **исключительно готовность C++ объекта-заглушки в RAM, а не GPU residency**. Вызовы GAPI из фоновых воркеров категорически запрещены. `CpuMesh` — это промежуточное представление данных пакета в RAM перед созданием GPU-ресурса. В этапе 22 `CpuMesh` честно остаётся в таблице `ResourceTable<CpuMesh>` до shutdown приложения (`Clear()`). Мы не обещаем его выгрузку простым `m_CpuMesh.reset()` на этапе 22 (так как запись кэша удерживает `std::shared_ptr`). Реальная политика выгрузки данных сетки из системной RAM будет решена на этапе 23 вместе с GPU upload. Физические меши (коллизии, convex hulls) изолированы от графических и управляются отдельно через `CpuResourceManager` (TODO 31, [`docs/ARCHITECTURE.md §31`](../docs/ARCHITECTURE.md)).
5. **Гибридный подсчёт ссылок:** `std::shared_ptr<T>` управляет физическим временем жизни в кэше. `ResourceRef<T>` управляет внешними движковыми ссылками через атомарный счётчик `ResourceBase::m_ExternalRefCount`. Кэш не инкрементирует внешний счётчик, поэтому число внешних ссылок равно строго `GetExternalRefCount()`.
6. **Ограничение памяти в полёте (Backpressure):** Лимиты байтов (`m_MaxInFlightBytes`) и запросов (`m_MaxInFlightRequests`) в полёте, пакетное ограничение (Batch Drain Limit) и RAII-токен `InFlightPermit` предотвращают неконтролируемое накопление сырых буферов в памяти.
7. **Детерминированный Shutdown:** Строгая последовательность в `Engine::Destroy`: остановка I/O $\to$ `JoinAll()` воркеров $\to$ сброс completion queues без исполнения $\to$ `WaitForGpu()` $\to$ очистка таблиц ресурсов $\to$ уничтожение менеджеров.
8. **Защита дисковых зон:** `IsLocationWritable(location)` по принципу явного allowlist (`User`, `Saves`, `Cache`, `Logs`) и защита от выхода за пределы директории (Path Traversal `..`) в `FileSystemBase::WriteAllBytes`.
9. **Контракт гарантированного разрешения в Running (No-Hang Contract):** Пока движок находится в состоянии `Running`, любой запрос в `ResourceTable<T>`, перешедший в состояние `Pending`, **гарантированно обязан завершиться вызовом `Resolve`** — либо валидным ресурсом, либо `std::unexpected(errorMessage)`. Ошибки чтения с диска, парсинга, повреждения файлов, сбои постановки задач в `TaskDispatcher` и любые C++ исключения транслируются в `std::unexpected`, исключая «вечный лоадинг» и зависание барьеров инициализации сцены (`AsyncInitTracker` / `CountdownTrigger`). При остановке движка (`Engine::Destroy()`, состояние `Destroying`) контракт No-Hang уступает место детерминированному shutdown: незавершённые фоновые задачи и очереди колбэков просто отменяются и очищаются без исполнения.

### 1.2. Осознанные упрощения MVP (Точки расширения будущих этапов)
Для исключения оверинжиниринга и избыточного кода на этапе куба зафиксированы следующие упрощения (архитектура оставляет для них точки расширения, но не реализует сейчас):
- **Без `Invalidate/Retry` и без поколений (`generation`):** Записи в `ResourceTable` неизменяемы и не удаляются до shutdown приложения (`Clear()`). Поддержка отмены, перезагрузки ресурсов и версионирования поколений отложена до этапа 50 (Hot Reload / Отмена запросов).
- **Без постоянно открытых дескрипторов файлов:** Доступ к архивам безопасен и стандартизован; persistent file handles отложены до замеров производительности (этап 58).
- **Без `Priority Promotion` и `Fair Scheduling`:** Используется стандартная двухприоритетная очередь `High` $\to$ `Background`.
- **Без аппаратного `GpuUploadScheduler`:** Реальный upload через Copy Queue, барьеры и fences реализуются в этапе 23.
- **Без шардирования `ResourceTable`:** Единый `std::shared_mutex` с Read-First поиском достаточен для базовых сцен; шардирование (Striped Locks) зафиксировано в TODO 27 (этап 58).
- **Без `ExecuteBudgeted`:** Главный поток на старте сбрасывает все колбэки через `ExecuteAll()`. Бюджетирование кадра вынесено в этап 50/58 для открытого мира.
- **Без сборщика мусора (GC):** Фоновая выгрузка ресурсов исключена из этапа 22.

---

## 2. Архитектура и сквозной конвейер

### 2.1. Потоковая схема конвейера (Pipelined Streaming)
```
[1. ЗАПРОС ВНИЗ]
GameObject (Main Thread)
  │  GpuResourceManager::GetAsync<GpuMesh>(guid, weak_this, onLoaded, eTaskPriority::High)
  │    └─ ResourceTable<GpuMesh>::GetOrRequest (статус: Pending, подписчик зарегистрирован)
  ▼
CpuResourceManager::GetAsync<CpuMesh>(guid, onLoadRequest, eTaskPriority::High)
  │    └─ ResourceTable<CpuMesh>::GetOrRequest (статус: Pending)
  ▼
[2. ДИСКОВЫЙ ВВОД-ВЫВОД С BACKPRESSURE (Выделенный поток IoScheduler)]
IoScheduler Thread (выделенный m_IoThread, последовательное чтение с диска):
  │  1. Контроль лимитов Backpressure (m_MaxInFlightBytes, m_MaxInFlightRequests)
  │  2. Чтение сырых байт из архива через FileSystem::ReadBytes
  │  3. Создание RAII-токена InFlightPermit
  ▼  4. Передача (std::vector<std::byte>, InFlightPermit) в onIoComplete
[3. CPU ДЕСЕРИАЛИЗАЦИЯ (Воркер TaskDispatcher)]
TaskDispatcher Worker (Пул воркеров, приоритет eTaskPriority::Normal / High):
  │  1. CpuMeshLoader::LoadFromMemory(entry, bytes) -> парсинг CPU-структуры
  │  2. Освобождение InFlightPermit (возврат квоты в IoScheduler)
  ▼  3. CpuResourceManager::GetTable<CpuMesh>().Resolve(guid, std::move(cpuMesh))
[4. GPU PREPARATION STUB (Фоновый воркер TaskDispatcher)]
TaskDispatcher Worker (колбэк по готовности CpuMesh, спланированный диспетчером CpuRM на пул воркеров):
  │  1. GpuResourceBuilder<GpuMesh>::Build(std::move(cpuMesh)) -> создание RAM-заглушки GpuMesh
  │     (ТЯЖЁЛАЯ СБОРКА ВЫПОЛНЯЕТСЯ СТРОГО НА ВОРКЕРЕ ПУЛА, А НЕ НА ГЛАВНОМ ПОТОКЕ!)
  │     (удерживает ResourceRef<CpuMesh> до этапа 23, где будет реализована политика освобождения после upload)
  ▼  2. GpuResourceManager::GetTable<GpuMesh>().Resolve(guid, std::move(gpuMesh))
[5. ОПОВЕЩЕНИЕ ВВЕРХ (Главный поток)]
Main Thread (колбэк запланирован диспетчером GpuRM через CallbackQueue):
  │  Вызов легковесного колбэка GameObject::OnMeshLoaded(result)
  ▼  CountdownTrigger::CountDown() -> сцена переходит в Ready!
```

> **Примечание по честному времени жизни памяти и разделению систем:**
> - `GpuResourceManager` обслуживает исключительно графические ресурсы. Физические ресурсы (коллизии, упрощённые сетки, примитивы PhysX/Jolt) изолированы от тяжёлых графических вертексов (TODO 31).
> - На этапе 22 дедупликация в `ResourceTable<GpuMesh>` гарантирует однократное чтение при множественных запросах сцены.
> - На этапе 22 `CpuMesh` честно остаётся в таблице `CpuResourceManager::ResourceTable<CpuMesh>` в системной RAM до shutdown приложения (`Clear()`). Реальное освобождение системной памяти после заливки в VRAM будет спроектировано и реализовано на этапе 23 вместе с конвейером GPU upload.

---

### 2.2. Архитектура `ResourceTable<T>`, Zero User Code Under Lock и Thread Affinity

`ResourceTable<T>` хранит `std::unordered_map<Guid, std::shared_ptr<OneShotEvent<ResultType>>>` и защищена `std::shared_mutex m_Mutex`.

#### 1. Метод `GetOrRequest`:
1. **Фаза 1 (под $L1$-замком таблицы):**
   - Read-First поиск под `std::shared_lock`: если событие уже существует, извлекаем `shared_ptr<OneShotEvent>` и `inserted = false`.
   - Если нет — берём `std::unique_lock`, выполняем `try_emplace` нового `safe_make_shared<OneShotEvent<ResultType>>()`, фиксируем `inserted = true`.
2. **Фаза 2 (замки таблицы $L1$ полностью сняты):**
3. **Фаза 3 (под $L2$-мьютексом события):**
   - Вызываем `eventPtr->Subscribe(context, onLoaded, m_Dispatcher)`. Если ресурс уже готов (`Resolved`), колбэк планируется в очередь через переданный `m_Dispatcher`.
4. **Фаза 4 (ВНЕ ВСЕХ ЗАМКОВ):**
   - Если `inserted == true`, вызываем `onLoadRequest(guid)` вне каких-либо мьютексов.
   - Постановка задачи на чтение в `IoScheduler` и передача запроса происходят при полностью снятых замках таблицы ресурсов, исключая любые межпоточные блокировки.

#### 2. Метод `Resolve`:
1. Под `std::shared_lock` ищет запись `find(guid)`.
2. Если запись отсутствует (ресурс был очищен) — результат отбрасывается. **`try_emplace` категорически не вызывается!**
3. Если запись найдена — извлекается `eventPtr`, замок таблицы $L1$ отпускается, и вызывается `eventPtr->Resolve(std::move(result), m_Dispatcher)` полностью вне замка таблицы.

#### 3. Разделение диспетчеров (Thread Affinity Contract):
- **`CpuResourceManager`:** инициализирует свои таблицы `ResourceTable<TCpu>` диспетчером **пула воркеров (`TaskDispatcher::Submit`)**.
  - Когда десериализация `CpuMesh` завершена и вызывается `CpuResourceManager::Resolve`, оповещение подписчика (`GpuResourceManager`) планируется на фоновый воркер `TaskDispatcher`.
  - Благодаря этому операция `GpuResourceBuilder<GpuMesh>::Build` (аллокация буферов, сборка меша в RAM) гарантированно выполняется **на фоновом потоке пула воркеров**, не создавая фризов и пауз на главном потоке.
- **`GpuResourceManager`:** инициализирует свои таблицы `ResourceTable<TGpu>` диспетчером **главного потока (`MainThreadQueue`)**.
  - Когда `GpuResourceBuilder::Build` завершён и вызывается `GpuResourceManager::Resolve`, оповещение подписчика (`GameObject::OnMeshLoaded`) отправляется в очередь главного потока.
  - На главном потоке исполняется только финальный лёгкий колбэк (сохранение `ResourceRef<GpuMesh>` и вызов `CountdownTrigger::CountDown()`).

---

### 2.3. Маршрутизация задач, IoScheduler и проброс приоритетов

1. **Модель исполнения задач и Backpressure (IoScheduler):**
   - Дисковый ввод-вывод изолирован в приватном `IoScheduler` внутри `CpuResourceManager`.
   - Выделенный поток ввода-вывода `m_IoThread` последовательно читает данные с диска без конкуренции головок/дескрипторов.
   - Механизм Backpressure (`InFlightPermit`) ограничивает суммарный объём байт (по умолчанию 64 МБ) и количество запросов (128) в полёте: если лимит превышен, `m_IoThread` засыпает на `m_BackpressureCv` до завершения десериализации предыдущих ресурсов на воркерах `TaskDispatcher`.
   - `GpuResourceManager` **полностью изолирован от диска** — он запрашивает типизированные данные у `CpuResourceManager::GetAsync<CpuMesh>`.
   - Вся дисковая работа, доступ к `DataAssetsManager` / `PackageManager` сосредоточены в `CpuResourceManager` и `IoScheduler`.

2. **Проброс приоритетов (`eTaskPriority`):**
   - Стартовая загрузка первой сцены окна инициируется из `ViewManager::SetInitialSceneAsync(view, sceneGuid, eTaskPriority::High)`.
   - Приоритет `eTaskPriority::High` сквозным образом транслируется через `SceneManager::LoadSceneAsync` $\to$ `Scene::Initialize` $\to$ `GameLayer::Populate` $\to$ `GameObject::Initialize`.
   - Все последующие загрузки и фоновые сцены используют стандартный/фоновый приоритет (`Normal` / `Background`).
   - `GpuResourceManager` транслирует приоритет запроса в `CpuResourceManager::GetAsync`, а затем использует его же при постановке задачи сборки GPU-ресурса.
   - `CpuResourceManager` передаёт `priority` в `IoScheduler::QueueRead` и в `m_TaskDispatcher.Submit(priority, ...)`, направляя задачу в соответствующий физический пул воркеров (`Critical`, `High`, `Normal`, `Background`).

3. **Контракт гарантированного разрешения (No-Hang в состоянии Running):**
   - Метод `TaskDispatcher::Submit` возвращает `bool` (`true` при принятии в очередь, `false` при закрытом пуле).
   - Если `Submit` вернул `false`, менеджер немедленно разрешает запрос вызовом `Resolve(guid, std::unexpected(...))`.
   - Любые ошибки чтения, парсинга или брошенные C++ исключения перехватываются через `try ... catch` и транслируются в `Resolve`, исключая зависание `Pending`.

---

### 2.4. Детерминированный порядок Shutdown в `Engine::Destroy()`

Строгая последовательность для исключения гонок и дедлоков:
1. Запретить новые запросы (`engineState = Destroying`).
2. `m_CpuResourceManager->Stop()` (останавливает `m_IoScheduler`, посылает `request_stop()`, `m_IoCv.notify_all()`, `m_IoThread.join()`).
3. `m_TaskDispatcher->JoinAll()` (завершение всех активных воркеров пула).
4. Очистка всех очередей завершения (resource completion queues и `m_MainThreadQueue`) **без исполнения колбэков**.
5. `m_GAPI->WaitForGpu()` (до очистки ресурсов).
6. Очистка таблиц ресурсов: `m_GpuResourceManager->Clear()`, `m_CpuResourceManager->Clear()`.
7. Уничтожение менеджеров и подсистем.

---

## 3. Изменяемые компоненты и файлы

1. `src/core/enums/eFileLocation.h` / `src/core/io/FileSystemBase.cpp` — проверка прав на запись `IsLocationWritable(location)` по allowlist (`User`, `Saves`, `Cache`, `Logs`) и защита от Path Traversal.
2. `src/core/io/package/DataAssetsManager.h/.cpp` и `PackageManager.h/.cpp` — методы `ReadRawBytes(entry)` и методы десериализации из памяти `DeserializeAssetFromMemory<T>` / `DeserializeEntryFromMemory<T>`.
3. `src/engine/resources/ResourceTable.h` — потокобезопасная таблица ресурсов: Read-First, Zero User Code Under Lock, единый `m_Dispatcher`, `Resolve` без `try_emplace`.
4. `src/engine/resources/ResourceBase.h` — переименование счётчика в `m_ExternalRefCount` и геттера в `GetExternalRefCount()`.
5. `src/engine/resources/io/IoScheduler.h/.cpp` *(NEW)* — приватный дисковый планировщик внутри `CpuResourceManager`: инкапсулирует выделенный `m_IoThread`, очереди `DoubleBufferedVector<IoReadRequest>`, `InFlightPermit` и `Stop()`.
6. `src/engine/resources/cpu/CpuResourceManager.h/.cpp` — владение `IoScheduler`, чтение архивов и координация CPU-ресурсов, методы `Stop()` и `Clear()`.
7. `src/engine/resources/cpu/loaders/` (`CpuMeshLoader`, `CpuMaterialLoader`, `CpuShaderLoader`) — статические методы `LoadFromMemory`.
8. `src/engine/resources/gpu/GpuResourceManager.h/.cpp` — приём `CpuResourceManager&` через DI, оркестрация конвейера через `ResourceTable<TGpu>`, изоляция от диска, вызов `GpuResourceBuilder`.
9. `src/engine/resources/gpu/GpuResourceBuilder.h` — трейты создания объектов-заглушек в системной памяти (`GpuResourceBuilder<TGpu>::Build`).
10. `src/engine/resources/gpu/GpuMesh.h` — фасад-заглушка с удержанием `ResourceRef<CpuMesh>`.
11. `src/engine/engine.h/.cpp` — владение `m_CpuResourceManager` и `m_GpuResourceManager`, порядок остановки в `Engine::Destroy()`.
12. `src/engine/scene/gameobject/GameObject.cpp` — асинхронный запрос `GpuMesh` и передача ошибок через `AsyncInitTracker::NotifyError`.
13. `src/engine/tasks/TaskDispatcher.h/.cpp` — метод `Submit` возвращает `bool` (`true` при `eEnqueueResult::Accepted`, `false` при `Closed`/ошибке) для явной обработки отказа постановки задачи в пулы потоков.

---

## 4. Пошаговая реализация (5 подшагов)

### **Подшаг 1: Защита дисковых зон, сырое чтение пакетов и парсеры данных (LoadFromMemory)**
- В `src/core/enums/eFileLocation.h`: добавить allowlist `IsLocationWritable(location)`.
- В `src/core/io/FileSystemBase.cpp`: добавить проверку allowlist и защиту от Path Traversal (`..`) в `WriteAllBytes`.
- В `DataAssetsManager` и `PackageManager`: реализовать `ReadRawBytes(entry)` и методы десериализации из памяти `Deserialize...FromMemory`.
- В `CpuMeshLoader`, `CpuMaterialLoader`, `CpuShaderLoader`: реализовать статические методы десериализации структуры данных из памяти `LoadFromMemory(entry, std::span<const zU8> bytes)`.
- **Верификация:** Компиляция `run_build.bat game_win`.

### **Подшаг 2: Таблица ресурсов `ResourceTable<T>` и счётчик ссылок `ResourceBase`**
- В `ResourceTable.h`:
  - `GetOrRequest`: Read-First под `shared_lock`, вставка под `unique_lock`, снятие замков перед `Subscribe` и `onLoadRequest` (Zero User Code Under Lock).
  - `Resolve`: поиск под `shared_lock` **строго без `try_emplace`**, снятие замка таблицы перед вызовом `eventPtr->Resolve`.
  - Интеграция `TaskDispatcher` / `CallbackQueue` через фабричный конструктор.
- В `ResourceBase.h`:
  - Переименовать `m_RefCount` $\to$ `m_ExternalRefCount`, `GetRefCount()` $\to$ `GetExternalRefCount()`.
- **Верификация:** Компиляция `run_build.bat game_win`.

### **Подшаг 3: Асинхронная загрузка через `TaskDispatcher` и `CpuResourceManager`**
- В `src/engine/tasks/TaskDispatcher.h/.cpp`:
  - Правка `Submit`: возвращает `bool` (`true` при `eEnqueueResult::Accepted`, `false` при `Closed` / ошибке).
- В `CpuResourceManager`:
  - Методы `GetAsync<T>` принимают `eTaskPriority priority = eTaskPriority::Normal`.
  - Реализация `DispatchLoad<T>(guid, priority)`: отправка задачи в `m_TaskDispatcher.Submit(priority, ...)`.
  - Воркер пула вызывает `LoadResourceSync<T>(guid)` (чтение байт + `LoadFromMemory`) $\to$ `CpuResourceManager::GetTable<T>().Resolve(...)`.
  - Обязательный `try ... catch` в задачах воркера и проверка результата `TaskDispatcher::Submit` с трансляцией исключений, дисковых ошибок и отказов очереди в `Resolve(guid, std::unexpected(...))`.
  - Методы `Stop()` и `Clear()`.
- **Верификация:** Компиляция `run_build.bat game_win`.

### **Подшаг 4: `GpuResourceBuilder` и конвейер `GpuResourceManager`**
- Создать `GpuResourceBuilder.h` со специализацией `Build` для меша, текстуры, материала, шейдера (создание RAM-заглушек без вызовов GAPI; `GpuMesh` удерживает `ResourceRef<CpuMesh>` до этапа 23).
- В `GpuResourceManager`:
  - Приём `CpuResourceManager&` через конструктор (DI). Полная изоляция от диска.
  - Типизированные `ResourceTable<TGpu>` (дедупликация запросов сцены по GUID).
  - Метод `GetAsync<TGpu>` с параметром `priority`: проверяет таблицу `ResourceTable<TGpu>` $\to$ запрашивает `CpuResourceManager::GetAsync<TCpu>(guid, weak_from_this(), ..., priority)` $\to$ по готовности CPU-данных воркер вызывает `GpuResourceBuilder::Build` $\to$ `GpuResourceManager::GetTable<TGpu>().Resolve`.
  - Гарантия No-Hang: задача воркера обёрнута в `try ... catch`, любая ошибка или отказ постановки транслируется в `Resolve(guid, std::unexpected(err))`.
  - Методы `Stop()` и `Clear()`.
- **Верификация:** Компиляция `run_build.bat game_win`.

### **Подшаг 5: Детерминированный Shutdown, связка с `GameObject` и финальная проверка**
- В `Engine` (`src/engine/engine.h/.cpp`):
  - Владение `m_CpuResourceManager` и `m_GpuResourceManager` (с передачей `*m_CpuResourceManager` в `GpuResourceManager`).
  - В `Engine::Destroy()`: строгий порядок остановки (`m_CpuResourceManager->Stop()` $\to$ `TaskDispatcher::JoinAll()` $\to$ очистка completion queues без исполнения $\to$ `WaitForGpu()` $\to$ `Clear()` таблиц $\to$ уничтожение менеджеров).
- В `GameObject` (`src/engine/scene/gameobject/GameObject.cpp`):
  - Инициализация меша и материала через `m_GpuResourceManager.GetAsync<GpuMesh>` и `m_GpuResourceManager.GetAsync<GpuMaterial>`.
  - Проверка сквозной цепочки: `GameObject` $\to$ `GpuResourceManager` $\to$ `CpuResourceManager` $\to$ `TaskDispatcher` (I/O + десериализация + GPU-сборка) $\to$ Main Thread `OnMeshLoaded` $\to$ `CountdownTrigger::CountDown()`.
- **Финальная верификация:** Запуск тестовых наборов `run_build.bat game_win`, `run_build.bat editor_dll`, `run_build.bat tests`.

---

## 5. Критерии приёмки

1. Проект компилируется через `run_build.bat` чисто (`MSVC /W4 /WX`, 0 ошибок, 0 предупреждений).
2. Полное отсутствие `OwnerToken`, `IsOwnerAlive`, `IResourceLoader` и стирания типов в кодовой базе.
3. `CpuResourceManager` полностью изолирован от GAPI.
4. `GpuResourceManager` использует `GpuResourceBuilder` и не делает вызовов GAPI из воркеров `TaskDispatcher`.
5. Строгое соблюдение Zero User Code Under Lock: отсутствие вызовов пользовательского кода под мьютексом таблицы ресурсов.
6. Отсутствие создания отсутствующих записей (`try_emplace`) в `Resolve`.
7. Детерминированный порядок остановки в `Engine::Destroy()` (без исполнения callbacks после `JoinAll`).
8. Соблюдение контракта No-Hang в состоянии Running: любой запрос в состоянии Running гарантированно разрешается (успехом или `std::unexpected`), исключая зависание `OneShotEvent` (при shutdown незавершённые запросы отменяются без исполнения callbacks).
9. Каскадная инициализация сцены на `CountdownTrigger` и отложенный запуск `InvokeStart()` функционируют штатно.
10. Пользователь явно подтвердил приёмку («Шаг принят»).
