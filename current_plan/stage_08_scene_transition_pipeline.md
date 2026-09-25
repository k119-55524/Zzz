# Этап 08: Конвейер смены сцен, двухфазная асинхронная загрузка и каркас переходов (Scene Transition Pipeline & Two-Phase Async Loading)

> **Статус:** Выполнен каркас по генплану. Уточнение 2026-09-10: асинхронный путь уже общий для Primary/Child/Independent View, повторно создавать его в 40 не нужно. Сейчас публикация/InvokeStart следует за CPU-инициализацией. **Целевое принятое поведение:** OnStart только после полной загрузки обязательных ресурсов, включая GPU-ready; ошибка обязательного ресурса пока THROW_RUNTIME. Общий контракт готовности вводится в [12](stage_12_texture2d_and_gpu_upload.md), подключение полного графа зависимостей завершается в 13. Описанные ниже вызовы InvokeStart фиксируют каркас, а не уже реализованную GPU-гарантию. Визуальные эффекты переходов отложены в 49.

---

## 🎯 Цель этапа

1. Исключить жесткую блокирующую синхронную загрузку сцены из конструктора [`View`](file:///c:/Workspaces/ZzzTest/src/engine/view/View.cpp) (соблюдение Single Responsibility Principle).
2. Спроектировать и внедрить **каркас** структуры параметров перехода сцен [`SceneTransitionParams`](file:///c:/Workspaces/ZzzTest/src/engine/scene/transition/SceneTransitionParams.h) с поддержкой флагов (`blockUserInput`, длительность `durationSeconds`, цвет затухания `fadeColor`, тип перехода `eTransitionType`).  
   *(На данном этапе создаётся каркас структур и состояний переходов; графические эффекты переходов реализуются в этапе 61 поверх рабочего рендера.)*
3. Обеспечить трехуровневую иерархию правил перехода:
   - Глобальная настройка в [`SceneManager`](file:///c:/Workspaces/ZzzTest/src/engine/scene/SceneManager.h);
   - Локальная настройка в [`SceneData`](file:///c:/Workspaces/ZzzTest/src/core/io/package/SceneData.h) с флагом `eTransitionSource` (`UseGlobal` / `Custom`);
   - Явное переопределение при динамической смене сцены в рантайме через [`View::SetScene`](file:///c:/Workspaces/ZzzTest/src/engine/view/View.h).
4. Учесть структуру `SceneTransitionParams` при сериализации и формировании бинарных данных / JSON ресурсов (для студии и пакетов).
5. **Публикация на логическом потоке (генплан, правило 14) через CallbackQueue:**
   - Чтение пакета, десериализация `SceneData`, аллокация памяти и инстанцирование `Scene` со слоями и скриптами выполняются полностью асинхронно в фоновом воркере `ThreadPool` без блокировки кадровой частоты (FPS).
   - В начале тика кадра `SceneManager::Update` через потокобезопасную очередь `m_MainThreadQueue` (шаблон `CallbackQueue`) на главном потоке сцена регистрируется в `m_Scenes`, запускаются скрипты (`scene->InvokeStart()`) и вызывается `onComplete(scene)`.
   - Проверка наличия задач на холостых кадрах выполняется за ~0.5 нс без единого захвата мьютекса (Zero-Overhead Idle).

---

## 📐 Архитектурные решения и спецификация

### 1. Чистая коллбэк-модель асинхронной загрузки сцены (Callback Architecture)

Изначальная концепция использования C++20 корутин (`AsyncTask<T>`, `FireAndForget`, `co_await`) для загрузки сцен (операции, выполняющейся раз в несколько минут) создавала чрезмерное усложнение (overengineering), аллокации фреймов корутин и предупреждения компилятора. Архитектура была переведена на чистую, прозрачную и эффективную модель коллбэков на базе пула потоков и очереди логического потока.

#### Чёткое разделение потоков выполнения:
```
void SceneManager::LoadSceneAsync(Guid sceneGuid, SceneLoadCallback onComplete)
   │
   ├──► 1. Если сцена уже в памяти — onComplete(scene) через m_MainThreadQueue и return;
   │
   ├──► 2. m_LoadingThreadPool->Submit([...]() { ... })
   │       ▼ [Фоновый поток пула (I/O, десериализация, создание Scene)]
   │       1. m_PackageManager->GetEntryByGuid(ePackage::Scene, sceneGuid);
   │       2. m_PackageManager->LoadPackageDataByGuid<SceneData>(ePackage::Scene, sceneGuid);
   │       3. safe_make_shared<Scene>(guid, name, scripts, ...);
   │
   └──► 3. m_MainThreadQueue.Push([...]() { ... })
           ▼ [Главный логический поток кадра (SceneManager::Update)]
           4. Добавление scene в m_Scenes;
           5. scene->InvokeStart();
           6. onComplete(scene); // Гарантированный вызов коллбэка строго в главном потоке!
```

### 2. Пул задач и диспетчеризация на главный поток (`m_MainThreadQueue`)

- Все I/O, десериализация данных (`SceneData`) и создание объекта `Scene` со слоями и скриптами выполняются в фоновом воркере `m_LoadingThreadPool`.
- Регистрация в `m_Scenes`, запуск скриптов (`InvokeStart()`) и передача результата в `onComplete` происходят строго на главном потоке кадра через `m_MainThreadQueue.Push(...)` (шаблон `CallbackQueue`).
- В `SceneManager::Update(time)` очередь задач `m_MainThreadQueue.ExecuteAll()` забирается локально под мьютексом (`std::move`), после чего задачи выполняются вне блокировки, исключая дедлоки и рекурсивные захваты.
- Валидация входных параметров: `ensure(!sceneGuid.IsEmpty())` и `ensure(onComplete != nullptr)`.

---

### 3. Структуры и перечисления переходов (`src/engine/scene/transition/`)

#### 3.1. `eTransitionType` и `eTransitionSource`
```cpp
namespace zzz::engine
{
    enum class eTransitionType : zU8
    {
        Instant = 0,    // Мгновенная смена сцены без эффектов
        FadeColor = 1,  // Затемнение/высветление через цвет (Fade Out -> Fade In)
        CrossFade = 2   // Плавное растворение старой сцены в новую
    };

    enum class eTransitionSource : zU8
    {
        UseGlobal = 0,  // Использовать глобальные настройки переходов из SceneManager
        Custom = 1      // Использовать параметры перехода, заданные для этой сцены
    };
}
```

#### 3.2. `SceneTransitionParams`
```cpp
namespace zzz::engine
{
    struct SceneTransitionParams
    {
        eTransitionType type{ eTransitionType::Instant };
        zF32 durationSeconds{ 0.3f };
        zzz::math::Color4<zF32> fadeColor{ 0.0f, 0.0f, 0.0f, 1.0f }; // Черный по умолчанию
        
        // Флаги управления поведением перехода (каркас)
        bool blockUserInput{ true };        // Блокировать доставку пользовательского ввода в скрипты на время перехода
        bool pauseOldSceneUpdate{ true };   // Приостанавливать Update() уходящей сцены
        bool renderLoadingSpinner{ false }; // Отображать индикатор ожидания при задержке I/O
    };
}
```

#### 3.3. Разрешение параметров перехода при создании сцены
- Разрешение типа и параметров перехода происходит в момент сборки сцены в фоновом воркере `LoadSceneAsync`:
  - Если в `SceneData` указан `transitionSource == eTransitionSource::Custom`, сцена получает свои локальные `transitionParams`.
  - Если `transitionSource == eTransitionSource::UseGlobal`, подставляются глобальные параметры `m_GlobalTransitionParams`, загруженные из `PackageManager->GetProjectManifestData()->GetDefaultTransitionParams()`.
- Сцена `Scene` сохраняет уже точные, готовые параметры `SceneTransitionParams m_TransitionParams` и **не зависит от `SceneManager`** (нет лишней связи и указателей на менеджер).
- Во `View::SetScene(newScene)` окно получает уже готовые параметры через простой геттер `newScene->GetTransitionParams()` и действует соответственно (мгновенное переключение либо запуск перехода с блокировкой ввода).

---

### 4. Расширение `SceneData` (Сериализация метаданных перехода и учет JSON)

В [`src/core/io/package/SceneData.h`](file:///c:/Workspaces/ZzzTest/src/core/io/package/SceneData.h):
1. Добавляются поля:
   - `eTransitionSource transitionSource{ eTransitionSource::UseGlobal };`
   - `SceneTransitionParams transitionParams{};`
2. Обновляются методы `Serialize` и `Deserialize`:
   - Сериализуются `transitionSource`, `type`, `durationSeconds`, `fadeColor`, `blockUserInput`, `pauseOldSceneUpdate`, `renderLoadingSpinner`.
   - **Историческая совместимость этапа 8:** при чтении старого формата, заканчивавшегося после gameObjects, предусматривались UseGlobal и дефолтные параметры. Это не универсальное правило принимать обрезанный пакет: дальнейшие изменения формата подчиняются явной версии/миграции (генплан, правило 17).
3. В схемах студии и JSON-метаданных учитывается блок `"transition"`:
   ```json
   "transition": {
       "source": "UseGlobal", // или "Custom"
       "type": "Instant",
       "duration": 0.3,
       "fadeColor": [0.0, 0.0, 0.0, 1.0],
       "blockUserInput": true,
       "pauseOldSceneUpdate": true,
       "renderLoadingSpinner": false
   }
   ```

---

### 5. Рефакторинг `View` (`src/engine/view/View.h`, `View.cpp`)

#### 5.1. Разгрузка конструктора
- Из `View::View` и `View::Initialize` **полностью удаляется** вызов `sceneManager->LoadScene(...)`.
- `m_ActiveScene` инициализируется в `nullptr`. Окно создается и запускается мгновенно без фриза.

#### 5.2. Каркас смены сцены и блокировки ввода
- Метод смены сцены принимает только новую сцену, параметры перехода разрешаются динамически:
  ```cpp
  void SetScene(std::shared_ptr<Scene> newScene);
  ```
- Метод проверки блокировки пользовательского ввода:
  ```cpp
  [[nodiscard]] bool IsUserInputBlocked() const noexcept;
  ```
- Состояние перехода (`eTransitionState`: `Idle`, `FadingOut`, `Activating`, `FadingIn`).
  - Пока действует переход с флагом `blockUserInput == true`, `IsUserInputBlocked()` возвращает `true`.

#### 5.3. Архитектурная модель двухсценного перехода (Задел на будущее)
- **Концепция**: На время выполнения анимации/эффекта перехода во `View` одновременно присутствуют две сцены:
  - `m_CurrentScene` (старая, уходящая сцена);
  - `m_NextScene` (новая, входящая сцена).
- Конкретная логика выбранного типа перехода (`Instant`, `FadeColor`, `CrossFade`) оркестрирует таймлайн:
  - В какой момент начинать рендерить вторую сцену (например, при CrossFade обе сцены рендерятся одновременно в два фреймбуфера и блендятся; при FadeColor сначала рисуется затухание старой сцены до черного, затем активируется новая).
  - В какой момент приостанавливать `Update()` уходящей сцены (согласно флагу `pauseOldSceneUpdate`).
  - В какой момент безопасно уничтожать/отпускать `m_CurrentScene`.
- *Внимание*: На этапе 08 реализуется базовый каркас без отрисовки визуальных оверлеев (Instant switchover), полномасштабная отрисовка двух фреймбуферов и блендинг войдут в этапы графического пайплайна 19–20, 22–23 и этап визуальных переходов 57.

---

### 6. Расширение `SceneManager` (`src/engine/scene/SceneManager.h`, `SceneManager.cpp`)

1. **Глобальные параметры:**
   - Хранение приватного `SceneTransitionParams m_GlobalTransitionParams`.
   - Инициализация из `ProjectManifestData::GetDefaultTransitionParams()`.
2. **Очередь задач логического потока:**
   - Потокобезопасная очередь `CallbackQueue<> m_MainThreadQueue;`.
   - В начале `SceneManager::Update(time)` вызывается `m_MainThreadQueue.ExecuteAll()` для публикации загруженных сцен, запуска скриптов и вызова коллбэков.
3. **Исключительно асинхронная загрузка (Single Path):**
   - Синхронные методы `LoadScene` и `LoadSceneByName` **полностью удалены** из движка во избежание блокировок и нарушения инвариантов потокобезопасности.
   - Загрузка сцен ведется строго асинхронно через:
     - `void LoadSceneAsync(Guid sceneGuid, SceneLoadCallback onComplete);`
     - `void LoadSceneAsync(std::string sceneName, SceneLoadCallback onComplete);`

---

### 7. Оркестрация во `ViewManager` и `Engine`

- В `Engine::OnUpdateSystem()`:
  - Перед `m_ViewManager->Update()` вызывается `m_SceneManager->Update()`, внутри которого первым делом разгружается очередь фоновых задач `m_MainThreadQueue`.
- Во `ViewManager`:
  - Окна создаются мгновенно без блокировок;
  - Запускается асинхронная привязка `SetupSceneAsync(view, sceneGuid)` через `m_SceneManager->LoadSceneAsync(sceneGuid, [viewWeak](auto sceneRes) { ... })`;
  - По готовности сцена монтируется во `View` через `view->SetScene(*sceneRes)`;
  - Безопасная слабая ссылка `std::weak_ptr<View>` предотвращает Use-After-Free, если окно было закрыто в процессе загрузки сцены.

---

## 📋 Чек-лист реализации (Checklist)

- [x] **1. Перечисления и структуры переходов:**
  - [x] Создать `src/core/scene/transition/SceneTransitionParams.h` (`eTransitionType`, `eTransitionSource`, `SceneTransitionParams`).
  - [x] Зарегистрировать в `src/core/CMakeLists.txt`.
- [x] **2. Сериализация в `SceneData` (с поддержкой обратной совместимости):**
  - [x] Добавить поля `transitionSource` и `transitionParams` в `src/core/io/package/SceneData.h`.
  - [x] Реализовать `Serialize` и `Deserialize` с гарантией совместимости со старыми пакетами.
- [x] **3. Разгрузка `View` и каркас переходов:**
  - [x] Удалить синхронный `LoadScene` из `View::Initialize`.
  - [x] Реализовать `View::SetScene(std::shared_ptr<Scene>)` с динамическим разрешением параметров перехода (`Scene::GetTransitionParams`).
  - [x] Реализовать `IsUserInputBlocked()` и каркас `eTransitionState`.
- [x] **4. Асинхронная загрузка через коллбэки в `SceneManager`:**
  - [x] Реализовать потокобезопасную очередь `m_MainThreadQueue` на базе шаблона `CallbackQueue`.
  - [x] Реализовать фоновые I/O, десериализацию и создание `Scene` в `m_LoadingThreadPool->Submit(...)`.
  - [x] Публикация сцены в `m_Scenes`, запуск `InvokeStart()` и вызов `onComplete` строго в главном потоке кадра через `m_MainThreadQueue` в `SceneManager::Update()`.
- [x] **5. Оркестрация во `ViewManager`:**
  - [x] Связать асинхронную установку сцены после создания `View` через коллбэк `SetupSceneAsync`.
- [x] **6. Устранение оверинжиниринга:**
  - [x] Удалены легаси корутины (`src/core/async/AsyncTask.h`).
- [x] **7. Верификация:**
  - [x] Проверить отсутствие циклических зависимостей заголовков.
  - [x] Сборка проекта (Ninja) и запуск юнит-тестов (64 из 64 тестов пройдены успешно).
