# Этап 11: Структура хранения GameObject в сцене, создание и инициализация

## 1. Контекст и цели этапа
- **Номер пункта:** **Пункт 11** (Уровень 2: GAPI-ресурсы, содержимое куба и сквозной рендер).
- **Цель:** Сформировать расширяемую архитектуру хранения, иерархии и пространственной выборки объектов внутри слоёв сцены (`ILayer`):
  1. **Слои сцены и специализация (с базовым классом `SceneTreeLayerBase`):**
     - **`SceneTreeLayerBase : public ILayer` (новый базовый класс):** Инкапсулирует 100% общей логики слоёв со сценовым графом: владеет `SceneTreeContainer`, `ISpatialStorage`, фабрикой доменов `IDomainFactory` и самими доменами `IObjectDomain`/`IEntityDomain`, реализует общий цикл `Update(dt)` и единое двухпроходное наполнение `Populate(LayerData, scriptFactory)`.
     - **`Layer3D : public SceneTreeLayerBase`:** Игровой 3D-мир. Возвращает `eLayerType::Layer3D`, создаёт 3D-пространственный индекс и 3D-фабрику доменов.
     - **`Layer2D : public SceneTreeLayerBase`** (замена устаревшего `LayerUI`): Полноценный 2D-мир (спрайты, 2D-физика) + плоский GUI / HUD. Возвращает `eLayerType::Layer2D`, создаёт 2D-пространственный индекс и 2D-фабрику доменов.
     - **`LayerMVVM : public ILayer`:** Реактивный декларативный UI (ZzzGUI). Наследуется напрямую от `ILayer`. **Домены, `SceneTreeContainer` и пространственное хранилище не нужны**, фабрика не нужна. Содержит дерево элементов интерфейса (`UIElement`, `Grid`, `Button`) и привязку к `ViewModel`.
  2. **Дерево сцены (Scene Graph / `SceneNode`):** Владеет плоским двухбуферным блоком `NodeStorageBlock`. Задает топологию («родитель-потомок»), хранит $TRS$, вычисляет мировые матрицы и управляет каскадным флагом через `BitTreeTracker`. Дерево **не производит вызовов логики `Update`**.
  3. **Домены объектов и сущностей (`ILayerDomain`) и фабрика (`IDomainFactory`):** Базовый контракт для доменов слоя. Разделение на `IObjectDomain` (ООП-объекты `GameObject`) и `IEntityDomain` (ECS-сущности `EntityID`) с созданием через фабрику `IDomainFactory`.
  4. **Пространственный ускоритель (`ISpatialStorage`):** Отдельный геометрический класс (один на слой в `Layer3D` и `Layer2D`). Хранит мировые AABB/границы в пространственных структурах и предоставляет метод `GetAll` для передачи видимых объектов в рендер.
  5. **Инициализация слоя (`Populate`):** Единое двухпроходное наполнение слоя в `SceneTreeLayerBase` из `LayerData` / `GameObjectData` с сохранением свойств объектов, скриптов `ScriptFactory`, ресурсов мешей и построением связей дерева.
- **Статус:** ⏳ В процессе согласования и разработки.
- **Зависимости:** `src/core/containers/BitTreeTracker.h`, `src/engine/scene/GameObject.h`, `src/engine/scene/Transform.h`, `src/engine/scene/layer/ILayer.h`, `src/engine/scene/storage/ISpatialStorage.h`, `src/core/enums/eLayerType.h`.

### 1.1. Физическое размещение компонентов в файловой структуре

Для соблюдения модульности, принципа единственной ответственности (SRP) и переиспользования:
```text
src/
├── core/
│   └── containers/
│       ├── BitTreeTracker.h / .cpp       <-- 64-арное битовое дерево (общесистемная утилита ядра)
│
└── engine/
    └── scene/
        ├── domain/                       <-- Домены объектов и сущностей
        │   ├── ILayerDomain.h            <-- Базовый интерфейс домена
        │   ├── IObjectDomain.h           <-- Интерфейс ООП-домена (GameObject)
        │   ├── IEntityDomain.h           <-- Интерфейс ECS-домена (Entity)
        │   ├── IDomainFactory.h          <-- Фабрика создания доменов
        │   ├── ObjectDomain.h / .cpp     <-- Реализация ООП-домена (замена ObjectWorld)
        │   └── EntityDomain.h / .cpp     <-- Реализация ECS-домена (замена EntityWorld)
        │
        ├── storage/                      <-- Пространственные структуры и SoA хранилище
        │   ├── ISpatialStorage.h         <-- Интерфейс пространственного ускорения
        │   ├── DefaultSpatialStorage.h/.cpp <-- Базовое хранилище (пространственные слоты O(1))
        │   ├── NodeStorageBlock.h / .cpp <-- SoA-блок (topology, transforms, matrices, bounds)
        │   └── SceneTreeContainer.h/.cpp <-- Двухбуферный контейнер сцены слоя
        │
        ├── layer/                        <-- Иерархия слоёв сцены
        │   ├── ILayer.h                  <-- Базовый интерфейс слоя
        │   ├── SceneTreeLayerBase.h/.cpp <-- Базовый класс для слоёв со сценовым деревом
        │   ├── Layer3D.h / .cpp          <-- 3D слой
        │   ├── Layer2D.h / .cpp          <-- 2D слой (бывший LayerUI)
        │   └── LayerMVVM.h / .cpp        <-- Декларативный UI слой
        │
        ├── GameObject.h / .cpp           <-- Логический фасад объекта сцены
        ├── Transform.h / .cpp            <-- Логический прокси-фасад трансформа
        └── Scene.h / .cpp                <-- Сцена
```

---

## 2. Архитектурные спецификации

### 2.1. Иерархия интерфейсов доменов (`ILayerDomain`) и фабрика (`IDomainFactory`)

Любой домен внутри слоя (как ООП, так и ECS) реализует базовый контракт `ILayerDomain`:

```cpp
class ILayerDomain
{
public:
    virtual ~ILayerDomain() = default;

    // 1. Фаза игровой логики (вызывается строго 1 раз в начале кадра)
    virtual void Update(const zzz::core::Time& time) = 0;

    // 2. Фаза пространственной синхронизации (после пересчета матриц дерева)
    virtual void UpdateSpatialStorage(ISpatialStorage& storage) = 0;

    // 3. Жизненный цикл и диагностика
    virtual void Clear() = 0;
    [[nodiscard]] virtual size_t GetCount() const noexcept = 0;
};
```

Специализированные интерфейсы:
- **`IObjectDomain : public ILayerDomain`** — контракт для ООП-домена:
  - `virtual GameObject* CreateObject(const Guid& guid, std::string name) = 0;`
  - `virtual void DestroyObject(GameObject* obj) = 0;`
- **`IEntityDomain : public ILayerDomain`** — контракт для ECS-домена:
  - `virtual EntityID CreateEntity(const Guid& guid, std::string name) = 0;`
  - `virtual void DestroyEntity(EntityID entity) = 0;`

Интерфейс фабрики доменов (`IDomainFactory`):
```cpp
class IDomainFactory
{
public:
    virtual ~IDomainFactory() = default;
    virtual std::unique_ptr<IObjectDomain> CreateObjectDomain() = 0;
    virtual std::unique_ptr<IEntityDomain> CreateEntityDomain() = 0;
};
```
- Фабрика, оба домена, `SceneTreeContainer` и `ISpatialStorage` хранятся как защищённые поля в базовом классе **`SceneTreeLayerBase`**.
- В конструкторах производных классов `Layer3D` и `Layer2D` передаются конкретные реализации фабрики (`DomainFactory3D` / `DomainFactory2D`) и хранилища (`DefaultSpatialStorage`), которые инициализируют поля базового класса.

---

### 2.2. Пространственный индекс (`ISpatialStorage`)

В соответствии с **Правилом 17** (строгая очерёдность этапов: `Camera`/`Frustum` — Этап 18, `RenderQueue` — Этап 20, физика/`Raycast` — Этап 25), на **Этапе 11** интерфейс `ISpatialStorage` реализует фундамент пространственного хранения над существующими типами (`math::Vec3`, `uint64_t userData`), без преждевременного введения несуществующих классов:

```cpp
namespace zzz
{
    using SpatialHandle = uint32_t;
    constexpr SpatialHandle c_InvalidSpatialHandle = 0xFFFFFFFF;

    struct SpatialBounds final
    {
        ::zzz::math::Vec3<zF32> min{ -0.5f, -0.5f, -0.5f };
        ::zzz::math::Vec3<zF32> max{  0.5f,  0.5f,  0.5f };
    };

    class ISpatialStorage
    {
    public:
        virtual ~ISpatialStorage() = default;

        // Регистрация и обновление пространственного положения
        virtual SpatialHandle Insert(const SpatialBounds& bounds, uint64_t userData) = 0;
        virtual void Update(SpatialHandle handle, const SpatialBounds& newBounds) = 0;
        virtual void Remove(SpatialHandle handle) = 0;
        virtual void Clear() = 0;

        // Базовая выборка всех зарегистрированных объектов
        virtual void GetAll(std::vector<uint64_t>& outUserData) const = 0;

        // Точки расширения на последующих этапах:
        // - QueryFrustum(const Camera&) появится на Этапе 18 (Камера и проекции)
        // - QueryAABB / QueryScreenRect появится на Этапе 18 / 20
        // - Raycast появится на Этапе 25 (Физический мир)
    };
}
```

Реализация `DefaultSpatialStorage` заменяет старый `DefaultSceneStorage` и хранит пространственные элементы с быстрым $O(1)$ доступом по `SpatialHandle`.

### 2.3. Иерархия слоёв сцены и базовый класс `SceneTreeLayerBase`

Архитектурное разделение слоёв по ответственности:
1. **Уровень 1 (Интерфейс):** `ILayer` — общий контракт жизненного цикла (`GetName`, `GetType`, `IsVisible`, `Update(dt)`, `Populate`).
2. **Уровень 2 (Базовый класс сценового графа):** `SceneTreeLayerBase : public ILayer` — инкапсулирует 100% общей логики слоёв, обладающих пространственным деревом и доменами (`Layer3D` и `Layer2D`).
3. **Уровень 3 (Конкретные реализации):**
   - `Layer3D final : public SceneTreeLayerBase` — 3D-мир.
   - `Layer2D final : public SceneTreeLayerBase` — 2D-мир / плоский HUD.
   - `LayerMVVM final : public ILayer` — декларативный UI (ZzzGUI). Наследуется **напрямую от `ILayer`**, не содержит ECS, пространственных деревьев и $TRS$-матриц; его `Update(dt)` обновляет только таймеры, анимации и привязки данных ViewModel.

```cpp
namespace zzz::engine
{
    /**
     * @class SceneTreeLayerBase
     * @brief Базовый класс для слоёв сцены, обладающих сценовым деревом и доменами (Layer3D, Layer2D).
     */
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

        // Общий пайплайн кадрового обновления
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
        // Виртуальные шаги кадрового цикла (для кастомизации физики/анимаций в 3D/2D при необходимости)
        virtual void OnUpdateDomains(float dt)
        {
            if (m_ObjectDomain) m_ObjectDomain->Update(dt);
            if (m_EntityDomain) m_EntityDomain->Update(dt);
        }

        virtual void OnUpdateSpatial()
        {
            m_TreeContainer.ResolveTransformsAndSpatial(m_SpatialStorage.get());
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

#### 2.3.1. Инициализация и двухпроходное наполнение слоя (`SceneTreeLayerBase::Populate`)
Сигнатура строго соответствует базовому контракту:
`void SceneTreeLayerBase::Populate(const ::zzz::core::LayerData& layerData, const ::zzz::core::ScriptFactory& scriptFactory)`

Вся существующая бизнес-логика наполнения объектов из `Layer3D` сохраняется и централизуется:
1. `GameObjectData` дополняется полем `Guid m_ParentGuid` (если объект корневой — `Guid::Empty()`).
2. **Проход 1 (Создание объектов и регистрация в структурах слоя):**
   - Для каждого `objData` из `layerData.GetObjects()`:
     - Если `objData.IsEntity()`: создаётся сущность в `m_EntityDomain` (на Этапе 11 сущности плоские, без дерева).
     - Если `GameObject`:
       - Создается узел в `SceneTreeContainer` (`NodeHandle handle = m_TreeContainer.CreateNode(...)`).
       - Создается объект в домене: `GameObject* go = m_ObjectDomain->CreateObject(objData.GetGuid(), objData.GetName())`.
       - `go->SetNodeHandle(handle)` — связь фасада с узлом.
       - Устанавливаются свойства: `go->SetActive(objData.IsActive())`, `go->SetMeshGuid(objData.GetMeshGuid())`, `go->SetMaterialGuid(objData.GetMaterialGuid())`.
       - Устанавливается локальный $TRS$ в хранилище через `go->GetTransform().SetLocalPosition(...)`, `SetLocalRotation(...)`, `SetLocalScale(...)`.
       - Инжектируются скрипты: для каждого `sGuid` из `objData.GetScriptGuids()` вызывается `scriptFactory.CreateScript(sGuid, go)` с добавлением `go->AddScript(script)`.
       - Загружаются ресурсы: при наличии `MeshGuid` и `m_ResourceManager` запрашивается `LoadDataAsset<MeshData>(go->GetMeshGuid())` с логированием числа вершин и треугольников.
       - Узел регистрируется в `m_SpatialStorage->Insert(bounds, userData)`.
       - Индекс узла сохраняется во временной таблице `std::unordered_map<Guid, NodeHandle> guidToHandle`.
3. **Проход 2 (Связывание иерархии):**
   - Для каждого объекта с `parentGuid != Guid::Empty()`:
     - Находится родительский `parentHandle` в таблице;
     - Вызывается `m_TreeContainer.SetParent(childHandle, parentHandle, false)` (локальный $TRS$ сохраняется как был в данных);
     - Если родитель не найден в слое — объект остаётся в корне с предупреждением в лог.

---

### 2.4. Структура хранения узлов (`SceneNode`) и плоские массивы слоя

#### Единый источник истины и роль `GameObject`:
- **`SceneNode` — единственный источник истины (Single Source of Truth) для топологии и трансформации.**
  - Хранится в плоских массивах `SceneTreeContainer`.
  - Занимает минимальный компактный размер в памяти (~80 байт), обеспечивая высокую кэш-локальность при перемножении матриц.
  - Поддерживает полиморфную полезную нагрузку (Payload): может связываться как с классическим `GameObject` (из `IObjectDomain`), так и с легковесным `EntityID` (из `IEntityDomain` ECS), либо быть пустым узлом-группировкой (Pivot/Folder).
- **`GameObject` — логический фасад (Logical Entity / Component Container):**
  - **Не хранит физические данные `Transform`:** координаты $X, Y, Z$, поворот и масштаб лежат в плоском непрерывном массиве `NodeStorageBlock::localTransforms`.
  - Поля `m_Parent`, `m_Children`, `m_Transform` удаляются из `GameObject`.
  - `GameObject` хранит только дескриптор `NodeHandle m_NodeHandle` (индекс узла и версия `generation`) и ссылку на контейнер сцены.
  - Класс `Transform` превращается в легковесный прокси-фасад (`TransformView`), который перенаправляет вызовы `SetPosition/GetPosition` напрямую в плоский массив `localTransforms[nodeIndex]`.
  - Топологические методы (`GetParent()`, `SetParent()`) становятся инлайновыми делегатами в `SceneTreeContainer`.
- **`Transform::SetDirty()` становится строгой операцией $O(1)$ без обхода детей:**
  - Из `Transform.cpp` полностью удаляется старый цикл с обходом детей `m_Owner.GetChild(i)->GetTransform().SetDirty()`.
  - При вызове `SetLocalPosition/Rotation/Scale` выставляется только локальный флаг в массиве `isDirty[nodeIndex] = true` и взводится 1 бит в трекере своего блока через `m_Container->MarkDirty(m_NodeHandle)` за $O(1)$.
  - Дети не трогаются вообще; каскадный спуск родительского флага dirty происходит строго 1 раз в фазе Handover Barrier при линейном обходе плоского массива.

```cpp
enum class SceneNodeType : uint8_t
{
    Empty,      // Папка / Pivot / пустой узел
    GameObject, // Логический ООП объект (ObjectDomain) — основной тип Этапа 11
    Entity      // Легковесная ECS сущность (EntityDomain) — зарезервировано под интеграцию ECS-дерева в Этапе 14
};

// Компактные структуры для SoA (Structure of Arrays)
struct NodeTopology {
    uint32_t parentIndex{ 0xFFFFFFFF };
    uint32_t firstChildIndex{ 0xFFFFFFFF };
    uint32_t nextSiblingIndex{ 0xFFFFFFFF };
    uint32_t prevSiblingIndex{ 0xFFFFFFFF };
}; // Ровно 16 байт (4 топологии на 1 кэш-линию 64B!)

struct LocalTransform {
    ::zzz::math::Vec3<zF32> position{ 0.0f, 0.0f, 0.0f };
    ::zzz::math::Quat<zF32> rotation{ 0.0f, 0.0f, 0.0f, 1.0f }; // Истинный Identity (0, 0, 0, 1)
    ::zzz::math::Vec3<zF32> scale{ 1.0f, 1.0f, 1.0f };
}; // Ровно 40 байт

struct NodeMetadata {
    std::string   name;
    SceneNodeType payloadType{ SceneNodeType::Empty };
    union {
        GameObject* gameObject{ nullptr };
        uint32_t    entityId; // Зарезервировано: активация в Этапе 14 (на Этапе 11 активна ветка gameObject)
    };
    SpatialHandle spatialHandle{ 0 };
    uint32_t      generation{ 1 };
    bool          isAlive{ true };
    bool          isActive{ true };
    bool          isDirty{ false };
}; // Холодный поток метаданных
```

#### 2.4.1. Унифицированный блок хранения узлов в SoA (`NodeStorageBlock`)
Разделение на параллельные непрерывные векторы гарантирует идеальную кэш-локальность (DOD Правило 13):

```cpp
class NodeStorageBlock final
{
public:
    // Горячие потоки (Hot Streams): читаются SIMD-инструкциями при расчете матриц и отсечении
    std::vector<NodeTopology>         topology;        // [idx] 16 байт
    std::vector<LocalTransform>       localTransforms; // [idx] 40 байт
    std::vector<math::Mat4<zF32>>     worldMatrices;   // [idx] 64 байта (1 кэш-линия на матрицу!)
    std::vector<SpatialBounds>        worldBounds;     // [idx] 24 байта (min/max Vec3)

    // Холодный поток (Cold Stream): метаданные, имена, полезная нагрузка
    std::vector<NodeMetadata>         metadata;        // [idx]

    // Битовый трекер изменений
    BitTreeTracker dirtyTracker;

    void EnsureCapacity(size_t requiredCapacity)
    {
        if (requiredCapacity > topology.capacity())
        {
            topology.reserve(requiredCapacity);
            localTransforms.reserve(requiredCapacity);
            worldMatrices.reserve(requiredCapacity);
            worldBounds.reserve(requiredCapacity);
            metadata.reserve(requiredCapacity);
        }
        dirtyTracker.EnsureCapacity(topology.capacity());
    }

    void MarkDirty(uint32_t nodeIndex)
    {
        metadata[nodeIndex].isDirty = true;
        dirtyTracker.Set(nodeIndex);
    }

    void ResolveTransformsAndSpatial(ISpatialStorage* spatialStorage);
    void ClearDirty() { dirtyTracker.Clear(); }
};
```
#### 2.4.2. Архитектура двухбуферного контейнера сцены (`SceneTreeContainer`) с двойной буферизацией и Staging
В соответствии с Правилом 14 (изоляция кадров и потоков) и согласованной архитектурой:
- Контейнер `SceneTreeContainer` содержит **двойной буфер узлов**:
  - `NodeStorageBlock m_PrimaryNodes` — активный буфер, из которого текущий кадр читает топологию, локальные и мировые матрицы.
  - `NodeStorageBlock m_SecondaryNodes` — теневой буфер для дефрагментации и непрерывной компоновки.
- **Поддержка поколений (`generation`):** структура `NodeHandle` содержит `{ uint32_t index; uint32_t generation; }`. При удалении узла его `generation` инкрементируется, предотвращая проблему ABA при повторном использовании слотов памяти.
- **Staging-спавн без блокировок:**
  - Создание новых объектов во время кадра (`SpawnGameObject`, спавн префабов) не меняет активный массив на лету, а помещается во временный плоский буфер `m_StagingSpawn` за $O(1)$.
- **Отложенные очереди мутаций топологии:**
  - `m_ReparentQueue` — запросы смены родителя накапливаются во время кадра и применяются батчем.
  - `m_DeleteQueue` — запросы удаления узлов накапливаются и освобождаются с возвратом слотов в `m_FreeIndices` и инкрементом `generation`.
- **Точка слияния и дефрагментация (Handover Barrier):**
  - Применение `m_ReparentQueue` и `m_DeleteQueue`.
  - Перенос объектов из `m_StagingSpawn` в основной массив (или свободные слоты `m_FreeIndices`) с регистрацией в `ISpatialStorage`.
  - При необходимости дефрагментации: фоновое уплотнение в `m_SecondaryNodes` с последующим атомарным `std::swap(m_PrimaryNodes, m_SecondaryNodes)`.
- Для передачи кадровых команд между потоками используется `SwapQueue<T>` (Правило 14).

#### 2.4.3. Контракт доступа к иерархии (`GameObject` Hierarchy API) и миграция `DestroyObject`
В соответствии с Правилом 16 (YAGNI):
- **Удаление устаревших коллекций и методов дочерних элементов:**
  - Физический вектор `std::vector<GameObject*> m_Children` удаляется из `GameObject`.
  - Методы `GetChildren()`, `GetChildCount()` и `GetChild(size_t index)` **удаляются насовсем**: они больше нигде в движке не вызываются (старый обход в `Transform::SetDirty()` заменён на $O(1)$ бит в `BitTreeTracker`).
  - Это избавляет узел `NodeTopology` от лишнего поля `childCount` (экономия памяти и идеальное выравнивание 16 байт) и предотвращает скрытые линейные проходы по списку сиблингов.
- **В `GameObject` остаются строго необходимые методы иерархии:**
  - `GameObject* GetParent() const noexcept;`
  - `void SetParent(GameObject* newParent, bool keepWorldTransform = true) noexcept;`
  - Топология («родитель-потомок») полностью инкапсулирована в `SceneTreeContainer` (`NodeTopology: parentIndex, firstChildIndex, nextSiblingIndex, prevSiblingIndex`). Любой обход поддерева при необходимости (например, для UI иерархии в будущем) будет производиться напрямую через контейнер сцены.
- **Миграция удаления `ObjectWorld::DestroyObject` (в `ObjectDomain`):**
  - Старый рекурсивный вызов C++ стека `while (obj->GetChildCount() > 0) DestroyObject(obj->GetChild(0));` **полностью удаляется**.
  - Вместо рекурсии уничтожение выполняет метод `SceneTreeContainer::DestroySubtree(NodeHandle rootHandle)` **строго итеративно** (без вызовов функций на стеке CPU):
    - Используется плоский буфер индексов (BFS/DFS очередь `std::vector<uint32_t>`);
    - Контейнер итеративно собирает индексы всех потомков по `firstChildIndex` и `nextSiblingIndex`;
    - Линейным циклом для каждого собранного индекса: уведомляется и удаляется `GameObject*` из `ObjectDomain` (если привязан), удаляется `spatialHandle` из `ISpatialStorage`;
    - Слоты возвращаются в `m_FreeIndices`, их `generation` инкрементируется ($O(1)$ на узел), корень поддерева отвязывается от родителя (`prev/next/firstChild` перелинковываются). Никаких рекурсивных вызовов.

### 2.5. Архитектура параллелизма `ViewManager` (Fork-Join) и покадровый конвейер
В соответствии с реальной архитектурой `ViewManager::Update()` и **Правилом 14**:
- В `ThreadPool` (`m_ThreadsUpdate`) на каждом тике запускаются параллельные задачи:
  1. **Задача отрисовки кадра $N-1$ на GPU:** `RenderManager::RenderFrame()`. Физически изолирована от `Scene` (работает только со статичным снапшотом команд рендера в GAPI).
  2. **Задачи подготовки кадра $N$ для каждого активного `View`:** последовательность `PreRender() -> Update(dt) -> PrepareFrame(scene) -> BuildRenderTree(scene)`.
- **Точка слияния (Join Barrier):** сразу после `m_ThreadsUpdate.Join()` локально для каждой сцены наступает завершение кадра перед `PostRender()`.

```text
[ВРЕМЕННАЯ ДИАГРАММА ТИКА ViewManager::Update()]
Task GPU (N-1):  |================ RenderManager::RenderFrame() (Без доступа к Scene) ================|
Task View_0 (N): |== BeginFrame -> Update(dt) -> ResolveTransforms -> PrepareFrame(Scene) ============|
Task View_K (N): |== BeginFrame -> Update(dt) -> ResolveTransforms -> PrepareFrame(Scene) ============|
                                                                                                        |
                                         [ ТОЧКА m_ThreadsUpdate.Join() / HANDOVER BARRIER ] <----------+
                                         (Локально для каждой Scene/Слоя перед PostRender, < 0.1 мс):
                                         1. Обработка Staging-спавна, ReparentQueue, DeleteQueue
                                         2. Каскадный расчёт Transform/Spatial и очистка BitTreeTracker
                                         3. Дефрагментация / Swap буферов при необходимости
                                         4. Готовность снапшота к отрисовке на GPU в кадре N+1
```

#### Фаза 1: Параллельное выполнение задач тика
- **В задаче `View` (Кадр $N$):**
  - **Старт кадра (`BeginFrame`):** вызов `dirtyTracker.EnsureCapacity(storage.topology.size())` и сброс остаточных битов при необходимости.
  - `Update(dt)`: исполняется логика объектов.
    - При изменении Transform выставляется `metadata[nodeIndex].isDirty = true` и взводится 1 бит в `BitTreeTracker::Set(nodeIndex)` ($O(1)$).
    - Динамический спавн объектов отправляется в `m_StagingSpawn` ($O(1)$).
    - Смена родителей отправляется в `m_ReparentQueue` ($O(1)$).
    - Запросы удаления отправляются в `m_DeleteQueue` ($O(1)$).
  - **Каскадный расчёт (`ResolveTransformsAndSpatial`):**
    - Обход по битам `BitTreeTracker`, вычисление `worldMatrix` и `worldBounds`, прямое обновление ячейки `ISpatialStorage` по `spatialHandle`, сброс `node.isDirty = false`.
  - **`PrepareFrame(const Scene& scene)` $\to$ `BuildRenderTree()`:**
    - Запрашивает зарегистрированные объекты через `ISpatialStorage::GetAll` (на Этапе 18 будет заменено на `QueryFrustum`) и строит снимок для рендера следующего такта.
- **В задаче GPU (Кадр $N-1$):**
  - `RenderManager::RenderFrame()` исполняет подготовленный на прошлом такте снапшот команд. Сцену не читает и не блокирует.

#### Фаза 2: Точка передачи кадра (Handover Barrier)
Наступает сразу после `m_ThreadsUpdate.Join()` локально для каждой сцены перед `PostRender()`:
1. **Шаг 1: Завершение параллельных задач тика:** `m_ThreadsUpdate.Join()` гарантирует завершение GPU кадра $N-1$ и кадра $N$ логики.
2. **Шаг 2: Обработка очередей мутаций:**
   - Применение `m_DeleteQueue`: освобождение слотов в `m_FreeIndices`, инкремент `generation`, удаление из `ISpatialStorage`.
   - Применение `m_ReparentQueue`: перелинковка топологии `firstChild / nextSibling / prevSibling`.
   - Перенос объектов из `m_StagingSpawn` в активный буфер `m_PrimaryNodes` с регистрацией в `ISpatialStorage`.
3. **Шаг 3: Очистка битовых масок изменений:** проверка корня `BitTreeTracker`, если корень $\neq 0$ — векторный сброс буфера битовой маски (`memset` / векторное зануление).
4. **Шаг 4: Дефрагментация (при необходимости):** упаковка слотов в `m_SecondaryNodes` и атомарный `std::swap(m_PrimaryNodes, m_SecondaryNodes)`.
5. **Шаг 5: Завершение кадра (`PostRender`):** подготовленный снапшот готов к отправке на отрисовку в следующем такте (кадр $N+1$).

---

### 2.6. Сводка жизненного цикла в плоской двухбуферной архитектуре

| Операция | Поведение в потоке логики / View-задаче | Поведение в точке передачи (Handover Barrier) |
|---|---|---|
| **Изменение Transform** | `isDirty = true`, взвод бита в `BitTreeTracker` ($O(1)$) | Каскадный расчёт `worldMatrix` + прямое обновление `ISpatialStorage` в `ResolveTransformsAndSpatial()` |
| **Очистка трекера** | Не выполняется | Проверка корня на 0; если $\neq 0$ — векторная заливка нулями |
| **Спавн ГО / Префаба** | Добавление в плоский `StagingSpawn` без блокировок ($O(1)$) | Перенос в свободные слоты `m_FreeIndices` или конец основного массива + `Insert` в `ISpatialStorage` |
| **Смена парента** | Запись команды в `m_ReparentQueue` ($O(1)$) | Перелинковка по `prev/nextSibling` без сдвига памяти + сохранение Sibling Order |
| **Удаление (Destroy)** | Запись индекса в `m_DeleteQueue` ($O(1)$) | Каскадное зануление ветки детей, возврат слотов в `m_FreeIndices`, инкремент `generation`, `Remove` из `ISpatialStorage` |
| **Дефрагментация** | Фоновая упаковка в `m_SecondaryNodes` при необходимости | Атомарный `std::swap(m_PrimaryNodes, m_SecondaryNodes)` после расчёта матриц |

---

## 3. Чек-лист Definition of Done (DoD)

- [ ] Переименовать `eLayerType::LayerUI` в `eLayerType::Layer2D` в `src/core/enums/eLayerType.h`
- [ ] Спроектировать интерфейсы `ILayerDomain`, `IObjectDomain`, `IEntityDomain` и `IDomainFactory` в `src/engine/scene/domain/`
- [ ] Разработать новый интерфейс пространственного хранилища `ISpatialStorage` (замена устаревшего `ISceneStorage` на геометрию `SpatialHandle`, `SpatialBounds`, `userData`, метод `GetAll`)
- [ ] Разработать отдельный класс `BitTreeTracker` в `src/core/containers/BitTreeTracker.h` / `.cpp` (единый плоский массив `std::vector<uint64_t>`, динамическая глубина, привязка `EnsureCapacity` к размеру слоя, быстрый обход `_BitScanForward64`, очистка)
- [ ] Разработать модульные тесты для `BitTreeTracker` в `tests/core/BitTreeTrackerTests.cpp` (пустое дерево, установка битов, граничные индексы, динамический рост, блочная очистка)
- [ ] Перевести `ObjectWorld` $\to$ `ObjectDomain` и `EntityWorld` $\to$ `EntityDomain` с реализацией интерфейсов
- [ ] Реализовать структуру `SceneNode` (единый источник истины, Payload под `GameObject`/`Entity`) и класс `NodeStorageBlock` (SoA-векторы)
- [ ] Реализовать контейнер `SceneTreeContainer` (плоские double-буферы `PrimaryNodes`/`SecondaryNodes`, `parentIndex`, `firstChildIndex`, `nextSiblingIndex`, `prevSiblingIndex`, переиспользование слотов через `m_FreeIndices` с проверкой поколений `generation`, интеграция с `BitTreeTracker`, спавн в `m_StagingSpawn`, очереди `m_ReparentQueue`/`m_DeleteQueue`, дефрагментация с атомарным `swap`)
- [ ] Рефакторить `GameObject` в логический фасад: удалить сырые поля `m_Parent`/`m_Children`, связать с `NodeHandle(index, generation)`, оставить методы иерархии `GetParent`, `SetParent` (с делегированием в `SceneTreeContainer`), удалить `GetChildren`, `GetChildCount`, `GetChild`, делегировать `Transform` в `SceneTreeContainer`
- [ ] Реализовать базовый класс `SceneTreeLayerBase : public ILayer` (общее владение `SceneTreeContainer`, `ISpatialStorage`, доменами, `Update(dt)` и двухпроходный `Populate`)
- [ ] Обновить `Layer3D : public SceneTreeLayerBase`: специализация под 3D-мир
- [ ] Реализовать `Layer2D : public SceneTreeLayerBase` (на замену `LayerUI`): специализация под 2D-мир
- [ ] Очистить `LayerMVVM : public ILayer`: чистое UI-дерево без лишних доменов
- [ ] Доработать `GameObjectData` (`parentGuid` / `parentIndex`) и интеграцию в `SceneTreeLayerBase::Populate`
- [ ] Проверить сборку под MSVC x64 + Ninja (0 ошибок)
- [ ] Обновить статус этапа в `general_plan.md`

