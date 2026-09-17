# Этап 52: Диспетчер задач с аппаратной адаптацией ядер (TaskDispatcher)

## 1. Контекст, статус и цель

- **Номер этапа:** 52 (Уровень 6: Apple/mobile и полировка).
- **Статус:** ⏳ В процессе
- **Цель:**
  1. Создать высокопроизводительный платформо-адаптивный диспетчер задач `TaskDispatcher` (`src/engine/tasks/`), устраняющий разрозненные локальные `ThreadPool` в сервисах движка (`SceneManager`, `ViewManager`) в пользу единого централизованного диспетчера CPU-задач под управлением `Engine`.
  2. Реализовать мгновенную маршрутизацию задач за $O(1)$ по приоритетам через массив пулов: `std::array<std::shared_ptr<ThreadPool>, static_cast<size_t>(eTaskPriority::Count)> m_Pools`.
  3. Реализовать сбор аппаратной топологии CPU через `CpuInfoCollector` под каждую платформу с сохранением результата в кроссплатформенную структуру `CpuTopology` (`src/core/hardware/CpuTopology.h`), встраиваемую в `HardwareState` (`src/core/hardware/HardwareState.h`). Для редактора (`CpuInfoCollectorEditor`) обеспечить опрос реального аппаратного количества ядер хоста (`std::thread::hardware_concurrency()`).
  4. Распределение ядер и квот:
     - **Desktop (Windows / Linux / macOS):** выделение 2 потоков `Critical` из первых доступных ядер, расчёт квот для `High`, `Normal`, `Background` по схеме `-2-1` в первую непустую ячейку кластера и `-1` в остальные для резерва системы;
     - **Mobile (Android / iOS):** жесткий лимит потокового бюджета воркеров `TaskDispatcher` ($\le 4$ воркеров суммарно, Mobile Hard Cap) для защиты от Thermal Throttling;
     - Прямая инициализация пулов из `CpuTopology` без раздувания лишних обёрток (CLI `--threads` исключён как YAGNI).
  5. Обеспечить корректный фазовый порядок завершения (Clean Shutdown):
     - В `TaskDispatcher` использовать метод `JoinAll()` и вызывать его при остановке подсистем в `Engine::Shutdown()`, а также в деструкторе `~TaskDispatcher()`;
     - В `ThreadPool` опираться на метод `Join()` (ожидание завершения очереди) и `Enqueue(task)`;
     - Опереться на существующий неблокирующий примитив ядра `CountdownTrigger` (`src/core/templates/CountdownTrigger.h`) для реактивного отслеживания завершения параллельных операций (`SceneManager`), а кадровые задачи `ViewManager` синхронизировать через `TaskDispatcher::Join(eTaskPriority::Critical)`.
- **Зависимости:**
  - `src/core/templates/ThreadPool.h` (базовый примитив пула воркеров);
  - `src/core/templates/CountdownTrigger.h` (реактивная синхронизация групп задач);
  - `src/core/hardware/HardwareState.h`, `src/core/hardware/CpuInfo.h` и `src/engine/platforms/hardware/HardwareManager.h`.
- **Отношение к другим этапам:** Этапы 36 (многопоточный culling) и 46 (асинхронная компиляция шейдеров) при их реализации будут использовать готовый централизованный `TaskDispatcher`.

---

## 2. Архитектура: Индексация по Enum и атомарная маршрутизация

### 2.1. Уровни задач, результат отправки и подсистемы

### 2.1. Разделение понятий: eTaskPriority, физические пулы и приоритеты ОС

Архитектура строго разделяет три независимых концепта:
1. **Логический приоритет задачи (`eTaskPriority`):** типизирует характер работы (`Critical = 0, High = 1, Normal = 2, Background = 3`). Клиенты оперируют только им.
2. **Физический пул (`ThreadPool`):** экземпляр пула потоков с фиксированным числом воркеров. Создаётся от 1 до 4 пулов в зависимости от наличия доступных ядер в кластерах CPU. Если какого-то типа ядер нет, массив указателей `m_Pools` маппирует соседние приоритеты на существующие пулы (каскадное копирование влево/вправо).
3. **Приоритет потока ОС (`eThreadPriority`):** уровень приоритета планировщика ОС (`Critical, High, Normal, Background`), задаваемый потокам пула при создании как мягкая рекомендация (`Priority Hint`).

Искусственные связи между перечислениями (вроде `static_assert` на равенство размеров) исключены: число логических приоритетов, число физических пулов и уровни приоритетов ОС независимы.

### 2.2. Контракт отправки задач и изоляция исключений

Метод `TaskDispatcher::Submit`:
```cpp
using ErrorHandler = std::function<void(std::exception_ptr)>;
void Submit(eTaskPriority priority, std::function<void()> task, ErrorHandler onError = nullptr);
```

1. **Контракт вызова:** Проверяет валидность функтора (`task`) и диапазон приоритета через `ensure`. Метод возвращает `void`. Синхронизация кадровых задач гарантируется контрактом вызова через `TaskDispatcher::Join(priority)`, а принятие задач в период работы движка обеспечивается контрактом жизненного цикла `Engine` (пулы открыты всегда, пока живы клиенты).
2. **Изоляция исключений (`safeTask`):** Задача оборачивается в защитную лямбду с блоком `try / catch (...)`. Поток-воркер никогда не падает аварийно.
3. **Локальный обработчик ошибок (`onError`):** При возникновении исключения вызывается переданная лямбда `onError(ex)`. Задача сама решает, как реагировать на сбой (например, пробросить исключение на главный поток через `Engine::DispatchToMainThread`, либо обработать локально). Если `onError == nullptr`, сбой логируется в `DOutError`.
4. **Panic Shutdown:** При необходимости кадровые клиенты через свой `onError` прокидывают исключение в очередь главного потока `Engine::DispatchToMainThread`. В начале кадра `Engine::OnUpdateSystem()` вызов `m_MainThreadQueue.ExecuteAll()` распаковывает исключение на главном потоке, инициируя штатный контролируемый `Shutdown()` движка с ожиданием GPU.

### 2.3. Синхронизация задач: `TaskDispatcher::Join(priority)` и `CountdownTrigger`

1. **Кадровая синхронизация окон (`ViewManager`):** `ViewManager::Update` отправляет задачи окон с приоритетом `Critical` и в конце кадра вызывает `m_TaskDispatcher.Join(eTaskPriority::Critical)`. Метод ожидает опустошения очереди и завершения задач данного пула.
2. **Реактивное отслеживание асинхронных операций (`SceneManager`):** Используется существующий шаблон ядра `CountdownTrigger`. Подзадачи по завершении вызывают декремент триггера, последний декремент реактивно выполняет финальный колбэк на главном потоке.

### 2.4. Принцип распределения пулов и копирование алиасов
 
 Диспетчер задач оперирует простым детерминированным алгоритмом:
 1. **CriticalPool (слот 0):** Гарантированно создаётся сразу на 2 потока (`CriticalWorker`, приоритет `eThreadPriority::Critical`).
 2. **Остальные пулы (слоты 1..3):** Создаются по квотам, рассчитанным из `CpuTopology`:
    - `HighWorker` (`counts[0]`, приоритет `eThreadPriority::High`);
    - `NormalWorker` (`counts[1]`, приоритет `eThreadPriority::Normal`);
    - `BackgroundWorker` (`counts[2]`, приоритет `eThreadPriority::Background`).
 3. **Копирование влево:** Если `High` или `Normal` не получили ядер, а справа есть живой пул, пустые слоты подтягивают его слева, не занимая `CriticalWorker`.
 4. **Копирование вправо:** Оставшиеся пустые правые слоты подтягивают ближайший существующий пул слева.
 
 ---
 
 ## 3. Распределение ядер, приоритеты ОС и потоковый бюджет (Zero Affinity Overhead)
 
 Расчёт выполняется однократно при создании `TaskDispatcher` в конструкторе `TaskDispatcher(const CpuTopology& topology)`:
 - Минимальное требование: 4 логических ядра.
 - Выделяется гарантированный кадровый пул `Critical` на 2 потока.
 - Для рабочих уровней (`High`, `Normal`, `Background`) рассчитываются квоты по кластерам:
   - Первая непустая ячейка: `-2` (Critical) и `-1` (резерв системы/MainThread);
   - Остальные ячейки: по `-1` (резерв системы).
 - На мобильных платформах учитывается Mobile Hard Cap ($\le 4$ воркеров).
 
 ### 3.2. Приоритеты ОС: Best-Effort Hints (Мягкие подсказки с безопасным фоллбэком)
 
 Выставление приоритетов потоков ОС является **рекомендацией планировщику (Best-effort hint)**, а не жестким контрактом:
 - **Windows:** Вызов `SetThreadPriority`.
 - **Linux / Android:** Вызов `pthread_setschedparam` / `setpriority`.
 - **Apple (iOS / macOS):** Установка QoS-класса.
 
 | Логический приоритет | Windows (`SetThreadPriority`) | Приоритет потока ОС (`eThreadPriority`) | Назначение |
 | :--- | :--- | :--- | :--- |
 | **`Critical`** (2 воркера) | `THREAD_PRIORITY_HIGHEST` (+2) | `eThreadPriority::Critical` | Срочные параллельные подзадачи кадра (`Update`, `ViewManager`) |
 | **`High`** | `THREAD_PRIORITY_ABOVE_NORMAL` (+1) | `eThreadPriority::High` | Срочные вычислительные задачи |
 | **`Normal`** | `THREAD_PRIORITY_NORMAL` (0) | `eThreadPriority::Normal` | Основные рабочие задачи (`SceneManager` загрузка слоёв) |
 | **`Background`** | `THREAD_PRIORITY_LOWEST` (-2) | `eThreadPriority::Background` | Фоновые задачи (декомпрессия, фоновый I/O) |
 
 ### 3.3. Сбор топологии через `CpuInfoCollector`
 - **Windows (`CpuInfoCollectorMSWin`):** `GetLogicalProcessorInformationEx(RelationProcessorCore, ...)` с анализом `EfficiencyClass`.
 - **Android (`CpuInfoCollectorAndroid`):** парсинг `/sys/devices/system/cpu/cpu*/cpu_capacity`.
 - **Apple (`CpuInfoCollectorApple`):** опрос уровней производительности через `sysctl`.
 - **Linux (`CpuInfoCollectorLinux`):** чтение `/sys/devices/system/cpu/cpu*/topology/`.
 - **Editor (`CpuInfoCollectorEditor`):** опрос реального аппаратного количества ядер хост-машины (`std::thread::hardware_concurrency()`).
 
 ---
 
 ## 4. Контракт API: `TaskDispatcher` (`src/engine/tasks/TaskDispatcher.h`)
 
 В соответствии с Правилом 16 (YAGNI) из контракта исключены неиспользуемые методы (`GetActiveWorkers`, `GetPendingTasks`).
 
 **Интерфейс и поля `TaskDispatcher` (`src/engine/tasks/TaskDispatcher.h`):**
 - Запрет копирования и перемещения (`Z_NO_COPY_MOVE`);
 - Конструктор от `const CpuTopology& topology` и деструктор `~TaskDispatcher()`;
 - Метод `Submit(priority, task, onError)`: приём задачи с приоритетом, целевой функцией и опциональным колбэком ошибки;
 - Метод `Join(priority)`: прямое ожидание завершения пула, назначенного на данный приоритет ($O(1)$ маршрутизация);
 - Метод `JoinAll()`: ожидание всех фактически созданных физических пулов циклом по массиву `m_Pools`;
 - Внутреннее поле:
   - Массив владения физическими пулами `m_Pools` (`std::array<std::shared_ptr<ThreadPool>, 4>`).
 
 ### 4.1. Расширение `ThreadPool` (`src/core/templates/ThreadPool.h`)
 В `ThreadPool`:
 1. **Настройка воркера при старте:**
    - Конструктор `ThreadPool` принимает `threadName`, `threadCount`, `priority` (`eThreadPriority`) и опциональный `onWorkerStart`.
    - Внутри `WorkerThread()` устанавливается имя потока (`SetCurrentThreadName`) и приоритет ОС (`SetCurrentThreadPriority`), защищённые `try/catch`.
 2. **Метод `Enqueue` (Fire-and-Forget):**
    - Метод `Enqueue(std::function<void()> task)`.
 3. **Штатный жизненный цикл:**
    - Метод `Join()` ожидает завершения текущих задач и опустошения очереди (`workQueue.empty() && activeThreadCount == 0`).
    - Безопасный деструктор `~ThreadPool()`.
    - Изоляция исключений задач обеспечивается обёрткой `safeTask` на уровне `TaskDispatcher::Submit` с вызовом локального `onError`.

---

## 6. Безопасность и порядок завершения (Clean Shutdown)
 
 Порядок завершения гарантирует отсутствие висячих задач и крашей при уничтожении сервисов:
 
 1. **Фаза 1 (Ожидание воркеров перед разрушением менеджеров):**
    - В начале `Engine::Shutdown()` (вызываемом из деструктора `Engine::~Engine()`) перед обнулением менеджеров вызывается метод:
      - `m_TaskDispatcher->JoinAll();`
    - Метод `JoinAll()` последовательно и гарантированно ожидает завершения всех фактически созданных физических пулов (`m_Pools`). Вызовы происходят в главном потоке кадра, что исключает гонки.
    - Это гарантирует, что все активные задачи сервисов полностью завершают выполнение. Ни один воркер больше не обратится к полям менеджеров по захваченному указателю, что на 100% исключает Use-After-Free.
 2. **Фаза 2 (Освобождение сервисов движка):**
    - После завершения задач воркеров менеджеры безопасно обнуляются в строгом порядке: `SceneManager`, `ViewManager`, `GAPI`, `Time`.
 3. **Фаза 3 (Освобождение диспетчера задач):**
    - Обнуляется `m_TaskDispatcher`: деструктор `~TaskDispatcher()` транзитивно вызывает `JoinAll()` и освобождает пулы потоков. Очищается `m_MainThreadQueue`.
 4. **Фаза 4 (Финальный сброс логов):**
    - `Logger` закрывается последним (автономный I/O поток).
 
 ### 6.1. Аварийный выход при критическом исключении воркера (Panic Shutdown)
 При возникновении сбоя в потоке-воркере:
 1. Поток не падает аварийным `std::terminate()`, а передаёт `std::exception_ptr` в лямбду `onError`, заданную для задачи;
 2. Ошибка отправляется в очередь главного потока через `Engine::DispatchToMainThread`;
 3. Главный поток в фазе `Engine::OnUpdateSystem()` вызывает `m_MainThreadQueue.ExecuteAll()`;
 4. Исключение распаковывается на главном потоке, кадровый цикл `MainLoop::Run()` прерывается, и `Engine::Run()` выполняет штатный контролируемый `Shutdown()` (Фазы 1–4) с очисткой ресурсов GAPI (`WaitForGpu`), предотвращая зависание драйвера и порчу данных.
 
 Именование воркеров через `SetCurrentThreadName`: длина имени строго ограничена (до 15 символов) во избежание ошибки `ERANGE` в `pthread_setname_np` на Linux/Android (например: `CriticalWorker`, `HighWorker`, `NormalWorker`, `BackgroundWorke`).
 
 ---
 
 ## 7. Чек-лист и порядок реализации
 
 1. **Расширение ядра (`src/core/templates/`):**
    - [x] В `ThreadPool.h`: метод `Enqueue(task)` и поддержка `eThreadPriority`.
    - [x] Создание `ThreadUtils.h/.cpp` для кроссплатформенной установки имён и приоритетов потоков ОС (с обрезкой имён до 15 символов).
 2. **Модель топологии CPU и аппаратное состояние:**
    - [x] Создать `src/core/hardware/CpuTopology.h` с полями logical capacity для каждого класса ядер (без избыточного `isHeterogeneous`);
    - [x] Встроить `CpuTopology` в `src/core/hardware/HardwareState.h`;
    - [x] Расширить `CpuInfoCollector` под Windows, Linux, Apple, Android;
    - [x] Обновить `CpuInfoCollectorEditor.h` для опроса реального числа ядер хоста (`std::thread::hardware_concurrency()`).
 3. **Класс `TaskDispatcher`:**
    - [x] Создать `src/engine/tasks/TaskDispatcher.h` и `src/engine/tasks/TaskDispatcher.cpp`;
    - [x] Метод `Submit(priority, task, onError)` с защитной лямбдой `safeTask`;
    - [x] Расчёт квот по кластерам (`-2-1` в первую непустую, `-1` в остальные);
    - [x] Реализовать каскадное копирование влево и вправо для отсутствующих пулов;
    - [x] Реализовать `Join(eTaskPriority priority)` за $O(1)$;
    - [x] Реализовать `JoinAll()` для штатной фазы ожидания всех воркеров.
 4. **Интеграция в `Engine` и клиенты:**
    - [x] Создать `m_TaskDispatcher` в `Engine.h/.cpp` и метод `DispatchToMainThread`;
    - [x] Вызывать `m_MainThreadQueue.ExecuteAll()` в начале кадра `Engine::OnUpdateSystem()`;
    - [x] В `Engine::Shutdown()` перед обнулением менеджеров вызвать `m_TaskDispatcher->JoinAll()`;
    - [x] Пробросить `TaskDispatcher&` в `SceneManager` и `ViewManager`;
    - [x] В `SceneManager` и `Scene` перевести загрузку слоёв на `TaskDispatcher&` (`eTaskPriority::Normal`) с реактивным `CountdownTrigger`;
    - [x] В `ViewManager` перевести кадровые задачи окон на `TaskDispatcher&` (`eTaskPriority::Critical`) и синхронизировать через `m_TaskDispatcher.Join(eTaskPriority::Critical)`;
    - [x] Исключить локальные `ThreadPool` из `SceneManager` и `ViewManager`.
 5. **Логирование:**
    - [x] Добавить вывод блока `[CpuTopology]` и `[TaskDispatcher] Workers Allocation` при старте.
 
 ---
 
 ## 8. Что НЕ входит в этап (YAGNI)
 
 - **Модификация user.dat / UserSettingsManager:** файл конфигурации пользователя не затрагивается; бюджет задаётся топологией CPU.
 - **CLI аргумент `--threads`:** исключён как YAGNI.
 - **Отмена задач из очереди (`Cancel` / `Drain`):** готовый `ThreadPool` полностью покрывает задачи движка через `Join()` и деструктор.
 - **Work-stealing между очередями:** пулы изолированы; P-ядра не воруют задачи E-ядер.
 - **Динамический граф задач (Task Graph с гранями зависимостей):** задачи независимы; синхронизация через `CountdownTrigger`, `TaskDispatcher::Join` и `CallbackQueue`.
 - **Акцессоры очередей (`GetActiveWorkers`, `GetPendingTasks`):** не реализуются до появления внутридвижкового UI-профайлера.
 
 ---
 
 ## 9. Затрагиваемые и новые файлы
 
 - **Новые файлы:**
   - `src/core/hardware/CpuTopology.h`
   - `src/engine/tasks/TaskPriority.h`
   - `src/engine/tasks/TaskDispatcher.h`
   - `src/engine/tasks/TaskDispatcher.cpp`
 - **Модифицируемые файлы:**
   - `src/core/CMakeLists.txt`
   - `src/core/templates/ThreadPool.h`
   - `src/core/hardware/HardwareState.h`
   - `src/core/utils/ThreadUtils.h` и `src/core/utils/ThreadUtils.cpp`
   - `src/engine/platforms/hardware/platforms/cpu/CpuInfoCollectorEditor.h`
   - `src/engine/platforms/hardware/platforms/cpu/CpuInfoCollectorMSWin.cpp` (и Linux/Apple/Android)
   - `src/engine/platforms/hardware/HardwareManager.h/.cpp`
   - `src/engine/engine.h` и `src/engine/engine.cpp`
   - `src/engine/scene/SceneManager.h` и `src/engine/scene/SceneManager.cpp`
   - `src/engine/scene/Scene.h` и `src/engine/scene/Scene.cpp`
   - `src/engine/view/ViewManager.h` и `src/engine/view/ViewManager.cpp`
   - `src/engine/CMakeLists.txt`
 
 ---
 
 ## 10. Критерии приёмки (Definition of Done — DoD)
 
 1. Код чисто компилируется без ошибок и предупреждений на текущем целевом роллаут-таргете (`run_build.bat` под Windows MSVC), а платформозависимый код под Linux/macOS/Android/iOS изолирован в специализированных файлах согласно Правилам 9–11.
 2. В консоль и лог-файл при старте выводится канонический блок `[CpuTopology]` и `[TaskDispatcher] Workers Allocation`.
 3. `SceneManager` и `ViewManager` не содержат локальных `ThreadPool`, а работают через переданный `TaskDispatcher&` на соответствующих приоритетах (`Normal` с `CountdownTrigger` и `Critical` с `TaskDispatcher::Join`).
 4. На системах с 4 логическими ядрами движок корректно стартует и не падает.
 5. Завершение приложения происходит чисто без deadlock'ов, висячих задач и Use-After-Free в потоках (ожидание `JoinAll` в `Engine::Shutdown()` перед обнулением менеджеров $\to$ деструктор `TaskDispatcher` $\to$ деструкторы `ThreadPool`).
 6. Никаких тестов не пишется и не запускается без явной команды пользователя (Правило 5 из `RULES.md`, Раздел 2 `GEMINI.md`).
 7. Этап принимается пользователем явно словом «Шаг принят».
