# Этап 51: Диспетчер задач с аппаратной адаптацией ядер (TaskDispatcher)

## 1. Контекст, статус и цель

- **Номер этапа:** 51 (Уровень 6: Apple/mobile и полировка).
- **Статус:** ⏳ Не начато (Предварительный план)
- **Цель:**
  1. Создать высокопроизводительный платформо-адаптивный диспетчер задач `TaskDispatcher` (`src/engine/tasks/`), устраняющий разрозненные локальные `ThreadPool` в сервисах движка (`SceneManager`, `ViewManager`) в пользу единого централизованного диспетчера CPU-задач под управлением `Engine`.
  2. Реализовать мгновенную маршрутизацию задач за $O(1)$ по приоритетам через индексацию enum в плоский массив указателей на пулы: `std::array<ThreadPool*, static_cast<size_t>(eTaskPriority::Count)> m_PoolRouting`.
  3. Реализовать сбор аппаратной топологии CPU через `CpuInfoCollector` под каждую платформу с сохранением результата в кроссплатформенную структуру `CpuTopology` (`src/core/hardware/CpuTopology.h`), встраиваемую в `HardwareState` (`src/core/hardware/HardwareState.h`). Для редактора (`CpuInfoCollectorEditor`) обеспечить реалистичный опрос ядер хоста (`std::thread::hardware_concurrency()`).
  4. Реализовать платформенную политику `PlatformTaskPolicy` (`src/engine/platforms/task/`):
     - **Desktop (Windows / Linux / macOS):** высокая степень параллелизации с адаптивным бюджетом воркеров по ядрам;
     - **Mobile (Android / iOS):** жесткий лимит потокового бюджета собственных воркеров `TaskDispatcher` ($\le 3\text{--}4$ воркера на процесс) для защиты от Thermal Throttling и высаживания батареи;
     - Учёт аргументов командной строки с чётким приоритетом: CLI (`--threads=N`) $\to$ Hardware Auto Policy (с безусловным Mobile Hard Cap на мобильных платформах; `user.dat` не затрагивается согласно YAGNI).
   5. Обеспечить корректный фазовый порядок завершения (Clean Shutdown):
      - В `ThreadPool` опереться на существующий метод `Join()` и деструктор пула, а метод `Enqueue` сделать атомарной точкой принятия задачи с возвратом статуса `Accepted` / `Closed`;
      - Опереться на существующий неблокирующий примитив ядра `CountdownTrigger` (`src/core/templates/CountdownTrigger.h`) для реактивного отслеживания групп асинхронных операций (`SceneManager`), а кадровые задачи `ViewManager` синхронизировать через `TaskDispatcher::Join(eTaskPriority::Critical)`.
- **Зависимости:**
  - `src/core/templates/ThreadPool.h` (базовый примитив пула воркеров);
  - `src/core/templates/CountdownTrigger.h` (реактивная синхронизация групп задач);
  - `src/core/hardware/HardwareState.h`, `src/core/hardware/CpuInfo.h` и `src/engine/platforms/hardware/HardwareManager.h`.
- **Отношение к другим этапам:** Этапы 35 (многопоточный culling) и 45 (асинхронная компиляция шейдеров) выполняются раньше этапа 51 на базе локальных временных воркеров/пулов, а в рамках Этапа 51 централизованно **мигрируют** на использование `TaskDispatcher`.

---

## 2. Архитектура: Индексация по Enum и атомарная маршрутизация

### 2.1. Уровни задач, результат отправки и подсистемы

Приоритеты задач типизируются перечислением `eTaskPriority`:
- **Critical (0):** Сверхсрочные кадровые подзадачи текущего кадра (`ViewManager`: параллельный `Update`, `PreRender`, `PrepareFrame`, `RenderFrame`). Приоритет ОС: Highest.
- **High (1):** Срочные вычислительные задачи кадра (будущий culling, подготовка дроуколлов). На 3-кластерных CPU направляются в PrimePool. Приоритет ОС: AboveNormal.
- **Normal (2):** Стандартные CPU-задачи (асинхронная сборка слоёв `SceneManager`). Направляются в PerfPool / CommonPool. Приоритет ОС: Normal.
- **Background (3):** Фоновые задачи (декомпрессия буферов, парсинг ассетов). Направляются в EffPool / CommonPool. Приоритет ОС: Lowest.

Результат отправки задачи типизируется перечислением `eSubmitResult`:
- `Accepted`: задача атомарно помещена в очередь физического пула;
- `Stopped`: диспетчер или целевой пул остановлен, либо передана пустая задача (`!task`), вызов отклонен;
- `InvalidPriority`: передан приоритет вне диапазона перечисления.

Архитектурные инварианты подсистем:
1. **Главный кадровый цикл и системный резерв:** Отдельного персистентного потока рендера нет — фаза `RenderFrame()` выполняется параллельно для окон через `TaskDispatcher` на уровне `Critical`. Резерв в 1–2 потока закладывается под поток главного цикла (`Update`) и системный запас ядра ОС и видеодрайвера GPU.
2. **ViewManager (миграция локального пула):** Локальный `m_ThreadsUpdate` удаляется. Все параллельные кадровые фазы окон отправляются в `TaskDispatcher` на уровень `Critical` и в конце кадра синхронизируются через `m_TaskDispatcher.Join(eTaskPriority::Critical)`.
3. **SceneManager:** Использует уровень `Normal` для асинхронного наполнения слоёв сцены с реактивной синхронизацией через существующий шаблон ядра `CountdownTrigger`.
4. **ResourceManager:** Существующий выделенный I/O-поток `ResourceManager` остаётся без изменений для файловых операций. В `TaskDispatcher` отправляются только CPU-тяжелые задачи (декомпрессия, парсинг).
5. **Logger:** Автономный поток логгера инициализируется до ядра и не зависит от `TaskDispatcher`. Сетевой поток бродкастера настраивается на пониженный приоритет (`THREAD_PRIORITY_BELOW_NORMAL` / `QOS_CLASS_UTILITY`).

### 2.2. Атомарное принятие задач и изоляция исключений

Во избежание гонок TOCTOU метод `ThreadPool::Enqueue` выступает атомарной точкой принятия: захватывает внутренний мьютекс пула, проверяет флаг остановки (`done`), добавляет функтор в очередь и будит воркер через условную переменную, возвращая статус `Accepted` или `Closed`.

Метод `TaskDispatcher::Submit`:
1. Проверяет валидность функтора: если передан пустой `task`, немедленно возвращает `eSubmitResult::Stopped` (защита Defense-in-Depth от `std::bad_function_call`).
2. Валидирует диапазон перечисления `eTaskPriority`.
3. Оборачивает задачу в защитную функцию `safeTask`, помеченную `noexcept`. Внутри блока `try/catch(...)` вызывается тело задачи. При возникновении исключения извлекается `std::current_exception()`, пишется ошибка в лог и вызывается опциональный колбэк `onError(exPtr)`.
4. Находит физический пул в массиве маршрутизации за $O(1)$ и вызывает `Enqueue`. Если пул закрыт, возвращается `eSubmitResult::Stopped`.

Контракт `onError` и паттерн Panic Shutdown:
- Колбэк `onError` исполняется на контексте воркер-потока и обязан быть `noexcept`.
- Для критических кадровых задач (`ViewManager`, декомпрессия обязательных ассетов) колбэк перекладывает `exception_ptr` в потокобезопасную очередь главного потока `m_MainThreadQueue` (`CallbackQueue`).
- Главный поток в начале кадра (`Engine::OnUpdateSystem()`) исполняет очередь: при наличии сохранённого исключения извлекается его описание и вызывается макрос `THROW_RUNTIME`, который логирует локацию через `std::source_location` и выбрасывает `std::runtime_error`. Управление перехватывается верхнеуровневым блоком `catch` в `Engine::Run()`, инициируя штатный контролируемый `Shutdown()` движка с ожиданием GPU через `WaitForGpu`.

### 2.3. Синхронизация задач: `TaskDispatcher::Join(priority)` и `CountdownTrigger`

В соответствии с Правилом 4 (YAGNI) и Правилом 5 (ревизия шаблонов ядра `src/core/templates/`), движок не создаёт искусственных надстроек (`TaskGroup`) с локальными мьютексами и счётчиками, а опирается на прямые архитектурные примитивы:

1. **Кадровая синхронизация окон (`ViewManager`):** `ViewManager::Update` отправляет задачи окон с приоритетом `Critical` и в конце кадра вызывает `m_TaskDispatcher.Join(eTaskPriority::Critical)`. Метод за $O(1)$ находит `CriticalPool` и ожидает опустошения очереди кадра. Воркеры не уничтожаются и готовы к следующему кадру.
2. **Реактивное отслеживание асинхронных операций (`SceneManager`):** Используется существующий неблокирующий шаблон ядра `CountdownTrigger`. Создаётся триггер на число слоёв сцены с финальным колбэком. Каждая параллельная подзадача наполнения слоя после завершения вызывает `trigger->CountDown()`. Поток, выполнивший последний декремент до нуля, реактивно исполняет завершающий колбэк на главном потоке без блокировки воркеров.

### 2.4. Принцип распределения пулов и маршрутизация (Simple & Deterministic)

Диспетчер задач оперирует предельно простой и прозрачной схемой без динамического балансирования:

1. **Выделенный кадровый пул (`CriticalPool`):**
   - Выделяются 1–2 обязательных потока с наивысшим приоритетом (`Highest`) строго под кадровые задачи окна и рендера.
   - На малоядерных системах ($TotalWorkersBudget \le 1$) отдельный кадровый пул не создаётся, чтобы не плодить лишние потоки.

2. **Рабочие пулы под остаток ядер:**
   - Каждому создаваемому физическому пулу сразу задаётся его фиксированный размер (`threadCount`).
   - **Однородный CPU (десктоп AMD/Intel):** создаётся **всего один** общий рабочий пул `m_CommonPool` на весь остаток ядер с приоритетом `Normal`.
   - **Гетерогенный CPU (P+E кластеры):** создаются `m_PerfPool` (под P-ядра, приоритет `Normal`) и `m_EffPool` (под E-ядра, приоритет `Lowest`). Если есть супер-ядра (Prime) — создаётся `m_PrimePool`.

3. **Массив маршрутизации `m_PoolRouting`:**
   - 4 элемента массива `m_PoolRouting` — это просто **указатели на готовые пулы**.
   - Несколько логических уровней приоритета могут указывать на один и тот же физический пул:
     - Однородный ПК: `m_PoolRouting = { CriticalPool, CommonPool, CommonPool, CommonPool }`;
     - Трёхкластерный CPU (Prime + Perf + Eff): `m_PoolRouting = { CriticalPool, PrimePool, PerfPool, EffPool }` (`Critical` → `CriticalPool`, `High` → `PrimePool`, `Normal` → `PerfPool`, `Background` → `EffPool`);
     - Двухкластерный CPU (Perf + Eff): `m_PoolRouting = { CriticalPool, PerfPool, PerfPool, EffPool }`;
     - Малоядерный ПК: `m_PoolRouting = { CommonPool, CommonPool, CommonPool, CommonPool }`.
   - Суммируются размеры **только уникальных физических пулов** — их сумма строго равна общему бюджету $TotalWorkersBudget$.

---

## 3. Распределение ядер, приоритеты ОС и потоковый бюджет (Zero Affinity Overhead)

Расчёт выполняется однократно при старте движка:
- Логические ядра запрашиваются через `std::thread::hardware_concurrency()` с защитным `std::max(1u, reportedCores)`.
- Выделяется системный резерв: 2 потока при числе ядер $\ge 6$, иначе 1 поток.
- Базовый бюджет воркеров равен разности логических ядер и резерва (минимум 1).
- Аргумент командной строки `--threads=N` ($N > 0$) задаёт верхний предел (cap) бюджета.
- На мобильных платформах применяется безусловный жесткий лимит (Mobile Hard Cap $\le 4$ воркеров) для предотвращения перегрева и троттлинга.

**Распределение бюджета:**
1. **Кадровый пул (`CriticalPool`):**
   - Если $TotalWorkersBudget \ge 4$: 2 потока;
   - Если $TotalWorkersBudget \in [2, 3]$: 1 поток;
   - Если $TotalWorkersBudget \le 1$: 0 потоков (выделенный кадровый пул не создаётся, все уровни обслуживает единый `CommonPool`).
2. **Рабочий остаток:**
   $$RemainingWorkers = TotalWorkersBudget - CriticalWorkers$$
   Все оставшиеся потоки целиком передаются рабочему пулу (`CommonPool` на однородном SMP, либо распределяются между `PerfPool` и `EffPool` на гетерогенных CPU). Сумма потоков уникальных пулов всегда строго равна $TotalWorkersBudget$.

### 3.2. Приоритеты ОС: Best-Effort Hints (Мягкие подсказки с безопасным фоллбэком)

Выставление приоритетов потоков ОС и QoS является **рекомендацией планировщику (Best-effort hint)**, а не жестким контрактом:
- **Apple (iOS / macOS):** Устанавливается `pthread_set_qos_class_self_np`. Планировщик Darwin сам решает распределение по P/E-кластерам.
- **Linux / Android:** Вызов `setpriority` с отрицательным nice требует привилегии `CAP_SYS_NICE`. При получении `EPERM` ошибка логгируется на уровне `Debug`, и поток безопасно остаётся с дефолтным приоритетом (`nice = 0`).
- **Windows:** Вызов `SetThreadPriority`.

| Физический пул | Windows (`SetThreadPriority`) | Apple (QoS) | Linux / Android (`nice` hint) | Назначение |
| :--- | :--- | :--- | :--- | :--- |
| **`CriticalPool`** (1–2 воркера) | `THREAD_PRIORITY_HIGHEST` (+2) *(запрет TIME_CRITICAL)* | `QOS_CLASS_USER_INTERACTIVE` | `nice = -10` (best-effort) | Срочные параллельные подзадачи кадра (`Update`, `RenderPrep`) |
| **`PrimePool`** (гетерогенный) | `THREAD_PRIORITY_ABOVE_NORMAL` (+1) | `QOS_CLASS_USER_INITIATED` | `nice = -5` (best-effort) | Срочные вычислительные задачи |
| **`PerfPool`** (гетерогенный) | `THREAD_PRIORITY_NORMAL` (0) | `QOS_CLASS_DEFAULT` | `nice = 0` | `High`, `Normal` |
| **`EffPool`** (гетерогенный) | `THREAD_PRIORITY_LOWEST` (-2) | `QOS_CLASS_BACKGROUND` | `nice = 10` | `Background` (фоновая компиляция/стриминг) |
| **`CommonPool`** (однородный SMP) | `THREAD_PRIORITY_NORMAL` (0) | `QOS_CLASS_DEFAULT` | `nice = 0` | Все уровни `High`, `Normal`, `Background` |
| **`Logger` (автономный I/O)** | `THREAD_PRIORITY_BELOW_NORMAL` (-1) | `QOS_CLASS_UTILITY` | `nice = 5` | Независимый I/O поток сетевого вещания |

> **Архитектурный инвариант распределения приоритетов:**
> - На **гетерогенных CPU (Случаи А и Б)** создаётся выделенный `EffPool` с приоритетом `Lowest`/`Background`, куда изолированно отправляются фоновые задачи (`Background`).
> - На **однородных системах (Случай В)** и малоядерных процессорах (Случай Г) для исключения лишних пулов в памяти создаётся ровно один `CommonPool` с приоритетом `Normal`, и уровни `High`, `Normal`, `Background` делят его воркеры. Это осознанный компромисс: аппаратное отсутствие E-ядер компенсируется тем, что `CriticalPool` по-прежнему имеет высший приоритет ОС (`Highest`) и даёт планировщику операционной системы явное предпочтение при распределении квантов времени CPU перед `CommonPool`.

### 3.3. Сбор топологии через `CpuInfoCollector`
- **Windows (`CpuInfoCollectorMSWin`):** `GetLogicalProcessorInformationEx(RelationProcessorCore, ...)` с анализом `EfficiencyClass`.
- **Android (`CpuInfoCollectorAndroid`):** парсинг `/sys/devices/system/cpu/cpu*/cpu_capacity`.
- **Apple (`CpuInfoCollectorApple`):** опрос количества уровней производительности через `sysctl hw.nperflevels` и затем динамический опрос `hw.perflevel{i}.logicalcpu` (без жёсткого допущения о ровно 2 уровнях).
- **Linux (`CpuInfoCollectorLinux`):** чтение `/sys/devices/system/cpu/cpu*/topology/`; при отсутствии в ядре данных о `cpu_capacity` или классе производительности — безопасный фоллбэк в однородную SMP-модель (`isHeterogeneous = false`).
- **Editor (`CpuInfoCollectorEditor`):** Заменяется с заглушки `return { 1 CPU }` на опрос реального аппаратного количества ядер хост-машины (`std::thread::hardware_concurrency()`), чтобы редактор не работал в режиме малоядерного фоллбэка.

---

## 4. Контракт API: `TaskDispatcher` (`src/engine/tasks/TaskDispatcher.h`)

В соответствии с Правилом 16 (YAGNI) из контракта исключены неиспользуемые методы (`GetActiveWorkers`, `GetPendingTasks`).

**Типизация физических пулов (`ePoolType`):**
- `Critical` (выделенный кадровый пул);
- `Prime` (супер-ядра Cortex-X при наличии);
- `Perf` (производительные ядра в гетерогенном режиме, либо ВСЕ рабочие потоки в однородном SMP-режиме);
- `Eff` (энергоэффективные E-ядра при наличии);
- `Count = 4`.

**Интерфейс и поля `TaskDispatcher` (`src/engine/tasks/TaskDispatcher.h`):**
- Запрет копирования и перемещения (`Z_NO_COPY_MOVE`);
- Конструктор от `const CpuTopology& topology` и деструктор `~TaskDispatcher()`;
- Метод `Submit(priority, task, onError)`: приём задачи с приоритетом, целевой функцией и опциональным колбэком ошибки;
- Метод `Join(priority)`: прямое ожидание завершения пула, назначенного на данный приоритет ($O(1)$ маршрутизация);
- Метод `JoinAll()`: ожидание всех фактически созданных физических пулов циклом по массиву `m_Pools`;
- Внутренние поля:
  - Массив указателей на пулы `m_PoolRouting` размера `eTaskPriority::Count` (`std::array<ThreadPool*, 4>`);
  - Фиксированный массив владения уникальными физическими пулами `m_Pools` (`std::array<std::unique_ptr<ThreadPool>, 4>`).

### 4.1. Расширение `ThreadPool` (`src/core/templates/ThreadPool.h`)
В `ThreadPool` вносятся минимальные необходимые расширения:
1. **Настройка воркера при старте (`onWorkerStart`):**
   - В конструктор `ThreadPool` добавляется опциональный колбэк `onWorkerStart(workerIndex)`.
   - Внутри `WorkerThread()` перед входом в цикл обработки задач вызывается `onWorkerStart(id)` строго под защитой блока перехвата `try/catch (...)` с выводом ошибки в лог (`DOutError`).
   - В этом колбэке диспетчер задаёт потоку имя (`SetThreadName`) и подсказку приоритета ОС (`SetThreadPriorityHint`), а защита исключает падение системного потока до входа в рабочий цикл.
2. **Атомарный метод `Enqueue` (Fire-and-Forget):**
   - Возвращает статус принятия задачи (`Accepted` либо `Closed`, если пул уже останавливается).
3. **Штатный жизненный цикл (Zero Modification в части исключений):**
   - В существующем `ThreadPool` уже реализован метод `Join()` (ожидание опустошения очереди и завершения активных задач) и безопасный деструктор `~ThreadPool()`.
   - Никаких методов отмены задач (`Cancel`) или принудительной очистки очереди не добавляется (YAGNI).
   - **Исключения не обрабатываются дважды:** `ThreadPool::WorkerThread` не модифицируется под `onError`. Задача оборачивается в `safeTask` строго на уровне `TaskDispatcher::Submit`, где перехватываются сбои и вызывается `onError`. Воркер исполняет уже защищённую функцию, и сигнатура `Enqueue` остаётся минималистичной (`std::function<void()>`).

---

## 5. Конвейер конфигурации и платформенная политика (PlatformTaskPolicy)

Порядок разрешения настроек строго структурирован (без модификации `user.dat` — чистый YAGNI):
1. **Аргументы командной строки (`--threads=N`)**: верхний предел пользователя (CLI Cap);
2. **`PlatformTaskPolicy`**: аппаратный расчёт базового бюджета по `CpuTopology`;
3. **`Mobile Hard Cap`**: абсолютный защитный потолок ОС ($\le 4$ воркеров на Android / iOS).

**Архитектурные правила применения настроек:**
1. **Безусловный Mobile Hard Cap:**
   - На мобильных платформах (Android / iOS) аппаратный лимит перегрева и термодроттлинга ($\le 4$ активных воркеров) является безусловным: финальный бюджет воркеров урезается до 4.

**Цепочка формирования конфигурации:**
[Железо / ОС] $\to$ [HardwareManager::CpuInfoCollector] (формирование `CpuTopology`) $\to$ [PlatformTaskPolicy::Resolve] (Desktop vs Mobile) $\to$ [TaskDispatcherConfig] (финальные скорректированные бюджеты) $\to$ [TaskDispatcher(config)] (создание пулов и заполнение `m_PoolRouting`).

### 5.1. Лимиты `PlatformTaskPolicy`:
1. **Desktop (`PlatformTaskPolicyDesktop` — Win/Linux/macOS):**
   - Бюджет воркеров:
     - При $LogicalCores \ge 6$: $TotalWorkersBudget = LogicalCores - 2$;
     - При $LogicalCores < 6$: $TotalWorkersBudget = \max(1, LogicalCores - 1)$.
2. **Mobile (`PlatformTaskPolicyMobile` — Android/iOS):**
   - Суммарный бюджет воркеров `TaskDispatcher`: **не более 3–4 воркеров на весь процесс** (Mobile Hard Cap):
     $$TotalWorkersBudget = \min(TotalWorkersBudget, 4)$$
   - `Critical`: 1 воркер на Prime/P-ядрах;
   - `Normal`: 1–2 воркера на P-ядрах;
   - `Background`: 1 воркер на E-ядрах.

При инициализации подсистемы в канонический лог (`::zzz::core::Hardware`) выводятся два структурированных блока фактов:
1. **Блок `[CpuTopology]`**: имя процессора, архитектура (x64/ARM64), общее число логических и физических ядер, признак гетерогенности (`isHeterogeneous`), а также ёмкости логических ядер по кластерам (`primeLogicalCapacity`, `performanceLogicalCapacity`, `efficiencyLogicalCapacity`), если система гетерогенная.
2. **Блок `[TaskDispatcherConfig]`**: применённая платформенная политика (`Desktop` / `Mobile`), перечень созданных физических пулов с их фактическим числом потоков и приоритетом ОС (`PriorityHint`), а также итоговая матрица маршрутизации `m_PoolRouting` по приоритетам (`Critical`, `High`, `Normal`, `Background`).

---

## 6. Безопасность и порядок завершения (Clean Shutdown)

Безопасное завершение гарантирует отсутствие висячих задач и крашей при уничтожении сервисов:

1. **Фаза 1 (Ожидание воркеров перед разрушением менеджеров):**
   - В начале `Engine::Shutdown()` (вызываемом из деструктора `Engine::~Engine()`) перед обнулением менеджеров вызывается единый метод ожидания всех пулов:
     - `m_TaskDispatcher->JoinAll()`
   - Метод `JoinAll()` последовательно и гарантированно ожидает завершения всех фактически созданных физических пулов (`m_CriticalPool`, `m_PrimePool`, `m_PerfPool`, `m_EffPool`, `m_CommonPool`). Это полностью изолирует `Engine::Shutdown()` от деталей топологии процессора и исключает необходимость перебирать приоритеты вручную.
   - Это гарантирует, что все активные задачи сервисов (асинхронная загрузка сцен `SceneManager`, наполнение слоёв, culling геометрии) полностью завершают выполнение. Ни один воркер больше не обратится к полям менеджеров по захваченному указателю, что на 100% исключает Use-After-Free.
2. **Фаза 2 (Освобождение сервисов движка):**
   - После полного затихания воркеров менеджеры безопасно обнуляются в строгом порядке: `SceneManager`, `ViewManager`, `GAPI`, `Time`.
3. **Фаза 3 (Освобождение диспетчера задач):**
   - Обнуляется `m_TaskDispatcher`: деструктор `~TaskDispatcher()` уничтожает уникальные физические пулы, а деструкторы `~ThreadPool()` транзитивно и безопасно останавливают системные потоки.
4. **Фаза 4 (Финальный сброс логов):**
   - `Logger` закрывается последним (автономный I/O поток).

### 6.1. Аварийный выход при критическом исключении воркера (Panic Shutdown)
При возникновении сбоя в фоновом потоке или воркере кадра:
1. Поток не прерывается аварийным `std::terminate()`, а передаёт `std::exception_ptr` через `onError` в `m_MainThreadQueue` (`CallbackQueue`);
2. Главный поток в фазе `Engine::OnUpdateSystem()` вызывает `m_MainThreadQueue.ExecuteAll()`;
3. Колбэк извлекает описание ошибки и вызывает макрос `THROW_RUNTIME`, который автоматически форматирует локацию (`std::source_location`), выводит `DOutException` в лог и бросает типизированный `std::runtime_error`;
4. Кадровый цикл `MainLoop::Run()` прерывается, и `Engine::Run()` выполняет штатный контролируемый `Shutdown()` (Фазы 1–4) с очисткой ресурсов GAPI (`WaitForGpu`), предотвращая зависание драйвера и порчу данных.

Именование воркеров через `SetThreadName`: `CriticalWorker_0`, `CriticalWorker_1`, `NormalWorker_N`, `BackgroundWorker_N`.

---

## 7. Чек-лист и порядок реализации

1. **Расширение ядра (`src/core/templates/`):**
   - В `ThreadPool.h`: добавить `onWorkerStart` в конструктор и атомарный метод `Enqueue`.
2. **Модель топологии CPU и аппаратное состояние:**
   - Создать `src/core/hardware/CpuTopology.h` с полями logical capacity для каждого класса ядер;
   - Встроить `CpuTopology` в `src/core/hardware/HardwareState.h`;
   - Расширить `CpuInfoCollector` под Windows, Linux (с SMP-фоллбэком), Apple (динамический `hw.nperflevels`), Android;
   - Обновить `CpuInfoCollectorEditor.h` для опроса реального числа ядер хоста (`std::thread::hardware_concurrency()`).
3. **Класс `TaskDispatcher`:**
   - Создать `src/engine/tasks/TaskDispatcher.h` и `src/engine/tasks/TaskDispatcher.cpp`;
   - Конструктор от `CpuTopology`: расчёт бюджетов потоков с учётом лимита мобилок (`Mobile Hard Cap <= 4`);
   - Физические пулы в фиксированном массиве `m_Pools` по `ePoolType` (Critical, Prime, Perf, Eff);
   - Реализовать заполнение `m_PoolRouting` с безопасным фоллбэком под 1–2 ядра / $TotalWorkersBudget \le 1$;
   - Реализовать `Submit` с проверкой `!task`, валидацией границ enum, атомарным вызовом `Enqueue` и безопасной изоляцией исключений через `std::exception_ptr`;
   - Реализовать `Join(eTaskPriority priority)` за $O(1)$ через `m_PoolRouting[priority]->Join()`;
   - Реализовать `JoinAll()` простым проходом по `m_Pools`.
4. **Интеграция в `Engine` и клиенты:**
   - Создать `m_TaskDispatcher` в `Engine.h/.cpp`;
   - Пробросить `TaskDispatcher&` в `SceneManager` и `ViewManager`;
   - В `SceneManager` и `Scene` (`src/engine/scene/Scene.h/.cpp`) перевести загрузку слоёв на `TaskDispatcher&` (`eTaskPriority::Normal`) с существующим `CountdownTrigger`;
   - В `ViewManager` (`src/engine/view/ViewManager.h/.cpp`) удалить локальный `m_ThreadsUpdate`, перевести кадровые задачи окон на `TaskDispatcher&` (`eTaskPriority::Critical`) и использовать `m_TaskDispatcher.Join(eTaskPriority::Critical)` для ожидания кадра;
   - Удалить локальный `m_LoadingThreadPool` из `SceneManager`.
6. **Безопасное завершение:**
   - В `Engine::Shutdown()` перед обнулением менеджеров вызвать `m_TaskDispatcher->JoinAll()`;
   - После завершения задач воркеров безопасно обнулить `SceneManager`, `ViewManager`, а затем `TaskDispatcher`.
7. **Логирование:**
   - Добавить вывод канонического блока `CpuTopology` и `TaskDispatcherConfig` при старте.

---

## 8. Что НЕ входит в этап (YAGNI)

- **Модификация user.dat / UserSettingsManager:** файл конфигурации пользователя не затрагивается; бюджет задаётся аппаратно через `PlatformTaskPolicy` либо через аргументы CLI.
- **Отмена задач из очереди (`Cancel` / `Drain`):** готовый `ThreadPool` полностью покрывает задачи движка через `Join()` и деструктор.
- **Work-stealing между очередями:** пулы изолированы; P-ядра не воруют задачи E-ядер.
- **Динамический граф задач (Task Graph с гранями зависимостей):** задачи независимы; синхронизация только через `CountdownTrigger`, `TaskDispatcher::Join` и `CallbackQueue`.
- **Акцессоры очередей (`GetActiveWorkers`, `GetPendingTasks`):** не реализуются до появления внутридвижкового UI-профайлера.

---

## 9. Затрагиваемые и новые файлы

- **Новые файлы:**
  - `src/core/hardware/CpuTopology.h`
  - `src/engine/tasks/TaskPriority.h`
  - `src/engine/tasks/TaskConstants.h`
  - `src/engine/tasks/TaskDispatcher.h`
  - `src/engine/tasks/TaskDispatcher.cpp`
- **Модифицируемые файлы:**
  - `src/core/CMakeLists.txt`
  - `src/core/templates/ThreadPool.h`
  - `src/core/hardware/HardwareState.h`
  - `src/engine/platforms/hardware/platforms/cpu/CpuInfoCollectorEditor.h`
  - `src/engine/platforms/hardware/platforms/cpu/CpuInfoCollectorMSWin.cpp` (и Linux/Apple/Android)
  - `src/engine/platforms/hardware/HardwareManager.h/.cpp`
  - `src/engine/engine.h` и `src/engine/engine.cpp`
  - `src/engine/scene/SceneManager.h` и `src/engine/scene/SceneManager.cpp`
  - `src/engine/scene/Scene.h` и `src/engine/scene/Scene.cpp`
  - `src/engine/view/ViewManager.h` и `src/engine/view/ViewManager.cpp`
  - `src/engine/CMakeLists.txt`
  - `src/projects/game_win/main.cpp` (и платформенные entry points Linux/Android/iOS/macOS)

---

## 10. Критерии приёмки (Definition of Done — DoD)

1. Код чисто компилируется без ошибок и предупреждений на текущем целевом роллаут-таргете (`run_build.bat` под Windows MSVC), а платформозависимый код под Linux/macOS/Android/iOS изолирован в специализированных файлах согласно Правилам 9–11.
2. В консоль и лог-файл при старте выводится канонический блок `[CpuTopology]` и `[TaskDispatcherConfig]`.
3. `SceneManager` и `ViewManager` не содержат локальных `ThreadPool`, а работают через переданный `TaskDispatcher&` на соответствующих приоритетах (`Normal` с `CountdownTrigger` и `Critical` с `TaskDispatcher::Join`).
4. На системах с $TotalWorkersBudget \le 1$ движок корректно стартует и не падает (фоллбэк на единый `m_CommonPool` с 1 воркером).
5. Завершение приложения происходит чисто без deadlock'ов, висячих задач и Use-After-Free в потоках (ожидание `Join` в `Engine::Shutdown()` перед обнулением менеджеров $\to$ деструктор `TaskDispatcher` $\to$ деструкторы `ThreadPool`).
6. Никаких тестов не пишется и не запускается без явной команды пользователя (Правило 5 из `RULES.md`, Раздел 2 `GEMINI.md`).
7. Этап принимается пользователем явно словом «Шаг принят».
