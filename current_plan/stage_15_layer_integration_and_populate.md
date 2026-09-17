# Этап 15: Интеграция слоёв, базовый класс `SceneTreeLayerBase`, покадровый конвейер и наполнение (`Populate`)

## 1. Контекст и цели этапа
- **Номер пункта:** **Пункт 15** (Уровень 2: GAPI-ресурсы, содержимое куба и сквозной рендер).
- **Цель:** Связать все компоненты воедино на уровне слоёв сцены, кадрового цикла и загрузки пакетов:
  1. Реализация доменов: `ObjectDomain` (на базе `SlotMap<GameObject*>`) и `EntityDomain` (на базе `EntityWorld`).
  2. Реализация 3-уровневой иерархии слоёв: `ILayer` $\to$ `SceneTreeLayerBase` (BeginFrame, Update, Populate, ApplyHandoverBarrier, владение) $\to$ `Layer3D` / `Layer2D` и автономный `LayerMVVM`.
  3. Двухпроходное наполнение слоя `SceneTreeLayerBase::Populate`: создание объектов, $TRS$, скриптов, мешей, связывание через `go->BindSceneTree(&m_TreeContainer, handle)` и связывание иерархии по `parentGuid`.
  4. Точная интеграция с покадровым конвейером `ViewManager` (вызов `scene->ApplyHandoverBarrier()` после `Join()` рабочих потоков).
  5. Проверка сквозной сборки проекта под MSVC x64 + Ninja и обновление `general_plan.md`.
- **Статус:** Выполнена интеграция по генплану. Populate создаёт узлы/объекты, связывает иерархию, присоединяет скрипты и сохраняет GUID ресурсов; CPU-чтение MeshData ещё не создаёт GPU Mesh. Формат/хранилища сцены уточняются в этапах 17–18, а lifecycle скриптов, готовность ресурсов и Draw завершаются в этапах 19–20, 22–23. Проверки внизу исторические; ревизия 2026-09-10 новых прогонов не включает.
- **Зависимости:** Этапы 12, 13, 14.

### Физическое размещение файлов
```text
src/
└── engine/
    └── scene/
        ├── domain/
        │   ├── ObjectDomain.h / .cpp        <-- Реализация ООП-домена
        │   ├── EntityDomain.h / .cpp        <-- Реализация ECS-домена
        │   └── DefaultDomainFactory.h / .cpp<-- Фабрика доменов
        │
        └── layer/
            ├── SceneTreeLayerBase.h / .cpp  <-- Базовый класс слоёв со сценовым деревом
            ├── Layer3D.h / .cpp             <-- 3D-слой
            ├── Layer2D.h / .cpp             <-- 2D-слой
            └── LayerMVVM.h / .cpp           <-- Очищенный декларативный UI-слой
```

---

## 2. Архитектурные спецификации

### 2.1. Контракт интерфейса `ILayer` и базовый класс `SceneTreeLayerBase`

В `ILayer` добавляются покадровые методы для управления жизненным циклом и барьером сдачи кадра:
```cpp
class ILayer
{
public:
    virtual ~ILayer() = default;

    [[nodiscard]] virtual const std::string& GetName() const noexcept = 0;
    [[nodiscard]] virtual eLayerType GetType() const noexcept = 0;

    [[nodiscard]] virtual bool IsVisible() const noexcept = 0;
    virtual void SetVisible(bool visible) noexcept = 0;

    /// @brief Начало кадра логики: сброс dirtyTracker перед выполнением скриптов.
    virtual void BeginFrame() {}

    /// @brief Кадровое обновление логики слоя.
    virtual void Update(float dt) = 0;

    /// @brief Барьер сдачи кадра: дифференциальная синхронизация Back Buffer -> Front Buffer.
    virtual void ApplyHandoverBarrier() {}

    virtual void Populate(
        const ::zzz::core::LayerData& layerData,
        const ::zzz::core::ScriptFactory& scriptFactory) = 0;
};
```

Базовый класс `SceneTreeLayerBase`:
```cpp
namespace zzz::engine
{
    class SceneTreeLayerBase : public ILayer
    {
    public:
        SceneTreeLayerBase(
            std::string name,
            std::shared_ptr<ResourceManager> resourceManager,
            std::unique_ptr<IDomainFactory> domainFactory,
            std::unique_ptr<ISpatialStorage> spatialStorage);
        ~SceneTreeLayerBase() override = default;

        Z_NO_COPY_MOVE(SceneTreeLayerBase);

        [[nodiscard]] const std::string& GetName() const noexcept override { return m_Name; }
        [[nodiscard]] bool IsVisible() const noexcept override { return m_IsVisible; }
        void SetVisible(bool visible) noexcept override { m_IsVisible = visible; }

        void BeginFrame() override
        {
            if (!m_IsVisible) return;
            m_TreeContainer.BeginFrame();
        }

        void Update(float dt) override
        {
            if (!m_IsVisible) return;
            OnUpdateDomains(dt);
            OnUpdateSpatial();
        }

        void ApplyHandoverBarrier() override
        {
            if (!m_IsVisible) return;
            m_TreeContainer.ApplyHandoverBarrier();
        }

        void Populate(const ::zzz::core::LayerData& layerData, const ::zzz::core::ScriptFactory& scriptFactory) override;

        [[nodiscard]] SceneTreeContainer& GetTreeContainer() noexcept { return m_TreeContainer; }
        [[nodiscard]] const SceneTreeContainer& GetTreeContainer() const noexcept { return m_TreeContainer; }

        [[nodiscard]] ISpatialStorage& GetSpatialStorage() noexcept { return *m_SpatialStorage; }
        [[nodiscard]] IObjectDomain& GetObjectDomain() noexcept { return *m_ObjectDomain; }
        [[nodiscard]] IEntityDomain& GetEntityDomain() noexcept { return *m_EntityDomain; }

    protected:
        virtual void OnUpdateDomains(float dt)
        {
            if (m_ObjectDomain) m_ObjectDomain->Update(dt);
            if (m_EntityDomain) m_EntityDomain->Update(dt);
        }

        virtual void OnUpdateSpatial()
        {
            m_TreeContainer.ResolveTransforms();
        }

        std::string                      m_Name;
        bool                             m_IsVisible{ true };
        std::shared_ptr<ResourceManager> m_ResourceManager;

        std::unique_ptr<IDomainFactory>  m_DomainFactory;
        std::unique_ptr<ISpatialStorage> m_SpatialStorage;
        SceneTreeContainer               m_TreeContainer;
        std::unique_ptr<IObjectDomain>   m_ObjectDomain;
        std::unique_ptr<IEntityDomain>   m_EntityDomain;
    };
}
```

---

### 2.2. Инициализация и двухпроходное наполнение слоя (`SceneTreeLayerBase::Populate`)

`GameObjectData` дополняется полем `Guid m_ParentGuid` (если объект корневой — `Guid::Empty()`).

Алгоритм наполнения:
1. **Проход 1 (Создание объектов и регистрация в структурах слоя):**
   - Создаётся локальная таблица отображения: `std::unordered_map<Guid, NodeHandle> guidToHandle;`
   - Для каждого `objData` из `layerData.GetObjects()`:
     - Если `objData.IsEntity()`:
       - Создаётся сущность в `m_EntityDomain` (на Этапе 11 сущности плоские, в ECS-домене).
     - Если объект — `GameObject` (`!objData.IsEntity()`):
       - Создается объект в объектном домене:
         `GameObject* go = m_ObjectDomain->CreateObject(objData.GetGuid(), objData.GetName());`
       - Создается узел в сценовом дереве:
         `NodeHandle handle = m_TreeContainer.CreateNode(objData.GetName(), go);`
       - Связывается фасад с узлом дерева через **фактический метод `BindSceneTree`**:
         `go->BindSceneTree(&m_TreeContainer, handle);`
       - Устанавливаются свойства:
         `go->SetActive(objData.IsActive());`
         `go->SetMeshGuid(objData.GetMeshGuid());`
         `go->SetMaterialGuid(objData.GetMaterialGuid());`
       - Устанавливается локальный $TRS$ в хранилище через прямой API `GameObject`:
         `go->SetLocalPosition(objData.GetPosition());`
         `go->SetLocalRotation(objData.GetRotation());`
         `go->SetLocalScale(objData.GetScale());`
       - Инжектируются пользовательские скрипты:
         `for (const auto& sGuid : objData.GetScriptGuids()) { auto script = scriptFactory.CreateScript(sGuid, go); if (script) go->AddScript(std::move(script)); }`
       - Загружаются ресурсы меша:
         При наличии `go->GetMeshGuid()` и `m_ResourceManager`:
           `m_ResourceManager->LoadDataAsset<MeshData>(go->GetMeshGuid());`
       - Узел регистрируется в пространственном индексе:
         `SpatialHandle spHandle = m_SpatialStorage ? m_SpatialStorage->Insert(static_cast<uint64_t>(handle.index)) : 0xFFFFFFFF;`
         `m_TreeContainer.SetSpatialHandle(handle, spHandle);`
       - `guidToHandle[objData.GetGuid()] = handle;`

2. **Проход 2 (Связывание иерархии):**
   - Для каждого `objData` из `layerData.GetObjects()`:
     - `parentGuid = objData.GetParentGuid();`
     - Если `parentGuid != Guid::Empty()`:
       - `childHandle = guidToHandle[objData.GetGuid()];`
       - Поиск родителя: `auto it = guidToHandle.find(parentGuid);`
       - Если родитель найден:
         `m_TreeContainer.SetParent(childHandle, it->second, false);`
         (`keepWorldTransform = false`, так как в файле пакета записан локальный $TRS$ относительно родителя).
       - Если родитель не найден в слое:
         `Z_LOG_WARN` и объект остаётся в корне дерева.

---

### 2.3. Конвейер жизненного цикла кадра и точка барьера во `ViewManager`

1. **Фаза начала кадра логики (в потоке логики):**
   - В начале `Scene::Update()` перед скриптами вызывается:
     ```cpp
     for (const auto& layer : m_Layers)
         if (layer) layer->BeginFrame();
     ```
   - `SceneTreeLayerBase::BeginFrame()` вызывает `m_TreeContainer.BeginFrame()` и Prepare трекера. Пустой путь без изменения размеров O(1); очистка непустого трекера/переразмеривание не считаются O(1).
   - Изменения через SetLocalPosition/SetLocalRotation пишутся в m_SecondaryNodes и dirtyTracker. SceneScript обновляется существующим сценовым путём; полный вызов жизненного цикла компонентных Script относится к этапу 19.
   - В конце `SceneTreeLayerBase::Update()` вызывается `m_TreeContainer.ResolveTransforms()`, пересчитывая мировые матрицы поддеревьев по `metadata[i].isDirty`.

2. **Параллельная фаза во `ViewManager::Update()`:**
   - Для окон отправляется RenderFrame с командами ранее подготовленного кадрового слота. RenderFrame не должен читать m_PrimaryNodes или живые GameObject.
   - Задачи окон исполняют PreRender, View::Update(time), PrepareFrame. View::Update обслуживает окно/переходы; Scene::Update вызывается один раз через SceneManager до ViewManager, не отдельно для каждого окна.
   - PrepareFrame предназначен для чтения опубликованного Front Buffer и записи собственных команд слота; фактические BuildRenderTree/SubmitRenderTree ещё заглушки, доводятся в этапе 23.
   - m_ThreadsUpdate.Join() ожидает CPU-задачи подготовки и отправки. Он не означает GPU completion; освобождение и переиспользование GPU-объектов требует fence.

3. **Точка Handover Barrier (после `Join()`, перед `PostRender()`):**
   - Во `ViewManager::Update()`:
     ```cpp
     m_ThreadsUpdate.Join();

     // Handover Barrier: сдача кадра активных сцен (Secondary -> Primary)
     if (auto scene = m_PrimaryView->GetActiveScene())
         scene->ApplyHandoverBarrier();
     for (const auto& view : m_ChildViews)
         if (auto scene = view->GetActiveScene())
             scene->ApplyHandoverBarrier();
     for (const auto& view : m_IndependentViews)
         if (auto scene = view->GetActiveScene())
             scene->ApplyHandoverBarrier();

     // PostRender: продвижение кадровых слотов после CPU-барьера
     m_PrimaryView->PostRender();
     ...
     ```
   - Метод `Scene::ApplyHandoverBarrier()` обходит все слои `m_Layers` и вызывает `layer->ApplyHandoverBarrier()`.
   - `SceneTreeLayerBase::ApplyHandoverBarrier()` вызывает `m_TreeContainer.ApplyHandoverBarrier()`.
   - В `SceneTreeContainer::ApplyHandoverBarrier()` выполняется дифференциальная синхронизация:
     - При изменении топологии синхронизируется вектор `topology`;
     - По грязным индексам из `m_SecondaryNodes.dirtyTracker.GetDirtyIndices()` обновляются `worldMatrices`, `localTransforms` и `metadata` во Front Buffer (`m_PrimaryNodes`).

4. **Очистка мёртвого кода в `SceneTreeContainer`:**
   - Поля `m_ReparentQueue`, `m_DeleteQueue` и приватный метод `ApplyDeferredQueues` удаляются из `SceneTreeContainer.h` / `.cpp` (Правило 16: Zero Technical Debt).
   - Это заменяет требование очередей из этапа 14, но не решает безопасное удаление из исполняемого callback. Этот обязательный контракт доводится вместе с lifecycle в этапе 22.

5. **Открытая семантика слоёв:**
   - Текущий SceneTreeLayerBase пропускает работу невидимого слоя. Это факт реализации, не окончательное решение: пользователь отложил вопрос обновления логики скрытых слоёв в TODO 21. При этой ревизии поведение кода не меняется.

---

## 3. Чек-лист Definition of Done (DoD)

- [x] Удалить неиспользуемые `m_ReparentQueue`, `m_DeleteQueue`, `ApplyDeferredQueues` из `SceneTreeContainer`
- [x] Реализовать `ObjectDomain.h` / `.cpp` и `EntityDomain.h` / `.cpp` в `src/engine/scene/domain/`
- [x] Реализовать `DefaultDomainFactory.h` / `.cpp` в `src/engine/scene/domain/`
- [x] Добавить методы `BeginFrame()` и `ApplyHandoverBarrier()` в интерфейс `ILayer`
- [x] Реализовать `SceneTreeLayerBase.h` / `.cpp` в `src/engine/scene/layer/`
- [x] Рефакторить `Layer3D` (наследование от `SceneTreeLayerBase`)
- [x] Реализовать `Layer2D` взамен `LayerUI` (наследование от `SceneTreeLayerBase`)
- [x] Очистить `LayerMVVM` от `ObjectWorld`/`EntityWorld` и сценового дерева
- [x] Доработать `GameObjectData` (добавить `parentGuid`, сериализация/десериализация)
- [x] Добавить вызовы `BeginFrame()` и `ApplyHandoverBarrier()` в `Scene` и подключить барьер во `ViewManager::Update()`
- [x] Зарегистрировать новые файлы в `src/engine/CMakeLists.txt`
- [x] Проверить сборку решения под MSVC x64 + Ninja через `cmd.exe /c run_build.bat` (0 ошибок)
- [x] Прогнать бенчмарки `EngineBenchmarks.exe`
- [x] Обновить статусы в `general_plan.md` и зафиксировать этап
