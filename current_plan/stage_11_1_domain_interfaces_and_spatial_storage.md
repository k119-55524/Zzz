# Этап 11.1: Интерфейсы доменов и пространственное хранилище (Domain Interfaces & Spatial Storage)

## 1. Контекст и цели подэтапа
- **Номер подпункта:** **Пункт 11.1** (часть Этапа 11: Структура хранения GameObject в сцене).
- **Цель:** Заложить интерфейсный фундамент абстракции слоёв сцены и пространственного индексирования без фантомных типов:
  1. Переименование `eLayerType::LayerUI` в `eLayerType::Layer2D` для подготовки перехода к полнофункциональному 2D-миру.
  2. Разработка базовых абстракций доменов сцены (`ILayerDomain`, `IObjectDomain`, `IEntityDomain`, `IDomainFactory`).
  3. Разработка нового интерфейса пространственного хранилища `ISpatialStorage` и его базовой реализации `DefaultSpatialStorage` на замену устаревшему `ISceneStorage`.
- **Статус:** ⏳ В процессе разработки.
- **Зависимости:** `src/core/enums/eLayerType.h`, `src/core/utils/Guid.h`, `src/math/vector/Vec3.h`.

### Физическое размещение файлов
```text
src/
├── core/
│   └── enums/
│       └── eLayerType.h                     <-- Переименование LayerUI -> Layer2D
│
└── engine/
    └── scene/
        ├── domain/                          <-- Новая папка интерфейсов доменов
        │   ├── ILayerDomain.h               <-- Базовый интерфейс домена
        │   ├── IObjectDomain.h              <-- Интерфейс ООП-домена (GameObject)
        │   ├── IEntityDomain.h              <-- Интерфейс ECS-домена (Entity)
        │   └── IDomainFactory.h             <-- Фабрика создания доменов
        │
        └── storage/                         <-- Пространственное хранилище
            ├── ISpatialStorage.h            <-- Интерфейс пространственного ускорения
            └── DefaultSpatialStorage.h/.cpp <-- Базовое хранилище (слоты O(1))
```

---

## 2. Архитектурные спецификации

### 2.1. Иерархия интерфейсов доменов (`ILayerDomain`) и фабрика (`IDomainFactory`)

Любой домен внутри слоя (как ООП, так и ECS) реализует базовый контракт `ILayerDomain`:

```cpp
namespace zzz::engine
{
    class ILayerDomain
    {
    public:
        virtual ~ILayerDomain() = default;

        // Кадровое обновление домена игровой логики
        virtual void Update(float dt) = 0;

        // Очистка состояния домена при смене сцены
        virtual void Clear() = 0;
    };

    class IObjectDomain : public ILayerDomain
    {
    public:
        virtual ~IObjectDomain() override = default;
        virtual ::zzz::GameObject* CreateObject(const ::zzz::core::Guid& guid, std::string name) = 0;
        virtual void DestroyObject(::zzz::GameObject* obj) = 0;

        // Поиск объекта по Guid (используется во 2-м проходе Populate для связывания иерархии parentGuid и скриптами)
        [[nodiscard]] virtual ::zzz::GameObject* FindObjectByGuid(const ::zzz::core::Guid& guid) const noexcept = 0;

        // Выборка всех активных объектов слоя (используется для сериализации слоя и отладочной инспекции)
        virtual void GetAllObjects(std::vector<::zzz::GameObject*>& outObjects) const = 0;
    };

    class IEntityDomain : public ILayerDomain
    {
    public:
        virtual ~IEntityDomain() override = default;
        virtual void CreateEntity(const ::zzz::core::Guid& guid, std::string_view name) = 0;
        virtual void DestroyEntity(const ::zzz::core::Guid& guid) = 0;
    };

    class IDomainFactory
    {
    public:
        virtual ~IDomainFactory() = default;
        virtual std::unique_ptr<IObjectDomain> CreateObjectDomain() = 0;
        virtual std::unique_ptr<IEntityDomain> CreateEntityDomain() = 0;
    };
}
```

### 2.2. Пространственный индекс (`ISpatialStorage`)

В соответствии с **Правилом 17** (строгая очерёдность этапов: `Camera`/`Frustum` — Этап 18, `RenderQueue` — Этап 20, физика/`Raycast` — Этап 25), на **Этапе 11.1** интерфейс `ISpatialStorage` реализует фундамент пространственного хранения над существующими типами (`::zzz::math::Vec3<zF32>`, `uint64_t userData`), без преждевременного введения несуществующих классов:

```cpp
namespace zzz::engine
{
    using SpatialHandle = uint32_t;
    constexpr SpatialHandle c_InvalidSpatialHandle = 0xFFFFFFFF;

    class ISpatialStorage
    {
    public:
        virtual ~ISpatialStorage() = default;

        // Регистрация и удаление объектов
        virtual SpatialHandle Insert(uint64_t userData) = 0;
        virtual void Remove(SpatialHandle handle) = 0;
        virtual void Clear() = 0;

        // Базовая линейная выборка всех зарегистрированных объектов
        virtual void GetAll(std::vector<uint64_t>& outUserData) const = 0;
        [[nodiscard]] virtual size_t GetCount() const noexcept = 0;

        // Точки расширения на последующих этапах:
        // - AABB / BoundingSphere появится в math/geometry
        // - QueryFrustum(const Camera&) появится на Этапе 18 (Камера и проекции)
        // - Raycast появится на Этапе 25 (Физический мир)
    };
}
```

Реализация `DefaultSpatialStorage` заменяет старый `DefaultSceneStorage` и хранит элементы с быстрым $O(1)$ доступом по `SpatialHandle` (на базе плоского вектора слотов с `FreeList`).

---

## 3. Чек-лист Definition of Done (DoD)

- [x] Переименовать `eLayerType::LayerUI` в `eLayerType::Layer2D` в `src/core/enums/eLayerType.h` и обновить `PackagePacker.cpp` (с поддержкой обратной совместимости)
- [x] Спроектировать и реализовать интерфейсы `ILayerDomain`, `IObjectDomain`, `IEntityDomain` и `IDomainFactory` в `src/engine/scene/domain/`
- [x] Спроектировать интерфейс `ISpatialStorage` в `src/engine/scene/storage/ISpatialStorage.h` (на базе `SpatialHandle`, `userData`, `GetAll`)
- [x] Реализовать `DefaultSpatialStorage` в `src/engine/scene/storage/DefaultSpatialStorage.h` / `.cpp`
- [x] Зарегистрировать новые файлы в `src/engine/CMakeLists.txt`
- [x] Проверить сборку под MSVC x64 + Ninja (0 ошибок)
