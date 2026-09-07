# Этап 11.3: Структура узлов сцены, SoA-хранилище, ISceneTreeAccessor, удаление Transform и рефакторинг GameObject (`SceneNode`, `SceneTreeContainer`, `GameObject`)

## 1. Контекст и цели подэтапа
- **Номер подпункта:** **Пункт 11.3** (часть Этапа 11: Структура хранения GameObject в сцене).
- **Цель:** Реализовать плоское высокопроизводительное SoA-хранилище сцены и превратить `GameObject` в легковесный фасад:
  1. Единый источник истины: компактные структуры `NodeTopology`, `LocalTransform`, `NodeMetadata` и блок `NodeStorageBlock` (SoA-векторы по Правилу 13 DOD).
  2. Двухбуферный контейнер `SceneTreeContainer`: активный буфер `m_PrimaryNodes` и теневой `m_SecondaryNodes`, спавн через `m_StagingSpawn`, очереди мутаций `m_ReparentQueue`/`m_DeleteQueue`, безопасное переиспользование слотов `m_FreeIndices` с проверкой поколений `generation`.
  3. **Полная ликвидация `class Transform`** (`Transform.h` / `Transform.cpp` удаляются): избавление от лишней ООП-прослойки и проблем выравнивания (C4324).
  4. Введение скрытого интерфейса `ISceneTreeAccessor`: `GameObject` хранит `NodeHandle` и указатель на интерфейс `ISceneTreeAccessor*`. Методы `SetLocalPosition`, `GetWorldMatrix` вызываются у `GameObject` напрямую и делегируются в плоский массив `SceneTreeContainer` за $O(1)$.
  5. Итеративный обход поддерева при удалении (`DestroySubtree`) без рекурсии на стеке CPU.
- **Статус:** ⏳ В процессе разработки.
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
        core::BitTreeTracker              dirtyTracker;

        void EnsureCapacity(size_t requiredCapacity);
        void MarkDirty(uint32_t nodeIndex);
        void ResolveTransforms();
        void ClearDirty() { dirtyTracker.Clear(); }
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

        // Очереди отложенных кадровых мутаций (Deferred Command Buffers)
        // Исключают реентрантность и порчу топологии посреди итерации логики скриптов:
        std::vector<uint32_t> m_DeleteQueue;
        std::vector<std::pair<uint32_t, uint32_t>> m_ReparentQueue;
    };
}
```

#### Архитектурное решение проблемы конкурентности и Torn Reads:
1. **Изоляция Рендера от Логики (Front/Back Buffer):**
   - Поток рендера кадра $N-1$ (`PrepareFrame` / построение снимка) читает **только зафиксированный буфер `m_PrimaryNodes`** (Front Buffer). В этот момент никакие мутации в него не пишутся.
   - Поток логики кадра $N$ выполняет симуляцию и расчёт матриц в рабочем буфере `m_SecondaryNodes`.
   - Это гарантирует **абсолютную невозможность Torn Reads** (разорванного чтения топологии или промежуточных матриц рендером) без применения мьютексов.
2. **Изоляция Логики от Самой Себя (Deferred Mutation Queues):**
   - Мутация топологии — это запись в 3–5 связанных полей (`parent`, `firstChild`, `next/prevSibling`), которая не может быть атомарной на уровне CPU.
   - Если скрипт внутри цикла обновления вызывает `SetParent` или `Destroy`, синхронная перелинковка привела бы к реентрантности и поломке итератора обхода.
   - Поэтому во время выполнения кадра вызовы `SetParent` и `Destroy` **только складируют команды в `m_ReparentQueue` и `m_DeleteQueue` за $O(1)$**.
   - Применение очередей к топологии происходит централизованно и пакетно перед расчётом матриц в точке Handover Barrier.

### 2.3. Алгоритм итеративного удаления поддерева (`SceneTreeContainer::DestroySubtree`)
Старый рекурсивный вызов C++ стека `while (obj->GetChildCount() > 0) DestroyObject(obj->GetChild(0));` **полностью удаляется**.
Вместо рекурсии уничтожение выполняет метод `SceneTreeContainer::DestroySubtree(NodeHandle rootHandle)` **строго итеративно** (без вызовов функций на стеке CPU):
1. **Сбор индексов поддерева (BFS/DFS без рекурсии):**
   - Используется плоский локальный буфер индексов `std::vector<uint32_t> toDeleteIndices`;
   - Буфер резервирует память (начиная с `rootHandle.index`);
   - Указатель чтения `size_t readIdx = 0`: пока `readIdx < toDeleteIndices.size()`:
     - Берется текущий индекс `uint32_t curIdx = toDeleteIndices[readIdx++]`;
     - Обход всех непосредственных детей: `uint32_t childIdx = m_PrimaryNodes.topology[curIdx].firstChildIndex`;
     - Пока `childIdx != 0xFFFFFFFF`:
       - `toDeleteIndices.push_back(childIdx)`;
       - `childIdx = m_PrimaryNodes.topology[childIdx].nextSiblingIndex`;
2. **Отвязка корня поддерева от родительской топологии:**
   - Для `rootHandle.index` извлекается `parentIndex = m_PrimaryNodes.topology[rootIdx].parentIndex`;
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
     - **Сброс битов изменений:**
       - `m_PrimaryNodes.dirtyTracker.Reset(idx)`;
       - `m_PrimaryNodes.metadata[idx].isDirty = false`;
     - **Освобождение слота:**
       - `metadata[idx].isAlive = false`;
       - `metadata[idx].generation++` (защита от ABA-проблемы для внешних дескрипторов);
       - `m_FreeIndices.push_back(idx)`;
       - `topology[idx]` сбрасывается в `0xFFFFFFFF`.

---

### 2.4. Скрытый интерфейс `ISceneTreeAccessor`, полное удаление `Transform` и рефакторинг `GameObject`
- **Полная ликвидация `class Transform`**: удаляются `Transform.h` и `Transform.cpp`.
- Вводится абстрактный скрытый интерфейс `ISceneTreeAccessor`:
  ```cpp
  class ISceneTreeAccessor
  {
  public:
      virtual ~ISceneTreeAccessor() = default;
      virtual void SetLocalPosition(NodeHandle handle, const ::zzz::math::Vec3<zF32>& pos) = 0;
      [[nodiscard]] virtual const ::zzz::math::Vec3<zF32>& GetLocalPosition(NodeHandle handle) const = 0;
      virtual void SetLocalRotation(NodeHandle handle, const ::zzz::math::Quat<zF32>& rot) = 0;
      [[nodiscard]] virtual const ::zzz::math::Quat<zF32>& GetLocalRotation(NodeHandle handle) const = 0;
      virtual void SetLocalScale(NodeHandle handle, const ::zzz::math::Vec3<zF32>& scale) = 0;
      [[nodiscard]] virtual const ::zzz::math::Vec3<zF32>& GetLocalScale(NodeHandle handle) const = 0;
      [[nodiscard]] virtual const ::zzz::math::Mat4<zF32>& GetWorldMatrix(NodeHandle handle) const = 0;
      virtual void SetParent(NodeHandle child, NodeHandle parent, bool keepWorldTransform = true) = 0;
      [[nodiscard]] virtual NodeHandle GetParent(NodeHandle handle) const = 0;
      [[nodiscard]] virtual GameObject* GetNodeOwner(NodeHandle handle) const = 0;
      virtual void MarkDirty(NodeHandle handle) = 0;
  };
  ```
- `SceneTreeContainer` реализует `ISceneTreeAccessor`.
- `GameObject` хранит `NodeHandle m_NodeHandle` и `ISceneTreeAccessor* m_SceneTree`:
  - `void SetLocalPosition(const ::zzz::math::Vec3<zF32>& pos) { m_SceneTree->SetLocalPosition(m_NodeHandle, pos); }`
  - `[[nodiscard]] const ::zzz::math::Vec3<zF32>& GetLocalPosition() const { return m_SceneTree->GetLocalPosition(m_NodeHandle); }`
  - `[[nodiscard]] const ::zzz::math::Mat4<zF32>& GetWorldMatrix() const { return m_SceneTree->GetWorldMatrix(m_NodeHandle); }`
  - `GameObject* GetParent() const noexcept { return m_SceneTree ? m_SceneTree->GetNodeOwner(m_SceneTree->GetParent(m_NodeHandle)) : nullptr; }`
  - `void SetParent(GameObject* newParent, bool keepWorldTransform = true) noexcept;`
  - Все устаревшие рекурсивные методы `GetChildren()`, `GetChildCount()`, `GetChild()` удалены.
  - Методы `GetTransform()` удалены — доступ к пространственным координатам происходит через сам `GameObject`.

---

## 3. Чек-лист Definition of Done (DoD)

- [ ] Создать скрытый интерфейс `src/engine/scene/storage/ISceneTreeAccessor.h`
- [ ] Реализовать `src/engine/scene/storage/NodeStorageBlock.h` и `.cpp`
- [ ] Реализовать `src/engine/scene/storage/SceneTreeContainer.h` и `.cpp` (реализует `ISceneTreeAccessor`)
- [ ] Удалить файлы `src/engine/scene/gameobject/Transform.h` и `Transform.cpp`
- [ ] Рефакторить `src/engine/scene/gameobject/GameObject.h` и `.cpp` (прямой API пространственных параметров через `ISceneTreeAccessor` по `NodeHandle`)
- [ ] Обновить вызовы `go->GetTransform().SetLocal...` на `go->SetLocal...` в `Layer3D.cpp` и тестах
- [ ] Обновить регистрацию файлов в `src/engine/CMakeLists.txt`
- [ ] Проверить сборку под MSVC x64 + Ninja (0 ошибок)
