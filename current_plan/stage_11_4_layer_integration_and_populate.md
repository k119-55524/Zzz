# Этап 11.4: Интеграция слоёв, базовый класс `SceneTreeLayerBase`, покадровый конвейер и наполнение (`Populate`)

## 1. Контекст и цели подэтапа
- **Номер подпункта:** **Пункт 11.4** (финальная часть Этапа 11: Структура хранения GameObject в сцене).
- **Цель:** Связать все компоненты воедино на уровне слоёв сцены, кадрового цикла и загрузки пакетов:
  1. Реализация доменов: `ObjectDomain` (на базе `SlotMap`) и `EntityDomain`.
  2. Реализация 3-уровневой иерархии слоёв: `ILayer` $\to$ `SceneTreeLayerBase` (Update, Populate, владение) $\to$ `Layer3D` / `Layer2D` и автономный `LayerMVVM`.
  3. Двухпроходное наполнение слоя `SceneTreeLayerBase::Populate`: создание объектов, $TRS$, скриптов, мешей и связывание по `parentGuid`.
  4. Точная интеграция с покадровым конвейером `ViewManager` (Handover Barrier после `Join()` потоков).
  5. Проверка сквозной сборки проекта под MSVC x64 + Ninja и обновление `general_plan.md`.
- **Статус:** ⏳ В процессе разработки.
- **Зависимости:** Этапы 11.1, 11.2, 11.3.

### Физическое размещение файлов
```text
src/
└── engine/
    └── scene/
        ├── domain/
        │   ├── ObjectDomain.h / .cpp        <-- Реализация ООП-домена
        │   └── EntityDomain.h / .cpp        <-- Реализация ECS-домена
        │
        └── layer/
            ├── SceneTreeLayerBase.h / .cpp  <-- Базовый класс слоёв со сценовым деревом
            ├── Layer3D.h / .cpp             <-- 3D-слой
            ├── Layer2D.h / .cpp             <-- 2D-слой
            └── LayerMVVM.h / .cpp           <-- Очищенный декларативный UI-слой
```

---

## 2. Архитектурные спецификации

### 2.1. Базовый класс `SceneTreeLayerBase`

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

        void Update(float dt) override
        {
            if (!m_IsVisible) return;
            OnUpdateDomains(dt);
            OnUpdateSpatial();
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

### 2.2. Инициализация и двухпроходное наполнение слоя (`SceneTreeLayerBase::Populate`)
Сигнатура базового контракта:
```cpp
void SceneTreeLayerBase::Populate(const ::zzz::core::LayerData& layerData, const ::zzz::core::ScriptFactory& scriptFactory) override;
```

Пошаговый алгоритм наполнения слоя:
1. `GameObjectData` дополняется полем `Guid m_ParentGuid` (если объект корневой — `Guid::Empty()`).
2. **Проход 1 (Создание объектов и регистрация в структурах слоя):**
   - Создаётся локальная таблица отображения: `std::unordered_map<Guid, NodeHandle> guidToHandle;`
   - Для каждого `objData` из `layerData.GetObjects()`:
     - Если `objData.IsEntity()`:
       - Создаётся сущность в `m_EntityDomain` (на Этапе 11 сущности плоские, без сценового дерева, прямо в ECS домене).
     - Если объект — `GameObject` (`!objData.IsEntity()`):
       - Создается объект в объектном домене:
         `GameObject* go = m_ObjectDomain->CreateObject(objData.GetGuid(), objData.GetName());`
       - Создается узел в сценовом дереве с прямой привязкой созданного `go`:
         `NodeHandle handle = m_TreeContainer.CreateNode(objData.GetName(), go);`
       - Связывается фасад с узлом дерева и контейнером:
         `go->SetNodeHandle(handle);`
         `go->SetSceneTree(&m_TreeContainer);`
       - Устанавливаются базовые свойства:
         `go->SetActive(objData.IsActive());`
         `go->SetMeshGuid(objData.GetMeshGuid());`
         `go->SetMaterialGuid(objData.GetMaterialGuid());`
       - Устанавливается локальный $TRS$ в хранилище через прямой API `GameObject`:
         `go->SetLocalPosition(objData.GetPosition());`
         `go->SetLocalRotation(objData.GetRotation());`
         `go->SetLocalScale(objData.GetScale());`
       - Инжектируются пользовательские скрипты:
         Для каждого `sGuid` из `objData.GetScriptGuids()`:
           `auto script = scriptFactory.CreateScript(sGuid, go);`
           `if (script) go->AddScript(std::move(script));`
       - Загружаются ресурсы меша:
         При наличии `go->GetMeshGuid()` и `m_ResourceManager`:
           `m_ResourceManager->LoadDataAsset<MeshData>(go->GetMeshGuid());`
           Логируется результат загрузки (число вершин и треугольников).
       - Узел регистрируется в пространственном индексе:
         `SpatialHandle spHandle = m_SpatialStorage ? m_SpatialStorage->Insert(static_cast<uint64_t>(handle.index)) : c_InvalidSpatialHandle;`
         `m_TreeContainer.SetSpatialHandle(handle, spHandle);`
       - Сохраняется дескриптор в таблице:
         `guidToHandle[objData.GetGuid()] = handle;`

3. **Проход 2 (Связывание иерархии):**
   - Для каждого `objData` из `layerData.GetObjects()`:
     - Проверяется `parentGuid = objData.GetParentGuid();`
     - Если `parentGuid != Guid::Empty()`:
       - Извлекается дескриптор текущего узла `childHandle = guidToHandle[objData.GetGuid()];`
       - Производится поиск родителя: `auto it = guidToHandle.find(parentGuid);`
       - Если родитель найден:
         `m_TreeContainer.SetParent(childHandle, it->second, false);`
         (флаг `keepWorldTransform = false`, так как в данных задан исходный локальный $TRS$ относительно родителя).
       - Если родитель не найден в слое:
         Выводится предупреждение в лог (`Z_LOG_WARN`), объект остаётся в корне сценового дерева.

---

### 2.3. Конвейер `ViewManager` (Handover Barrier и сдача кадра)
В `ViewManager::Update()` покадровый цикл организован следующим образом:
1. **Параллельная фаза кадра $N$:**
   - Поток логики исполняет скрипты и системы, изменяя $TRS$ и накапливая структурные мутации в очередях `m_ReparentQueue` / `m_DeleteQueue` контейнера.
   - Поток рендера кадра $N-1$ читает готовый Front Buffer (`m_PrimaryNodes`).
2. **Точка завершения параллельных задач (`m_ThreadsUpdate.Join()`):**
   - Гарантия завершения всех вычислений логики и рендера.
3. **Локальный барьер сдачи кадра (Handover Barrier) перед `PostRender()`:**
   - **Применение Reparent/Delete:** пакетное применение `m_ReparentQueue` и `m_DeleteQueue` к топологии в один последовательный проход без риска реентрантности. Обе очереди работают по существующим индексам узлов и роста ёмкости хранилища не вызывают.
   - **Каскадный расчёт:** обновление мировых матриц по битовой маске `dirtyTracker` (накопленной за кадр $N$, до применения спавна) и синхронизация ячеек `ISpatialStorage`.
   - **Очистка трекера:** векторный сброс `dirtyTracker` при наличии изменений (<0.05 мс). После этого шага `dirtyTracker` гарантированно пуст.
   - **Применение Staging-спавна:** создание новых узлов из очереди спавна выполняется строго после очистки трекера. Если создание узлов требует роста `NodeStorageBlock`/`BitTreeTracker` (`EnsureCapacity`), обнуление `dirtyTracker` при этом безопасно, так как на этом шаге он уже гарантированно пуст (см. контракт `EnsureCapacity` в Этапе 11.2). Новые узлы получают первый расчёт мировой матрицы на кадре $N+1$.
   - **Переключение буферов (Front/Back Swap):** `std::swap(m_PrimaryNodes, m_SecondaryNodes)`. Буфер с актуальными матрицами становится новым Front Buffer для чтения рендером на следующем такте кадра $N+1$.
4. **`PostRender()`:** переключение цепочки показа (Swapchain Present).

---

## 3. Чек-лист Definition of Done (DoD)

- [ ] Реализовать `ObjectDomain.h` / `.cpp` и `EntityDomain.h` / `.cpp` в `src/engine/scene/domain/`
- [ ] Реализовать `SceneTreeLayerBase.h` / `.cpp` в `src/engine/scene/layer/`
- [ ] Обновить `Layer3D` (наследование от `SceneTreeLayerBase`)
- [ ] Реализовать `Layer2D` взамен `LayerUI` (наследование от `SceneTreeLayerBase`)
- [ ] Очистить `LayerMVVM` от доменов и сценового дерева
- [ ] Доработать `GameObjectData` (`parentGuid`) и двухпроходное связывание в `SceneTreeLayerBase::Populate`
- [ ] Зарегистрировать все файлы в `src/engine/CMakeLists.txt`
- [ ] Проверить сборку всего решения под MSVC x64 + Ninja (0 ошибок)
- [ ] Обновить статусы в `general_plan.md`
