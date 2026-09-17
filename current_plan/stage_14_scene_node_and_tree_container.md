# Этап 14: Структура узлов сцены, SoA-хранилище, ISceneTreeAccessor, удаление Transform и рефакторинг GameObject (`SceneNode`, `SceneTreeContainer`, `GameObject`)

## 1. Контекст и цели этапа
- **Номер пункта:** **Пункт 14** (Уровень 2: GAPI-ресурсы, содержимое куба и сквозной рендер).
- **Цель:** Реализовать плоское высокопроизводительное SoA-хранилище сцены и превратить `GameObject` в легковесный фасад:
  1. Единый источник истины: компактные структуры `NodeTopology`, `LocalTransform`, `NodeMetadata` и блок `NodeStorageBlock` (SoA-векторы по Правилу 13 DOD).
  2. Двухбуферный контейнер `SceneTreeContainer`: опубликованный `m_PrimaryNodes`, рабочий `m_SecondaryNodes`, переиспользование слотов `m_FreeIndices` с проверкой `generation`. Staging и очереди мутаций были частью раннего проекта, но не действующего контракта после этапа 15.
  3. **Полная ликвидация `class Transform`** (`Transform.h` / `Transform.cpp` удаляются): избавление от лишней ООП-прослойки и проблем выравнивания (C4324).
  4. Введение скрытого интерфейса `ISceneTreeAccessor`: `GameObject` хранит `NodeHandle` и указатель на интерфейс `ISceneTreeAccessor*`. Методы `SetLocalPosition`, `GetWorldMatrix` вызываются у `GameObject` напрямую и делегируются в плоский массив `SceneTreeContainer` за $O(1)$.
  5. Итеративный обход поддерева при удалении (`DestroySubtree`) без рекурсии на стеке CPU.
- **Статус:** ✅ Выполнено.

> Уточнение 2026-09-10: интеграция и актуализация выполнены в [этапе 15](stage_15_layer_integration_and_populate.md). Неиспользуемые очереди удалены; не восстанавливать их по старым наброскам. Безопасные мутации при исполнении компонентных скриптов предстоит довести в этапе 22. Статус реализации не означает, что незаполненный пункт прогона модульных тестов ниже уже подтверждён.
- **Зависимости:** `BitTreeTracker.h`, `ISpatialStorage.h`, `src/math/matrix/Mat4.h`, `src/math/quat/Quat.h`, `src/math/vector/Vec3.h`.

### Физическое размещение файлов
```text
src/
└── engine/
    └── scene/
        ├── storage/
        │   ├── NodeStorageBlock.h / .cpp    <-- SoA блок данных и расчет матриц
        │   ├── SceneTreeContainer.h / .cpp  <-- Двухбуферный менеджер дерева сцены (реализует ISceneTreeAccessor)
        │   └── ISceneTreeAccessor.h         <-- Скрытый интерфейс доступа к плоскому массиву сцены
        │
        └── gameobject/
            └── GameObject.h / .cpp          <-- Фасад объекта сцены (Transform удален!)
```

---

## 2. Архитектурные спецификации

### 2.1. Структуры SoA (`NodeStorageBlock`)

```cpp
namespace zzz::engine
{
    enum class SceneNodeType : uint8_t
    {
        Empty,
        GameObject, // Основной тип Этапа 11
        Entity      // Зарезервировано: активация в Этапе 14
    };

    struct NodeTopology {
        uint32_t parentIndex{ 0xFFFFFFFF };
        uint32_t firstChildIndex{ 0xFFFFFFFF };
        uint32_t nextSiblingIndex{ 0xFFFFFFFF };
        uint32_t prevSiblingIndex{ 0xFFFFFFFF };
    }; // Ровно 16 байт

    struct LocalTransform {
        ::zzz::math::Vec3<zF32> position{ 0.0f, 0.0f, 0.0f };
        ::zzz::math::Quat<zF32> rotation{ 0.0f, 0.0f, 0.0f, 1.0f }; // Identity
        ::zzz::math::Vec3<zF32> scale{ 1.0f, 1.0f, 1.0f };
    }; // Ровно 40 байт

    struct NodeMetadata {
        std::string   name;
        SceneNodeType payloadType{ SceneNodeType::Empty };
        union {
            GameObject* gameObject{ nullptr };
            uint32_t    entityId; // Зарезервировано под Этап 14
        };
        SpatialHandle spatialHandle{ 0 };
        uint32_t      generation{ 1 };
        bool          isAlive{ true };
        bool          isActive{ true };
        bool          isDirty{ false };
    };

    class NodeStorageBlock final
    {
    public:
        std::vector<NodeTopology>         topology;
        std::vector<LocalTransform>       localTransforms;
        std::vector<math::Mat4<zF32>>     worldMatrices;   // 64 байта = 1 кэш-линия
        std::vector<NodeMetadata>         metadata;
        core::BitTreeTracker              dirtyTracker;    // Отслеживает измененные слоты SoA (без топологического порядка)

        void EnsureCapacity(size_t requiredCapacity);
        void MarkDirty(uint32_t nodeIndex);

        /// @brief Разрешение мировых матриц (parent-before-child).
        /// @details Выполняет собственный top-down обход по графу NodeTopology (от корней к листьям),
        /// каскадно наследуя dirty-состояние от родителей. НЕ опирается на порядок GetDirtyIndices(),
        /// так как индексы слотов SoA не гарантируют топологический порядок.
        /// dirtyTracker используется после Resolve для сбора затронутых слотов (отправка в Render Snapshot, обновление AABB).
        void ResolveTransforms();
    };
}
```

### 2.2. Двухбуферный контейнер (`SceneTreeContainer`)

```cpp
namespace zzz::engine
{
    struct NodeHandle {
        uint32_t index{ 0xFFFFFFFF };
        uint32_t generation{ 0 };
        /// @brief Проверка на ненулевой дескриптор (полную проверку с generation выполняет SceneTreeContainer::IsValid).
        [[nodiscard]] bool IsValid() const noexcept { return index != 0xFFFFFFFF; }
    };

    class SceneTreeContainer final
    {
    public:
        SceneTreeContainer();
        ~SceneTreeContainer() = default;

        NodeHandle CreateNode(const std::string& name, GameObject* owner = nullptr);
        void SetParent(NodeHandle child, NodeHandle parent, bool keepWorldTransform = true);
        void DestroySubtree(NodeHandle root, ISpatialStorage* spatialStorage = nullptr);

        void SetNodeOwner(NodeHandle handle, GameObject* owner);
        void SetSpatialHandle(NodeHandle handle, SpatialHandle spHandle);
        [[nodiscard]] SpatialHandle GetSpatialHandle(NodeHandle handle) const noexcept;

        [[nodiscard]] LocalTransform& GetLocalTransform(NodeHandle handle);
        [[nodiscard]] const LocalTransform& GetLocalTransform(NodeHandle handle) const;
        void SetLocalTransform(NodeHandle handle, const LocalTransform& transform);

        void MarkDirty(NodeHandle handle);
        void ResolveTransforms();
        void ApplyHandoverBarrier();

        [[nodiscard]] bool IsValid(NodeHandle handle) const noexcept;
        [[nodiscard]] const math::Mat4<zF32>& GetWorldMatrix(NodeHandle handle) const noexcept;
        [[nodiscard]] GameObject* GetNodeOwner(NodeHandle handle) const noexcept;
        [[nodiscard]] NodeHandle GetParent(NodeHandle handle) const noexcept;

    private:
        NodeStorageBlock      m_PrimaryNodes;   // Front Buffer: статичный снимок для параллельного чтения рендером
        NodeStorageBlock      m_SecondaryNodes; // Back Buffer: рабочий буфер для мутаций текущего кадра
        std::vector<uint32_t> m_FreeIndices;

        // Очереди из раннего проекта удалены в этапе 15.
    };
}
```

#### Архитектурное решение проблемы конкурентности и Torn Reads:
1. **Изоляция Рендера от Логики (Front/Back Buffer):**
   - PrepareFrame строит данные и команды очередного кадрового слота только из опубликованного m_PrimaryNodes и стабильных ресурсов. RenderFrame отправляет команды предыдущего слота, не читая сценовое дерево. Экстракция и Draw ещё завершаются в этапе 23.
   - Поток логики кадра $N$ выполняет симуляцию и расчёт матриц в рабочем буфере `m_SecondaryNodes`.
   - Согласованность Front Buffer требует завершения всех его CPU-читателей до публикации изменений. GPU lifetime регулируется отдельно по fence (генплан, правило 14).
2. **Мутации во время обхода скриптов:**
   - Очереди m_ReparentQueue/m_DeleteQueue удалены в этапе 15 как неиспользуемые. SetParent/DestroySubtree работают с текущим рабочим буфером; двойная буферизация не защищает итератор логики от собственной мутации.
   - В этапе 22 до разрешения удаления/смены родителя из callback определить безопасную точку и способ применения операций. Конкретный механизм пока не выдаётся за реализованный.
3. **Жизненный цикл кадра и сброс dirtyTracker:**
   - В начале каждого кадра логики (перед обновлением скриптов) вызывается `m_SecondaryNodes.dirtyTracker.Prepare(m_SecondaryNodes.metadata.size())`, что за $O(1)$ сбрасывает dirty-биты при отсутствии изменений либо очищает буфер.
   - По мере мутаций скриптов (`SetLocalPosition`, `SetLocalRotation`) взводятся биты `m_SecondaryNodes.dirtyTracker.Set(index)`.
4. **Механика сдачи кадра (`ApplyHandoverBarrier`):**
   - В действующем контейнере нет применения отложенных очередей. Изменения рабочего буфера должны быть завершены до публикации; дополнительный контракт мутаций скриптов относится к 14.
   - Запускается `m_SecondaryNodes.ResolveTransforms()`, пересчитывающий мировые матрицы затронутых поддеревьев.
   - В точке барьера кадра `ApplyHandoverBarrier()`:
     - Синхронизируются изменения из `m_SecondaryNodes` в `m_PrimaryNodes`: обновляются измененные `worldMatrices`, топология и метаданные.
     - `m_PrimaryNodes` фиксируется для следующей экстракции в PrepareFrame; запись возможна только после завершения читателей.
     - `m_SecondaryNodes` готов к следующему кадру симуляции $N+1$.

### 2.3. Алгоритм итеративного удаления поддерева (`SceneTreeContainer::DestroySubtree`)
Старый рекурсивный вызов C++ стека `while (obj->GetChildCount() > 0) DestroyObject(obj->GetChild(0));` **полностью удаляется**.
Вместо рекурсии уничтожение выполняет метод `SceneTreeContainer::DestroySubtree(NodeHandle rootHandle)` **строго итеративно** (без вызовов функций на стеке CPU) и работает **исключительно с рабочим буфером `m_SecondaryNodes`**:
1. **Сбор индексов поддерева (BFS/DFS без рекурсии):**
   - Используется плоский локальный буфер индексов `std::vector<uint32_t> toDeleteIndices`;
   - Буфер резервирует память (начиная с `rootHandle.index`);
   - Указатель чтения `size_t readIdx = 0`: пока `readIdx < toDeleteIndices.size()`:
     - Берется текущий индекс `uint32_t curIdx = toDeleteIndices[readIdx++]`;
     - Обход всех непосредственных детей: `uint32_t childIdx = m_SecondaryNodes.topology[curIdx].firstChildIndex`;
     - Пока `childIdx != 0xFFFFFFFF`:
       - `toDeleteIndices.push_back(childIdx)`;
       - `childIdx = m_SecondaryNodes.topology[childIdx].nextSiblingIndex`;
2. **Отвязка корня поддерева от родительской топологии:**
   - Для `rootHandle.index` извлекается `parentIndex = m_SecondaryNodes.topology[rootIdx].parentIndex`;
   - Если `parentIndex != 0xFFFFFFFF`:
     - Перелинковываются сиблинги: `prevSiblingIndex` и `nextSiblingIndex` связываются напрямую;
     - Если `rootIdx` был `firstChildIndex` родителя, `parent.firstChildIndex` переставляется на `root.nextSiblingIndex`;
   - Корень изолирован.
3. **Линейное уничтожение полезной нагрузки и структур данных:**
   - В цикле по всем `idx` из `toDeleteIndices` (от листьев к корню или в порядке сбора):
     - **ObjectDomain:** Если `metadata[idx].payloadType == SceneNodeType::GameObject` и `metadata[idx].gameObject != nullptr`:
       - Оповещается домен об уничтожении объекта (удаление скриптов, компонентов);
       - Указатель зануляется;
     - **ISpatialStorage:** Если `metadata[idx].spatialHandle != 0`:
       - Вызывается `spatialStorage->Remove(metadata[idx].spatialHandle)`;
       - `metadata[idx].spatialHandle = 0`;
     - **Сброс состояния:**
       - Флаг `m_SecondaryNodes.metadata[idx].isDirty = false`;
       - Сам бит в `dirtyTracker` сбрасывать точечно не требуется (в `BitTreeTracker` нет побитового сброса) — трекер сбрасывается пакетно через `Prepare()` в начале кадра, а слот признаётся мёртвым через `isAlive == false`.
     - **Освобождение слота:**
       - `metadata[idx].isAlive = false`;
       - `metadata[idx].generation++` (защита от ABA-проблемы для внешних дескрипторов `NodeHandle`);
       - `m_FreeIndices.push_back(idx)`;
       - `topology[idx]` сбрасывается в `0xFFFFFFFF`.

---

### 2.4. Скрытый интерфейс `ISceneTreeAccessor`, полное удаление `Transform` и рефакторинг `GameObject`
- **Единый источник истины (Правило 6):**
  - Все пространственные данные (`LocalTransform`, `worldMatrices`), топология, а также свойства узла (`name`, `isActive`, `spatialHandle`) хранятся **строго в одном месте — в SoA-блоках `SceneTreeContainer` (`NodeMetadata`)**.
  - В `GameObject` поля `m_Transform`, `m_SpatialHandle`, `m_IsActive`, `m_Name` **полностью удаляются**.
  - `GameObject` становится легковесным 32-байтовым фасадом: хранит `ISceneTreeAccessor* m_SceneTree`, `NodeHandle m_NodeHandle`, `Guid m_Guid`, а также контейнеры игрового поведения (скрипты, компоненты).
  - Все методы `GameObject` (`GetName`, `SetName`, `IsActive`, `SetActive`, `GetSpatialHandle`, `SetSpatialHandle`, `GetLocalPosition`, `SetLocalPosition` и т.д.) делегируются в `m_SceneTree` за $O(1)$.

- Вводится абстрактный скрытый интерфейс `ISceneTreeAccessor`:
  ```cpp
  class ISceneTreeAccessor
  {
  public:
      virtual ~ISceneTreeAccessor() = default;

      // Пространственные координаты
      virtual void SetLocalPosition(NodeHandle handle, const ::zzz::math::Vec3<zF32>& pos) = 0;
      [[nodiscard]] virtual const ::zzz::math::Vec3<zF32>& GetLocalPosition(NodeHandle handle) const = 0;
      virtual void SetLocalRotation(NodeHandle handle, const ::zzz::math::Quat<zF32>& rot) = 0;
      [[nodiscard]] virtual const ::zzz::math::Quat<zF32>& GetLocalRotation(NodeHandle handle) const = 0;
      virtual void SetLocalScale(NodeHandle handle, const ::zzz::math::Vec3<zF32>& scale) = 0;
      [[nodiscard]] virtual const ::zzz::math::Vec3<zF32>& GetLocalScale(NodeHandle handle) const = 0;
      [[nodiscard]] virtual const ::zzz::math::Mat4<zF32>& GetWorldMatrix(NodeHandle handle) const = 0;

      // Топология и владение
      virtual void SetParent(NodeHandle child, NodeHandle parent, bool keepWorldTransform = true) = 0;
      [[nodiscard]] virtual NodeHandle GetParent(NodeHandle handle) const = 0;
      [[nodiscard]] virtual GameObject* GetNodeOwner(NodeHandle handle) const = 0;
      virtual void MarkDirty(NodeHandle handle) = 0;

      // Свойства узла (делегирование из GameObject)
      virtual void SetActive(NodeHandle handle, bool active) = 0;
      [[nodiscard]] virtual bool IsActive(NodeHandle handle) const = 0;
      virtual void SetName(NodeHandle handle, std::string name) = 0;
      [[nodiscard]] virtual const std::string& GetName(NodeHandle handle) const = 0;
      virtual void SetSpatialHandle(NodeHandle handle, SpatialHandle spHandle) = 0;
      [[nodiscard]] virtual SpatialHandle GetSpatialHandle(NodeHandle handle) const noexcept = 0;
  };
  ```

- `SceneTreeContainer` реализует `ISceneTreeAccessor`.
- `GameObject` фасад:
  - `void SetLocalPosition(const ::zzz::math::Vec3<zF32>& pos) { m_SceneTree->SetLocalPosition(m_NodeHandle, pos); }`
  - `[[nodiscard]] const ::zzz::math::Vec3<zF32>& GetLocalPosition() const { return m_SceneTree->GetLocalPosition(m_NodeHandle); }`
  - `[[nodiscard]] const ::zzz::math::Mat4<zF32>& GetWorldMatrix() const { return m_SceneTree->GetWorldMatrix(m_NodeHandle); }`
  - `void SetActive(bool active) { m_SceneTree->SetActive(m_NodeHandle, active); }`
  - `[[nodiscard]] bool IsActive() const { return m_SceneTree ? m_SceneTree->IsActive(m_NodeHandle) : false; }`
  - `const std::string& GetName() const { return m_SceneTree->GetName(m_NodeHandle); }`
  - `SpatialHandle GetSpatialHandle() const noexcept { return m_SceneTree ? m_SceneTree->GetSpatialHandle(m_NodeHandle) : 0; }`
  - `GameObject* GetParent() const noexcept { return m_SceneTree ? m_SceneTree->GetNodeOwner(m_SceneTree->GetParent(m_NodeHandle)) : nullptr; }`
  - `void SetParent(GameObject* newParent, bool keepWorldTransform = true) noexcept;`
  - Все устаревшие рекурсивные методы `GetChildren()`, `GetChildCount()`, `GetChild()` удалены.
  - Методы `GetTransform()` удалены — доступ к пространственным координатам происходит через сам `GameObject`.

---

## 3. Чек-лист Definition of Done (DoD)

- [x] Создать скрытый интерфейс `src/engine/scene/storage/ISceneTreeAccessor.h`
- [x] Реализовать `src/engine/scene/storage/NodeStorageBlock.h` и `.cpp`
- [x] Реализовать `src/engine/scene/storage/SceneTreeContainer.h` и `.cpp` (реализует `ISceneTreeAccessor`)
- [x] Удалить файлы `src/engine/scene/gameobject/Transform.h` и `Transform.cpp`
- [x] Рефакторить `src/engine/scene/gameobject/GameObject.h` и `.cpp` (прямой API пространственных параметров через `ISceneTreeAccessor` по `NodeHandle`)
- [x] Обновить вызовы `go->GetTransform().SetLocal...` на `go->SetLocal...` в `Layer3D.cpp` и тестах
- [x] Обновить регистрацию файлов в `src/engine/CMakeLists.txt`
- [x] Реализовать модульные тесты в `src/qa/tests/engine/SceneTreeContainerTests.cpp`:
  - Создание, удаление узлов, валидация `NodeHandle` и защита от ABA-проблемы через `generation`
  - Иерархия: `SetParent`, перелинковка `firstChild`/`siblings`, отсоединение от родителя
  - Итеративное удаление поддерева `DestroySubtree`
  - Корректность ResolveTransforms: $M_{world} = M_{local} \times M_{parent\_world}$ (row-vector)
  - Очереди из первоначального перечня удалены в этапе 15; это не заявка на существующее покрытие deferred mutations
- [x] Реализовать тесты производительности в `src/qa/benchmark/common/templates/SceneTreeContainerBench.cpp`:
  - `ResolveTransforms` на плоской сцене (10 000 / 100 000 объектов)
  - `ResolveTransforms` на глубокой иерархии (пирамида / дерево глубиной 5–10 уровней)
  - Пакетное создание узлов `CreateNode`
  - Мутации `SetLocalPosition` / `MarkDirty`
  - Итеративное удаление `DestroySubtree`
- [x] Зарегистрировать флаг теста `Z_TEST_ENGINE_SCENE_TREE_CONTAINER` в `src/qa/tests/TestsConfig.h`
- [x] Проверить сборку под MSVC x64 + Ninja (0 ошибок, 0 предупреждений)
- [ ] Прогнать модульные тесты `SceneTreeContainerTest.*`
- [x] Прогнать бенчмарки `--benchmark_filter=SceneTreeContainer`
