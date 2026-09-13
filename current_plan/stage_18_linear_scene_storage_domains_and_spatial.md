# Этап 18: Линейное runtime-хранилище сцены, домены и spatial

## 1. Контекст, статус и цель

- **Номер этапа:** 18 (Уровень 2: GAPI-ресурсы, содержимое куба и сквозной рендер).
- **Статус:** ⏳ Детальный план подготовлен по прямой команде пользователя; реализация начинается только после завершения и приёмки этапов 16 и 17.
- **Цель:** построить простой и предсказуемый runtime-core слоя поверх гарантированного этапом 17 preorder-массива: линейно инициализировать мировые трансформы, пересчитывать каждое затронутое поддерево не более одного раза, убрать временные связи с `LayerData`, однозначно распределять узлы между Object/Entity domain и хранить в spatial только объекты с мешем.
- **Зависимости:** этап 17 гарантирует корректный плоский массив каждого слоя, `parentIndex < childIndex`, неизменяемые GUID и сохранение порядка JSON.
- **Следующий потребитель:** этап 19 использует готовые CPU-ресурсы независимо от сценовой топологии; этап 21 подключает скриптовый lifecycle; этап 22 после update формирует неизменяемые render-команды кадра `N`, исполняемые render-потоком как `N-1`.

На этом этапе иерархия после `Populate` неизменяема: runtime `spawn/destroy/reparent` не поддерживаются. Это позволяет использовать индекс узла как стабильный `NodeHandle` без generation и не тащить преждевременный `SlotMap` в простой core.

---

## 2. Согласованные контракты

### 2.1. Единственный источник трансформа

1. `NodeStorage` является единственным владельцем локального TRS, мировой матрицы, topology-флагов и связи узла с другими runtime-хранилищами.
2. `GameObject` хранит только ссылку на `NodeStorage` и `NodeHandle`; его transform API делегирует чтение/запись в `NodeStorage`.
3. Object/Entity domain не хранят копии TRS или world matrix.
4. `DefaultSpatialStorage` на этом этапе хранит только плотный список `NodeHandle` участников, а не копии мировых матриц. Поэтому после изменения transform обновляется только `NodeStorage`; domain и плоский spatial автоматически видят актуальное состояние через handle и не могут рассинхронизироваться.
5. Когда в последующем spatial получит производные данные (world AABB/BVH), он будет обновлять их по потоку изменённых диапазонов из `NodeStorage`. Фиктивное копирование matrix/position только ради будущего API в этап 18 не добавляется.

### 2.2. Стабильные handle первого runtime-core

Используются отдельные типы/алиасы поверх `zU32`:

```cpp
using NodeHandle = zU32;
using DomainHandle = zU32;
using SpatialHandle = zU32;

inline constexpr zU32 kInvalidHandle = 0xFFFFFFFF;
```

Смешивать значения разных пространств индексов без явного API нельзя. `NodeHandle` стабилен на всё время жизни `GameLayer`, потому что число узлов и их порядок после `Populate` не меняются. Стабильность `DomainHandle` и `SpatialHandle` в рамках этапа обеспечивается отсутствием runtime-удаления.

### 2.3. Cache-friendly topology

1. Текущая First-Child/Next-Sibling топология (`firstChildIndex`, `nextSiblingIndex`, `prevSiblingIndex`) удаляется из горячего пути: для preorder-массива она не нужна ни для начального расчёта, ни для каскадного transform update.
2. `NodeStorage` хранит отдельные плотные массивы минимум для:
   - `parentIndex`;
   - `subtreeEnd` — первый индекс после поддерева;
   - локального `Transform`;
   - world matrix;
   - компактных флагов узла;
   - холодных `NodeBindings`.
3. `isActive` сохраняется в flags, но не влияет на расчёт world matrix, domain update, регистрацию в spatial или будущий render на этом этапе. Эффективная активность с наследованием от родителя будет отдельной задачей.
4. Неиспользуемый `isStatic` не участвует в алгоритмах этапа. Его нельзя оставлять в горячей структуре как бессодержательную заготовку; при появлении реальной static-семантики флаг добавляется вместе с её потребителем.
5. `Transform` остаётся AoS внутри собственного непрерывного массива: при пересчёте одного узла одновременно нужны position, rotation и scale. World matrices хранятся отдельно и последовательно.

Для узла `i` выполняются инварианты:

```text
parentIndex == kInvalidNodeIndex || parentIndex < i
i < subtreeEnd[i] <= nodeCount
все потомки i находятся в [i + 1, subtreeEnd[i])
```

### 2.4. Линейная инициализация `NodeStorage`

1. Все векторы один раз получают окончательный размер по `objects.size()`.
2. В одном прямом проходе копируются parent, flags и локальный TRS, а world matrix рассчитывается сразу:

```cpp
world[i] = parent[i] == kInvalidNodeIndex
    ? LocalMatrix(local[i])
    : LocalMatrix(local[i]) * world[parent[i]];
```

3. `subtreeEnd` вычисляется линейно по preorder/topology (допустим отдельный обратный проход).
4. Конструктор не помечает каждый узел dirty и не запускает рекурсивный `ResolveSubtree`.
5. Нарушение `parentIndex < childIndex` и выход индексов за границы ловятся контрактной проверкой. Полная структурная проверка дерева остаётся в Assets Builder этапа 17 и не дублируется сложным runtime-валидатором.
6. Рекурсия из transform pipeline удаляется: глубина JSON-дерева не расходует стек runtime.

Сложность начальной сборки слоя — `O(N)` по времени и `O(N)` по данным без повторного пересчёта цепочек родителей.

### 2.5. Dirty tracker без ступенчатого перерасхода

`BitTreeTracker` сохраняется как 64-арный sparse tracker, но его размещение и жизненный цикл исправляются:

1. Число слов каждого уровня вычисляется от фактической ёмкости:

```text
leafWords = ceil(capacity / 64)
parentWords = ceil(childWords / 64)
... до одного корневого слова
```

2. Полный размер равен сумме фактических уровней. Переход с 4096 к 4097 узлам больше не выделяет полный уровень на 4096 leaf-слов.
3. Подготовка ёмкости отделяется от очистки состояния. `BeginFrame()` не сбрасывает dirty-биты.
4. Используется операция `ConsumeDirtyIndices()` с таким контрактом:
   - возвращает индексы по возрастанию во внутреннем заранее зарезервированном буфере;
   - очищает посещённые ветви/слова после сбора;
   - повторный вызов без новых `Set()` возвращает пустой span;
   - hot path не аллоцирует память.
5. `Prepare(capacity)` с двойной семантикой «resize плюс очистка кадра» заменяется явным API либо получает однозначное имя; все вызовы и комментарии приводятся к новому контракту.
6. Вместимость и промежуточные вычисления размеров проверяются до выделения памяти.

Ожидаемая память дерева — `O(N / 64)` слов с небольшой геометрической надбавкой, а не размер ближайшей полной пирамиды.

### 2.6. Однократный пересчёт перекрывающихся поддеревьев

`ResolveTransforms()` возвращает `std::span<const TransformChangeRange>` из переиспользуемого буфера:

```cpp
struct TransformChangeRange
{
    zU32 begin;
    zU32 end; // exclusive
};
```

Алгоритм:

1. Получить отсортированные dirty indices через consume.
2. Идти по ним по возрастанию и держать `coveredUntil`.
3. Если dirty index меньше `coveredUntil`, его уже покрывает ранее пересчитанный ancestor и повторный обход пропускается.
4. Для нового dirty root последовательно пересчитать диапазон `[root, subtreeEnd[root])`; parent каждой следующей ноды уже актуален благодаря preorder.
5. Добавить/объединить соответствующий изменённый диапазон и установить `coveredUntil = subtreeEnd[root]`.
6. Вернуть span, валидный до следующего `ResolveTransforms()`/перестроения storage.

Итоговая сложность — `O(D + C)`, где `D` — число явно отмеченных dirty-узлов, `C` — число уникально затронутых узлов. Один узел не пересчитывается дважды в одном resolve, даже если до него были изменены и родитель, и несколько потомков.

### 2.7. Фаза кадра и однопоточный update

Порядок работы `GameLayer`:

```text
BeginFrame слоя
  -> domain update / будущие script setters
  -> NodeStorage::ResolveTransforms()
  -> последующие потребители TransformChangeRange
  -> в этапе 22 публикация immutable render-команд N
```

1. Весь update и transform resolve выполняются одним logic thread.
2. Dirty state очищается только при consume внутри resolve, поэтому setter между `BeginFrame` и `Update` не может быть потерян.
3. `ILayer::BeginFrame` может остаться общей фазой lifecycle, но `GameLayer` больше не вызывает в ней очистку `NodeStorage`.
4. Render thread не получает доступ к mutable `NodeStorage`. Снимок команд `N/N-1` реализуется в этапе 22 после завершения update; в этапе 18 не вводятся половинчатые thread-safe обёртки.

### 2.8. Живые `NodeBindings` и маршрутизация доменов

```cpp
enum class eNodeDomainKind : zU8
{
    None,
    Object,
    Entity
};

struct NodeBindings
{
    DomainHandle domainHandle{ kInvalidHandle };
    SpatialHandle spatialHandle{ kInvalidHandle };
    eNodeDomainKind domainKind{ eNodeDomainKind::None };
};
```

1. `layerObjectIndex` удаляется: после `Populate` он ссылается на временный `LayerData` и не является runtime-binding. Во время заполнения `objects[nodeIndex]` уже соответствует `NodeHandle` благодаря этапу 17.
2. Для каждого узла `isEntity == false` создаётся ровно одна запись в `IObjectDomain`; binding получает `Object` и возвращённый `DomainHandle`.
3. Для каждого узла `isEntity == true` создаётся ровно одна запись в `IEntityDomain`; binding получает `Entity` и возвращённый `DomainHandle`.
4. Одновременная регистрация одного узла в двух доменах запрещена. После `Populate` `None` также запрещён.
5. `NodeStorage` предоставляет узкие setters/getters bindings; внешние подсистемы не получают mutable span всего массива.

Для Object domain создание возвращает и handle, и указатель, необходимый текущему `GameObject::Initialize`, например:

```cpp
struct ObjectRegistration
{
    DomainHandle handle;
    GameObject* object;
};
```

Для Entity domain `CreateEntity` возвращает `DomainHandle`, а `EntityStub` временно получает `NodeHandle`. Конкретная ECS-компонентная модель остаётся будущим этапом.

### 2.9. Минимальная адаптация доменных хранилищ

1. `IObjectDomain` хранит объекты в плотном индексируемом массиве `std::vector<std::unique_ptr<GameObject>>`; `DomainHandle` является индексом.
2. GUID-индекс хранит `Guid -> DomainHandle`, а индекс имён — handles вместо сырых указателей. `FindObjectByGuid/Name` разрешает handle через плотный массив.
3. Перед `Populate` домены получают `Reserve` по фактическому числу объектов своего вида, чтобы исключить рост внутренних векторов во время построения слоя.
4. Отдельные `GameObject` пока остаются heap-объектами, чтобы не менять адреса, на которые ссылаются скриптовые заготовки. Устранение этих аллокаций и полноценный скриптовый hot path относятся к этапу 21.
5. `EntityWorld` остаётся простой плотной заготовкой, но возвращает индекс созданной записи и хранит `NodeHandle`.
6. Загрузку Mesh через текущий `ResourceManager` этот этап не перерабатывает и не оценивает: ресурсы/GPU находятся в этапах 16, 19 и 20.
7. Runtime destroy не используется до определения политики стабильности handles. Существующие методы удаления не должны вызываться из кадрового пути этапа 18; их окончательный контракт определяется вместе с runtime-мутациями.

### 2.10. Плоский spatial только для узлов с мешем

1. `GameLayer::Populate` регистрирует узел в `DefaultSpatialStorage` только когда `GameObjectData::HasMesh() == true`, независимо от `isEntity` и `isActive`.
2. Узел без меша получает `spatialHandle == kInvalidHandle`.
3. Узел с мешем получает handle, возвращённый spatial; предположение `spatialHandle == nodeIndex` удаляется.
4. `DefaultSpatialStorage` на первом этапе — плотный `std::vector<NodeHandle>` без свободных слотов, `isOccupied` и free-list, потому что runtime insert/remove не входят в текущий контракт.
5. Чтение для следующего потребителя выполняется allocation-free через `std::span<const NodeHandle>`, а не через `GetAll(std::vector<uint64_t>&)` с копированием каждого кадра.
6. `Build(const NodeStorage&)`, который не знает наличия меша и сейчас ошибочно использует `isActive`, заменяется явной регистрацией из `GameLayer::Populate`.
7. На этом этапе spatial не является BVH/Octree и не обещает frustum/raycast. Это компактный список потенциально рисуемых mesh-узлов и точка последующего расширения.

### 2.11. Lifetime слоя

`GameObject` содержит невладеющий указатель на `NodeStorage`. Поэтому порядок членов `GameLayer` должен гарантировать, что `NodeStorage` создаётся раньше доменов и уничтожается после них. Порядок объявления членов исправляется с учётом обратного порядка деструкторов C++, а не оставляется неявной зависимостью.

---

## 3. Порядок реализации

### Шаг 1. Типы handles и topology

- Ввести раздельные `NodeHandle`, `DomainHandle`, `SpatialHandle` и invalid constants.
- Заменить FCNS topology на плотные `parentIndex`, `subtreeEnd` и flags.
- Удалить `layerObjectIndex`, добавить живые domain/spatial bindings.
- Сузить публичный API `NodeStorage` до необходимых getters/setters и read-only spans.

### Шаг 2. Исправление `BitTreeTracker`

- Вычислять фактические размеры и offsets уровней от capacity.
- Разделить resize и очистку/consume.
- Реализовать сбор dirty indices с очисткой посещённых слов и без кадровых аллокаций.
- Адаптировать существующие места использования и старые QA-исходники только насколько требуется для их компиляции; новые unit-тесты не добавлять.

### Шаг 3. Линейный `NodeStorage`

- Однократно разместить массивы и заполнить их из preorder `GameObjectData`.
- Рассчитать `subtreeEnd` и initial world matrices линейными проходами.
- Удалить `ResolveSubtree` и начальную маркировку всех узлов dirty.
- Реализовать пропуск перекрывающихся dirty-поддеревьев и выдачу объединённых `TransformChangeRange`.
- Убрать влияние `isActive` на transform resolve.

### Шаг 4. Доменные handles

- Сделать Object domain плотным индексируемым владельцем с GUID/name индексами по handle.
- Возвращать `ObjectRegistration` при создании объекта.
- Возвращать handle из Entity domain и связать `EntityStub` с `NodeHandle`.
- Добавить `Reserve` и сохранить взаимно исключающую маршрутизацию `isEntity`.

### Шаг 5. Минимальный spatial

- Заменить slot/free-list scaffold плотным массивом mesh-node handles.
- Сделать чтение через span без копирования.
- Регистрировать только `HasMesh()` и записывать фактически выданный handle в `NodeBindings`.
- Не фильтровать по `isActive` и не копировать world matrix в spatial.

### Шаг 6. Сборка `GameLayer`

- Сначала создать окончательный `NodeStorage`.
- Посчитать Object/Entity/mesh количества и зарезервировать домены/spatial.
- Одним проходом создать ровно один domain object/entity на node и при необходимости spatial entry.
- Проверить заполненность и корректность bindings.
- Исправить порядок владения членами слоя.
- Оставить resource-loading код без архитектурных изменений.

### Шаг 7. Кадровый путь

- Удалить преждевременный dirty reset из `GameLayer::BeginFrame`.
- После domain update выполнить один `ResolveTransforms`.
- Сохранить изменённые диапазоны доступными последующему потребителю в пределах текущей logic-фазы.
- Не добавлять параллельный доступ к сцене и render thread в этот этап.

### Шаг 8. Верификация и приёмка

- Собрать затронутые таргеты через `run_build.bat`; новые ошибки и предупреждения не допускаются.
- Упаковать постоянный проект после реализации этапа 17 и запустить игровой таргет до успешного `Scene::Initialize`/`GameLayer::Populate`.
- По debugger/существующему диагностическому пути подтвердить: world matrices корректны для нескольких уровней; изменение родителя обновляет всё поддерево; одновременная отметка родителя и ребёнка не пересчитывает ребёнка дважды.
- Подтвердить, что число spatial entries равно числу `HasMesh()`, а не числу всех/активных nodes; object и entity получают взаимно исключающие bindings.
- Подтвердить отсутствие аллокаций в повторном `SetLocal* -> ResolveTransforms` после предварительного reserve и отсутствие ступенчатого скачка dirty-tree на границе 4096/4097.
- Зафиксировать дату, конфигурацию и фактически выполненные проверки в этом документе.
- Новые unit-тесты и отдельный `ctest` не входят в этап по решению пользователя; существующие тестовые исходники меняются только при необходимости собрать изменённые API.
- Статус в `general_plan.md` меняется на `✅ Выполнено` только после явного подтверждения пользователя «Шаг принят».

---

## 4. Основные затрагиваемые файлы

### Dirty tracking и scene storage

- `src/core/containers/BitTreeTracker.h`
- `src/core/containers/BitTreeTracker.cpp`
- `src/engine/scene/storage/NodeTypes.h`
- `src/engine/scene/storage/NodeStorage.h`
- `src/engine/scene/storage/NodeStorage.cpp`

### Domains и GameObject

- `src/engine/scene/domain/IObjectDomain.h`
- `src/engine/scene/domain/IObjectDomain.cpp`
- `src/engine/scene/domain/ObjectDomain2D.h/.cpp`
- `src/engine/scene/domain/ObjectDomain3D.h/.cpp`
- `src/engine/scene/domain/IEntityDomain.h`
- `src/engine/scene/domain/EntityDomain.h`
- `src/engine/scene/domain/EntityDomain.cpp`
- `src/engine/scene/entity/EntityWorld.h`
- `src/engine/scene/entity/EntityWorld.cpp`
- `src/engine/scene/gameobject/GameObject.h`
- `src/engine/scene/gameobject/GameObject.cpp`

### Layer и spatial

- `src/engine/scene/storage/ISpatialStorage.h`
- `src/engine/scene/storage/DefaultSpatialStorage.h`
- `src/engine/scene/storage/DefaultSpatialStorage.cpp`
- `src/engine/scene/layer/ILayer.h`
- `src/engine/scene/layer/GameLayer.h`
- `src/engine/scene/layer/GameLayer.cpp`
- связанные CMake/QA-исходники — только при фактической необходимости сборки

---

## 5. Критерии готовности

- [ ] Начальная сборка topology и всех world matrices выполняется линейно и без рекурсии.
- [ ] Для каждого узла соблюдаются `parentIndex < childIndex` и корректный непрерывный `subtreeEnd`.
- [ ] Dirty tree выделяет память пропорционально фактической ёмкости и не имеет скачка полного уровня на 4096/4097.
- [ ] Dirty-биты переживают `BeginFrame` и очищаются только при consume.
- [ ] Перекрывающиеся dirty-поддеревья не приводят к повторному пересчёту узла.
- [ ] Повторный transform update не делает heap allocations после подготовки ёмкости.
- [ ] `NodeStorage` является единственным источником TRS/world; домены и spatial не содержат устаревающих копий.
- [ ] Каждый node связан ровно с одним Object/Entity domain через `domainKind/domainHandle`.
- [ ] `layerObjectIndex` удалён из runtime bindings.
- [ ] В spatial зарегистрированы только `HasMesh()` nodes, независимо от `isEntity` и сохранённого `isActive`.
- [ ] Spatial handle не отождествляется с node index; узлы без меша имеют invalid spatial handle.
- [ ] Плоский spatial читается allocation-free как span handles.
- [ ] `NodeStorage` живёт дольше ссылающихся на него `GameObject`.
- [ ] Слой успешно создаётся из упакованной сцены этапа 17, а существующий executable доходит до начала/завершения `Populate` без новых ошибок и предупреждений.
- [ ] Результаты фактически выполненной сборки/прогона записаны с датой и конфигурацией.
- [ ] Пользователь явно подтвердил: «Шаг принят».

---

## 6. Не входит в этап

- Runtime `spawn/destroy/reparent`, generation handles, compaction и сохранение handles при удалении.
- Эффективная активность, наследование `isActive`, фильтрация update/spatial/render по активности.
- Полноценный ECS, компоненты Entity и оптимизация его систем.
- Жизненный цикл скриптов, GUID-резолв объектов для скриптов, пользовательский `OnStart/OnUpdate/OnDestroy` и hot reload — этап 21.
- Полное устранение отдельных heap allocations `GameObject` — этап 21 после фиксации скриптового ownership.
- Копирование/двойная буферизация render-команд и исполнение `N-1` — этап 22.
- AABB, BVH/Octree, frustum culling, raycast и инкрементальное обновление производного spatial index.
- Изменения `SceneManager`, включая in-flight дедупликацию и политику выгрузки.
- ResourceManager, GPU upload, material/shader readiness.
- Prefab и его идентичность.
- Новые unit-тесты.
