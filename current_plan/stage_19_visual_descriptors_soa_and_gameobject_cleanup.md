# Этап 19: Двухуровневый плоский массив визуалов в NodeStorage и очистка GameObject

## 1. Контекст, статус и цель

- **Номер этапа:** 19 (Уровень 2: GAPI-ресурсы, содержимое куба и сквозной рендер).
- **Статус:** ⏳ Не начато
- **Цель:** 
  1. Избавиться от громоздкой полиморфной структуры `VisualPayload` (`std::variant`, динамические векторы сабмешей `std::vector<Guid>`) в `GameObject` и доменах.
  2. Вынести визуальные дескрипторы в двухуровневый плоский массив внутри `NodeStorage` (`VisualRange` в `m_NodeVisuals` по индексу `NodeHandle` + непрерывный `m_Draws`), адресуемый за $O(1)$ без аллокаций в куче на узел и с нативной поддержкой как одиночных мешей/спрайтов, так и MultiMesh.
  3. Очистить `GameObject` от визуальных данных, оставив его компактным объектом идентификации и логики компонентов/скриптов.
  4. Объединить флаги состояния (`Active` и `Visible`) в плоский байтовый массив `m_Flags` (`eNodeFlags`) в `NodeStorage` с поддержкой constexpr битовых операторов (`|`, `&`, `~`). Флаг `Visible` по умолчанию включён (`true`) для всех узлов, так как логическая видимость ортогональна наличию геометрии.
  5. Централизовать наполнение визуалов в `NodeStorage::InitializeFromObjects`: массивы `m_NodeVisuals` и `m_Draws` заполняются напрямую из `GameObjectData` сцены для ВСЕХ узлов (и `Object`, и `Entity`), гарантируя целостность spatial-регистрации по `storage.HasMesh(handle)`. Вектор `m_Draws` аллоцируется строго через предварительный `reserve` без повторных реаллокаций.
  6. Устранить зависимость порядка инициализации доменов от `NodeStorage`: передавать `NodeStorage&` непосредственно в метод `CreateObject` домена.
  7. Полностью удалить устаревшие файлы `src/engine/scene/visual/VisualTypes.h` и `src/core/enums/eVisualType.h` вместе с записями в `CMakeLists.txt`.
- **Зависимости:** этап 18 гарантирует стабильные `NodeHandle`, плоский `NodeStorage` и `DefaultSpatialStorage`, содержащий только узлы с мешем.
- **Следующий потребитель:** этап 20 (GPU-буферы, загрузка в GPU и GPU-меш); этап 23 (сквозной рендер), где рендер-поток получает список видимых `NodeHandle` от spatial-хранилища и за $O(1)$ забирает мировую матрицу (`m_WorldMatrices[handle]`) и непрерывный диапазон дескрипторов отрисовки (`GetDraws(handle)`).

---

## 2. Архитектурные решения и согласованные контракты

### 2.1. Единое хранение флагов: `Active` и `Visible` в `m_Flags` (`eNodeFlags`)

Флаги состояния узла хранятся в массиве `m_Flags` внутри `NodeStorage`. Для поддержки безопасных операций со scoped enum реализуются constexpr битовые операторы:
```cpp
enum class eNodeFlags : zU8
{
    None    = 0,
    Active  = 1 << 0,  // Активность (симуляция, скрипты OnUpdate, физика)
    Visible = 1 << 1   // Видимость для рендера (отрисовка)
};

[[nodiscard]] constexpr eNodeFlags operator|(eNodeFlags lhs, eNodeFlags rhs) noexcept
{
    return static_cast<eNodeFlags>(static_cast<zU8>(lhs) | static_cast<zU8>(rhs));
}

[[nodiscard]] constexpr eNodeFlags operator&(eNodeFlags lhs, eNodeFlags rhs) noexcept
{
    return static_cast<eNodeFlags>(static_cast<zU8>(lhs) & static_cast<zU8>(rhs));
}

[[nodiscard]] constexpr eNodeFlags operator~(eNodeFlags flag) noexcept
{
    return static_cast<eNodeFlags>(~static_cast<zU8>(flag));
}

inline eNodeFlags& operator|=(eNodeFlags& lhs, eNodeFlags rhs) noexcept
{
    lhs = lhs | rhs;
    return lhs;
}

inline eNodeFlags& operator&=(eNodeFlags& lhs, eNodeFlags rhs) noexcept
{
    lhs = lhs & rhs;
    return lhs;
}
```
1. **Разделение семантики**:
   - `Active` — управляет участием узла в симуляции и вызовами логики/скриптов.
   - `Visible` — управляет исключительно отрисовкой в кадровом конвейере (невидимые триггеры, коллизии, стелс). По умолчанию при создании узла флаг `Visible` **всегда выставлен**, независимо от наличия геометрии:
     ```cpp
     m_Flags[i] = (obj.IsActive() ? eNodeFlags::Active : eNodeFlags::None) | eNodeFlags::Visible;
     ```
2. **Компактность и локальность по кэшу**:
   - `m_Flags` занимает ровно 1 байт (`zU8`) на узел.
   - Флаги множества узлов непрерывно упакованы в памяти.
   - Изменение активности и видимости выполняется битовой маской без инвалидации стабильного `NodeHandle`.
3. **Проксирование через `GameObject`**:
   - `GameObject::IsActive()` / `SetActive(bool)` делегируют вызов в `m_NodeStorage->IsActive(m_NodeHandle)` / `SetActive(m_NodeHandle, bool)`.
   - `GameObject::IsVisible()` / `SetVisible(bool)` делегируют вызов в `m_NodeStorage->IsVisible(m_NodeHandle)` / `SetVisible(m_NodeHandle, bool)`.

### 2.2. Двухуровневый плоский массив визуалов (`VisualRange` и `DrawDescriptor`)

В `NodeTypes.h` объявляются плоские структуры:

```cpp
/**
 * @struct VisualRange
 * @brief Диапазон дескрипторов отрисовки узла в общем плоском массиве m_Draws.
 */
struct VisualRange
{
    zU32 begin{ 0 };
    zU32 count{ 0 };

    [[nodiscard]] bool HasMesh() const noexcept { return count > 0; }
};

/**
 * @struct DrawDescriptor
 * @brief Непосредственный дескриптор одной отрисовки (связка геометрии и материала).
 */
struct DrawDescriptor
{
    Guid meshGuid;
    Guid materialGuid;

    [[nodiscard]] bool HasMesh() const noexcept { return meshGuid.IsValid(); }
    [[nodiscard]] bool HasMaterial() const noexcept { return materialGuid.IsValid(); }
};
```

1. **Модель памяти в `NodeStorage`**:
   - `std::vector<VisualRange> m_NodeVisuals;` — параллельный массив размером строго `m_Count` (число узлов), индексируемый по `NodeHandle`.
   - `std::vector<DrawDescriptor> m_Draws;` — общий непрерывный массив дескрипторов отрисовки для всего слоя.
2. **Предварительный подсчёт и `reserve` для `m_Draws`**:
   Перед заполнением дескрипторов выполняется быстрый проход по объектам для вычисления общего числа вызовов отрисовки и резервирования памяти без реаллокаций:
   ```cpp
   size_t totalDraws = 0;
   for (const auto& obj : objects)
   {
       if (obj.IsMultiMesh())
       {
           totalDraws += obj.GetSubmeshGuids().size();
       }
       else if (obj.GetMeshGuid().IsValid())
       {
           totalDraws += 1;
       }
   }
   m_Draws.reserve(totalDraws);
   ```
3. **Обработка геометрии и контракт MultiMesh**:
   - **Узел без меша (пустышка, кость, триггер):**
     `m_NodeVisuals[i] = VisualRange{ 0, 0 };`
   - **SingleMesh3D / 2D Sprite:**
     `begin = static_cast<zU32>(m_Draws.size());`
     `m_Draws.push_back(DrawDescriptor{ obj.GetMeshGuid(), obj.GetMaterialGuid() });`
     `m_NodeVisuals[i] = VisualRange{ begin, 1 };`
   - **MultiMesh3D и правило fallback-материала:**
     В формате `GameObjectData` размеры `submeshes` и `materials` могут различаться. 
     **Явно закреплённое правило:** если `s >= materials.size()`, для подсетки устанавливается пустой `Guid()`. На этапе загрузки/рендера пустой `materialGuid` связывается с дефолтным fallback-материалом движка.
     ```cpp
     const zU32 begin = static_cast<zU32>(m_Draws.size());
     const auto& submeshes = obj.GetSubmeshGuids();
     const auto& materials = obj.GetMaterialGuids();
     for (size_t s = 0; s < submeshes.size(); ++s)
     {
         Guid matGuid = (s < materials.size()) ? materials[s] : Guid();
         m_Draws.push_back(DrawDescriptor{ submeshes[s], matGuid });
     }
     m_NodeVisuals[i] = VisualRange{ begin, static_cast<zU32>(submeshes.size()) };
     ```

### 2.3. Контракт доступа к визуалам в `NodeStorage`

1. `NodeStorage` инкапсулирует `m_NodeVisuals` и `m_Draws`, инициализируя их в `InitializeFromObjects(objects)`.
2. **Иммутабельность в этапе 19**:
   - Во избежание рассинхронизации со статическим spatial-индексом мутирующие методы (`SetMeshGuid`, `SetMaterialGuid`, `SetVisual`) в рантайме отсутствуют.
3. Публичные методы доступа в `NodeStorage`:
   - `[[nodiscard]] VisualRange GetVisualRange(NodeHandle handle) const;`
   - `[[nodiscard]] std::span<const DrawDescriptor> GetDraws(NodeHandle handle) const;`
   - `[[nodiscard]] bool HasMesh(NodeHandle handle) const noexcept;` (проверяет `m_NodeVisuals[handle].count > 0`)
   - `[[nodiscard]] bool IsVisible(NodeHandle handle) const;`
   - `void SetVisible(NodeHandle handle, bool visible);`
   - `[[nodiscard]] std::span<const VisualRange> GetNodeVisuals() const noexcept;`
   - `[[nodiscard]] std::span<const DrawDescriptor> GetAllDraws() const noexcept;`

### 2.4. Полная очистка `GameObject` от визуалов

1. Из `GameObject` полностью удаляются:
   - Поле `VisualPayload m_Visual;`
   - Локальные копии мешей, материалов и векторов сабмешей.
   - Зависимости от `VisualTypes.h`.
2. `GameObject` содержит исключительно:
   - Идентичность: `Guid m_Guid`, `std::string m_Name`.
   - Связь со сценой: `NodeStorage* m_NodeStorage`, `NodeHandle m_NodeHandle`.
   - Компоненты/скрипты: существующий контейнер `std::vector<std::shared_ptr<Script>> m_Scripts` (сохраняем `std::shared_ptr` из-за `std::enable_shared_from_this<BaseScript>` и `EventBus`).
3. Конструктор `GameObject`:
   ```cpp
   GameObject::GameObject(
       const Guid& guid,
       std::string name,
       NodeStorage& storage,
       NodeHandle nodeHandle)
       : m_Guid(guid)
       , m_Name(std::move(name))
       , m_NodeStorage(&storage)
       , m_NodeHandle(nodeHandle)
       , m_Scripts()
   {
       ensure(m_NodeHandle != kInvalidNodeHandle, "GameObject: передан невалидный NodeHandle");
   }
   ```
4. Константный доступ к визуалам из `GameObject`:
   - `[[nodiscard]] VisualRange GetVisualRange() const { return m_NodeStorage->GetVisualRange(m_NodeHandle); }`
   - `[[nodiscard]] std::span<const DrawDescriptor> GetDraws() const { return m_NodeStorage->GetDraws(m_NodeHandle); }`
   - `[[nodiscard]] bool HasMesh() const { return m_NodeStorage->HasMesh(m_NodeHandle); }`

### 2.5. Развязка времени жизни и контракт доменов (`IObjectDomain`, `ObjectDomain2D`, `ObjectDomain3D`)

1. В `Scene.cpp` домены создаются **до** создания слоя и `NodeStorage`.
   Поэтому `IObjectDomain` **не сохраняет** указатель на `NodeStorage` в своём конструкторе:
   ```cpp
   IObjectDomain() = default;
   virtual ~IObjectDomain() = default;
   ```
2. `NodeStorage&` передаётся непосредственно в вызовы создания объектов:
   ```cpp
   virtual ObjectRegistration CreateObject(
       NodeStorage& storage,
       NodeHandle nodeHandle,
       const GameObjectData& objData) = 0;
   ```
3. Метод `RegisterObject` в `IObjectDomain`:
   ```cpp
   ObjectRegistration RegisterObject(
       NodeStorage& storage,
       NodeHandle nodeHandle,
       const Guid& guid,
       std::string name);
   ```
   Внутри метода создаётся:
   `std::make_unique<GameObject>(guid, std::move(name), storage, nodeHandle)`.
4. В `GameLayer::Populate`:
   ```cpp
   m_ObjectDomain->CreateObject(m_NodeStorage, nodeHandle, objData);
   ```
5. Из доменов полностью удаляются:
   - `VisualPayload` и метод `ValidateVisual(const VisualPayload&)`.
   - Зависимости от полиморфных структур визуала.

### 2.6. Полное удаление `VisualTypes.h` и `eVisualType.h`

1. Заголовочный файл `src/engine/scene/visual/VisualTypes.h` удаляется из проекта и из `src/engine/CMakeLists.txt`.
2. Заголовочный файл `src/core/enums/eVisualType.h` удаляется из проекта и из `src/core/CMakeLists.txt`.
3. Структуры `VisualPayload`, `SimpleMeshData`, `MultiMesh3DData`, `SpriteData` и перечисление `eVisualType` полностью искореняются из кодовой базы ядра движка.
4. Все модули переключаются на `NodeTypes.h`.

---

## 3. Что НЕ входит в этап (Non-goals)

1. **Атласные UV-координаты и цветовой тинт 2D-спрайтов**: поля `uv[4]` и `tint` из старого `SpriteData` выносятся из узла сцены и откладываются до этапа 21 («Базовые Shader и Material»), где войдут в параметры `MaterialData` / константные буферы материала.
2. **Динамическая смена меша/материала в рантайме**: публичные сеттеры геометрии в рантайме не вводятся до реализации динамического обновления spatial-индекса (YAGNI, Правило 16).
3. **Указатель на слой внутри GameObject (`Layer*`)**: исключён из скоупа, так как создание доменов в `Scene` предшествует слоям и класс `Layer` отсутствует.
4. **Переработка компонентной системы скриптов**: вызовы `ScriptFactory` и `ResourceManager` в `GameObject::Initialize` сохраняются как рабочий каркас; полноценный жизненный цикл скриптов реализуется в этапе 22.
5. **GPU-буферы и шейдеры**: создание реальных `GPUBuffer`, staging, fences и GAPI-ресурсов относится к этапам 20–21.
6. **Написание и запуск тестов**: категорически запрещено правилами проекта (Блок 2 GEMINI.md). Единственный критерий проверки — компиляция через `run_build.bat`.

---

## 4. Порядок реализации

1. **Базовые типы (`NodeTypes.h`):**
   - Добавить флаг `Visible = 1 << 1` в `eNodeFlags`.
   - Реализовать constexpr битовые операторы для `eNodeFlags` (`|`, `&`, `~`, `|=`, `&=`).
   - Объявить структуры `VisualRange` (`begin`, `count`) и `DrawDescriptor` (`meshGuid`, `materialGuid`).
2. **Хранилище узлов (`NodeStorage.h / .cpp`):**
   - Добавить параллельные массивы `std::vector<VisualRange> m_NodeVisuals` и `std::vector<DrawDescriptor> m_Draws`.
   - В `InitializeFromObjects`:
     - Выставлять `m_Flags[i] = (obj.IsActive() ? eNodeFlags::Active : eNodeFlags::None) | eNodeFlags::Visible`.
     - Выполнять предварительный подсчёт `totalDraws` и вызывать `m_Draws.reserve(totalDraws)`.
     - Заполнять `m_NodeVisuals` и `m_Draws` с поддержкой как одиночных мешей/спрайтов, так и MultiMesh (с fallback `Guid()` для недостающих материалов).
   - Реализовать методы доступа: `GetVisualRange`, `GetDraws`, `HasMesh`, `IsVisible`, `SetVisible`, `GetNodeVisuals()`, `GetAllDraws()`.
3. **Удаление устаревших типов (`VisualTypes.h`, `eVisualType.h`):**
   - Удалить `src/engine/scene/visual/VisualTypes.h` и очистить `src/engine/CMakeLists.txt`.
   - Удалить `src/core/enums/eVisualType.h` и очистить `src/core/CMakeLists.txt`.
4. **Очистка и адаптация `GameObject` (`GameObject.h / .cpp`):**
   - Удалить поле `m_Visual` и методы работы с `VisualPayload`.
   - Конструктор принимает `(guid, name, NodeStorage&, NodeHandle)` со строгой проверкой `ensure` и списком инициализации полей (Правило 8.1).
   - Хранить скрипты как `std::vector<std::shared_ptr<Script>> m_Scripts`.
   - Реализовать методы `GetVisualRange()`, `GetDraws()`, `HasMesh()`, делегирующие в `m_NodeStorage`.
5. **Упрощение доменов (`IObjectDomain`, `ObjectDomain2D`, `ObjectDomain3D`):**
   - Конструктор `IObjectDomain()` остаётся дефолтным (без сохранения `NodeStorage*`).
   - Сигнатура: `CreateObject(NodeStorage& storage, NodeHandle nodeHandle, const GameObjectData& objData)`.
   - Метод `RegisterObject(storage, nodeHandle, guid, name)` создаёт `GameObject`.
   - Удалить `ValidateVisual` и зависимости от `VisualPayload`.
6. **Адаптация слоя (`GameLayer.h / .cpp`):**
   - В `Populate` вызывать `m_ObjectDomain->CreateObject(m_NodeStorage, nodeHandle, objData)`.
7. **Верификация сборки:**
   - Выполнить `run_build.bat`.
   - Убедиться в 0 ошибок и 0 предупреждений компилятора.

---

## 5. Критерии приёмки (Definition of Done)

1. **Единое битовое хранение флагов (`Active` и `Visible`):**
   - `Active` и `Visible` определены в `eNodeFlags` и хранятся в массиве `m_Flags` в `NodeStorage`.
   - Реализованы constexpr битовые операторы для `eNodeFlags`.
   - Флаг `Visible` по умолчанию выставлен для всех создаваемых узлов.
2. **Двухуровневый плоский массив визуалов:**
   - `VisualRange` в `m_NodeVisuals` индексируется за $O(1)$ по `NodeHandle`.
   - Массив `m_Draws` непрерывен, память под него резервируется перед наполнением (`reserve`).
   - Поддерживаются SingleMesh, MultiMesh (с fallback для недостающих материалов) и узлы без визуала.
   - Нулевое число динамических аллокаций в куче на отдельный узел.
3. **Корректная развязка времени жизни:**
   - `IObjectDomain` не хранит ссылку на `NodeStorage`, ссылка `NodeStorage&` передаётся в `CreateObject`.
4. **Удаление устаревших заголовков:**
   - Файлы `VisualTypes.h` и `eVisualType.h` полностью удалены с диска и из `CMakeLists.txt`.
5. **Компактный `GameObject`:**
   - `GameObject` освобождён от визуальных данных, размер экземпляра минимизирован.
   - Скрипты сохранены как `std::shared_ptr<Script>`.
6. **Целостность Spatial Registration:**
   - Регистрация в `DefaultSpatialStorage` работает для всех узлов (и `Object`, и `Entity`) через `storage.HasMesh(nodeHandle)`.
7. **Чистая компиляция:**
   - Проект собирается через `run_build.bat` с 0 ошибок и 0 предупреждений.
   - Тесты не пишутся и не запускаются (Правило 5 и Блок 2 правил GEMINI.md).

---

## 6. Список затрагиваемых файлов

| Файл | Роль в этапе |
|---|---|
| `src/core/enums/eVisualType.h` | **Полное удаление файла** |
| `src/core/CMakeLists.txt` | Удаление `eVisualType.h` из списка файлов сборки |
| `src/engine/scene/storage/NodeTypes.h` | Добавление флага `Visible` в `eNodeFlags`, битовые операторы, объявление `VisualRange` и `DrawDescriptor` |
| `src/engine/scene/storage/NodeStorage.h` | Массивы `m_NodeVisuals`, `m_Draws`, методы `IsVisible/SetVisible`, геттеры `GetVisualRange`, `GetDraws` |
| `src/engine/scene/storage/NodeStorage.cpp` | Заполнение `m_NodeVisuals`, `m_Draws` и `m_Flags` в `InitializeFromObjects` с `reserve` и поддержкой MultiMesh fallback |
| `src/engine/scene/gameobject/GameObject.h` | Удаление `VisualPayload`, конструктор с `NodeStorage&`, прокси-геттеры, `shared_ptr<Script>` |
| `src/engine/scene/gameobject/GameObject.cpp` | Конструктор по Правилу 8.1 с `ensure`, очистка от `VisualPayload` |
| `src/engine/scene/domain/IObjectDomain.h` | Удаление хранения `NodeStorage*`, метод `CreateObject(NodeStorage&, ...)` |
| `src/engine/scene/domain/IObjectDomain.cpp` | Реализация `RegisterObject(NodeStorage&, ...)` |
| `src/engine/scene/domain/ObjectDomain2D.h/cpp` | Реализация `CreateObject(NodeStorage&, ...)`, удаление методов валидации `VisualPayload` |
| `src/engine/scene/domain/ObjectDomain3D.h/cpp` | Реализация `CreateObject(NodeStorage&, ...)`, удаление методов валидации `VisualPayload` |
| `src/engine/scene/visual/VisualTypes.h` | **Полное удаление файла** |
| `src/engine/CMakeLists.txt` | Удаление `VisualTypes.h` из списка файлов сборки |
| `src/engine/scene/layer/GameLayer.cpp` | Вызов `CreateObject(m_NodeStorage, nodeHandle, objData)` |
