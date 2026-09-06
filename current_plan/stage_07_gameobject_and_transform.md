# Этап 07: Сущность сцены (`GameObject`), иерархический `Transform` и `GameObjectData`

## 1. Контекст и цели этапа
- **Номер пункта:** **Пункт 7** (Уровень 2: GAPI-ресурсы, содержимое куба и сквозной рендер).
- **Цель:** Спроектировать фундаментальный узел сцены — игровой объект (`GameObject`) и его неотъемлемую пространственную основу (`Transform`). Обеспечить иерархию родитель/дети, вычисление мировой матрицы ($TRS$) с механизмом ленивой инвалидации (`dirty flag`), слоты под меш и материал, а также бинарный контейнер `GameObjectData` в `src/core/io/package/` для интеграции со сценами и префабами.
- **Обоснование позиции в плане:** Сцена состоит из `GameObject`'ов. Создание структуры объекта и его пространственной трансформации первым шагом позволяет затем загружать меши, текстуры и материалы прямо в готовые целевые слоты сцены.
- **Статус:** `🔄 В работе`.
- **Зависимости:** `src/math/vector/Vec3.h`, `src/math/matrix/Mat4.h`, `src/math/quat/Quat.h`, `src/core/utils/Guid.h`, `src/core/serialize/Serializer.h`.

---

## 2. Архитектурные принципы

1. **Компонентная основа (Transform как прямое поле, а не указатель):**
   - У каждого `GameObject` **всегда** есть экземпляр `Transform`, который хранится как **прямое поле-значение (`Transform m_Transform;`)**, а не как указатель на кучу (`unique_ptr`/`shared_ptr`).
   - Это полностью исключает лишнюю динамическую аллокацию в куче, проверки на `nullptr` и обеспечивает 100% кэш-локальность: память трансформа лежит в том же сплошном блоке, что и сам `GameObject`.
   - `Transform` владеет позицией (`Vec3`), поворотом (`Quat`) и масштабом (`Vec3`).
2. **Иерархия сцены в `GameObject` (Parent / Children):**
   - Древовидную связь сцены хранит сам `GameObject`: указатель на родителя `GameObject* m_Parent` и список дочерних элементов `std::vector<GameObject*> m_Children`.
   - `Transform` фокусируется строго на локальной и мировой пространственной математике ($TRS$), не раздувая свою ответственность графом сцены.
   - При смене родителя локальные координаты пересчитываются относительно нового родителя ($M_{local\_new} = M_{parent\_world\_new}^{-1} \times M_{world\_old}$), а мир уведомляется для актуализации своего состояния.
3. **Ленивый пересчёт матриц (Dirty Flag Pattern):**
   - Матрица мира $M_{world} = M_{parent\_world} \times M_{local}$ вычисляется **только по требованию** (`GetWorldMatrix()`).
   - При изменении локальной позиции, поворота или масштаба выставляется флаг `m_IsDirty = true`, который каскадно помечает `dirty` всех потомков `GameObject`.
4. **Слоты ресурсов (`MeshGuid`, `MaterialGuid`):**
   - Для отрисовки куба (и любых 3D объектов) `GameObject` хранит идентификаторы привязанных ассетов: `Guid m_MeshGuid` и `Guid m_MaterialGuid`.
   - Если идентификаторы пустые (`guid.IsEmpty()`) — объект является пустым узлом (Empty / Pivot / Marker / Group).
5. **Сериализация сцены (`GameObjectData`):**
   - В бинарном архиве `package.dat` и файлах сцен `.zs` объекты сохраняются через структуру `GameObjectData`, содержащую домен объекта (`eObjectDomain`), имя, GUID, пространственные параметры и связи с ресурсами.
6. **Двухмировая модель хранения и обновления (Zero-Branching в горячем цикле):**
   - Внутри `Scene` хранение разделено на два изолированных мира:
     - `ObjectWorld`: классические `GameObject` с иерархией `Transform` и пользовательскими скриптами `Script`.
     - `EntityWorld`: точка роста под плоские пакетные структуры данных и системы.
   - В горячем цикле кадра `Scene::Update()` нет ветвлений `if`: `ObjectWorld::Update()` крутит свои объекты, а `EntityWorld::Update()` прогоняет свои пакеты.
   - Классификация и выбор целевого мира происходят ровно один раз при загрузке сцены по полю `eObjectDomain`.
7. **Система слоёв сцены (`ILayer`):**
   - Сцена (`Scene`) управляет списком слоёв (`std::vector<std::unique_ptr<ILayer>> m_Layers`).
   - Каждый слой инкапсулирует свою логику обновления, видимости и подготовки к рендеру:
     - `Layer3D`: основной 3D мир (объекты сцены, `GameObject`, освещение).
     - `LayerUI`: стандартный прямой UI / HUD (спрайты, панели, экранный оверлей).
     - `LayerMVVM`: реактивный data-driven UI (ViewModels, дата-биндинги, иерархический граф элементов).
8. **Пространственное хранилище и выборка (`ISceneStorage`):**
   - Каждый слой владеет собственной стратегией геометрической/пространственной выборки объектов (`ISceneStorage`).
   - Для `Layer3D` по умолчанию подключается `DefaultSceneStorage` (линейное хранилище с отдачей всех объектов без отсечения, являющееся эталоном для последующих `QuadTree` / `BVH` / `Octree`).
   - Сцена регулирует порядок отрисовки слоёв, а каждый слой через своё хранилище собирает видимые объекты для конвейера рендера.
9. **Стабильная идентичность `GameObject` и защита связей:**
   - `GameObject` является сущностью со стабильной идентичностью (`Z_NO_COPY_MOVE(GameObject)`).
   - `Transform::m_Owner` (`GameObject&`), пользовательские скрипты (`Script::m_Owner`, `GameObject*`) и иерархия сцены (`m_Parent`, `m_Children`) требуют 100% стабильности адреса `this`.
   - Поэтому сам `GameObject` создаётся со стабильным адресом в памяти (блочный/динамический пул).
10. **Элиминация кэш-штрафов и $O(1)$ удаление (`SlotMap<GameObject*>` + `SlotHandle`):**
    - `ObjectWorld` держит непрерывный плотный массив указателей `SlotMap<GameObject*>`.
    - Сам `GameObject` хранит свой `SlotHandle m_WorldHandle` (8 байт), что даёт строго $O(1)$ удаление в `DestroyObject(GameObject*)` без линейного поиска и без промежуточных `unordered_map`.
    - При обходе `Update()` процессор бежит по непрерывному массиву без пропусков.
    - При удалении объекта из мира удаляется его слот в `SlotMap` через Swap & Pop (перемещается только 8-байтный указатель в массиве, сам `GameObject` не двигается и его внутренние ссылки не ломаются).
11. **Согласованность с Правилом 13 («Строгая последовательность зависимостей») и Правилом 30 («YAGNI»):**
    - На Шаге 7 не вводятся фиктивные зависимости от ещё не реализованных компонентов (`Camera` — Шаг 13, `RenderCommand` / `RenderQueue` — Шаг 17, `Frustum` / Culling — Шаг 27).
    - Контракты `ILayer` и `ISceneStorage` содержат только методы, реально востребованные на Шаге 7 (`Insert`, `Remove`, `Clear`, `GetAll`, `Update(dt)`).
    - Скелетные заготовки слоёв `LayerUI` и `LayerMVVM` создаются с минимальным контрактом (пустой `Update`), чтобы сцена `Scene` была готова принимать UI без изменения своей структуры.
    - Методы сбора видимых для рендера (`CollectRenderables`, `QueryVisible`) будут добавлены точно в срок на соответствующих шагах при появлении потребителей.
12. **Многопоточная изоляция кадра через неизменяемый снимок (Immutable Render Snapshot, Правило 28):**
    - Поток рендеринга (Render Thread) физически изолирован от объектов логики сцены (`GameObject`, `Transform`, `ObjectWorld`).
    - Вся экстракция данных из объектов сцены в плоский список кадровых команд отрисовки (`RenderCommand`, `RenderQueue`) реализуется в Шаге 17.
    - Данные рендера (мировые матрицы $TRS$, указатели на ресурсы) будут передаваться в рендер-поток по значению как неизменяемый снимок (Immutable Snapshot) через атомарный `SwapQueue::Swap`.
    - Это фундаментально и навсегда устраняет гонки данных (Data Races) между потоком логики и потоком рендера без использования мьютексов и блокировок (Zero-Lock).

---

## 3. Архитектурная спецификация

### 3.1. Класс пространственной трансформации: `src/engine/scene/Transform.h` и `.cpp`

```cpp
// src/engine/scene/Transform.h
#pragma once

#include "math/vector/Vec3.h"
#include "math/matrix/Mat4.h"
#include "math/quat/Quat.h"
#include "core/utils/Defines.h"

namespace zzz
{
	class GameObject;

	/**
	 * @class Transform
	 * @brief Пространственная трансформация игрового объекта в 3D мире.
	 *
	 * @details Управляет локальным положением, вращением (кватернион), масштабом и вычислением TRS.
	 * Реализует кэширование мировой матрицы через dirty flag.
	 */
	class Transform final
	{
	public:
		explicit Transform(GameObject& owner) noexcept;
		~Transform() = default;

		Z_NO_COPY_MOVE(Transform);

		// --- Локальные параметры ---
		[[nodiscard]] const ::zzz::math::Vec3<zF32>& GetLocalPosition() const noexcept { return m_LocalPosition; }
		[[nodiscard]] const ::zzz::math::Quat<zF32>& GetLocalRotation() const noexcept { return m_LocalRotation; }
		[[nodiscard]] const ::zzz::math::Vec3<zF32>& GetLocalScale() const noexcept { return m_LocalScale; }

		void SetLocalPosition(const ::zzz::math::Vec3<zF32>& position) noexcept;
		void SetLocalRotation(const ::zzz::math::Quat<zF32>& rotation) noexcept;
		void SetLocalScale(const ::zzz::math::Vec3<zF32>& scale) noexcept;

		// --- Мировые параметры ---
		[[nodiscard]] ::zzz::math::Vec3<zF32> GetWorldPosition() const noexcept;
		[[nodiscard]] ::zzz::math::Quat<zF32> GetWorldRotation() const noexcept;
		[[nodiscard]] const ::zzz::math::Mat4<zF32>& GetWorldMatrix() const noexcept;
		[[nodiscard]] ::zzz::math::Mat4<zF32> GetLocalMatrix() const noexcept;

		// --- Базисные векторы направления ---
		[[nodiscard]] ::zzz::math::Vec3<zF32> GetForward() const noexcept;
		[[nodiscard]] ::zzz::math::Vec3<zF32> GetUp() const noexcept;
		[[nodiscard]] ::zzz::math::Vec3<zF32> GetRight() const noexcept;

		// --- Трансформационные операции ---
		void Translate(const ::zzz::math::Vec3<zF32>& delta) noexcept;
		void Rotate(const ::zzz::math::Quat<zF32>& deltaRotation) noexcept;
		void LookAt(const ::zzz::math::Vec3<zF32>& target, const ::zzz::math::Vec3<zF32>& up = { 0.0f, 1.0f, 0.0f }) noexcept;

		// --- Инвалидация и пересчёт ---
		void SetDirty() noexcept;
		[[nodiscard]] GameObject& GetGameObject() const noexcept { return m_Owner; }

	private:
		void UpdateWorldMatrix() const noexcept;

		GameObject& m_Owner;

		::zzz::math::Vec3<zF32> m_LocalPosition{ 0.0f, 0.0f, 0.0f };
		::zzz::math::Quat<zF32> m_LocalRotation{ 0.0f, 0.0f, 0.0f, 1.0f };
		::zzz::math::Vec3<zF32> m_LocalScale{ 1.0f, 1.0f, 1.0f };

		mutable ::zzz::math::Mat4<zF32> m_WorldMatrix{ ::zzz::math::Mat4<zF32>::Identity() };
		mutable bool m_IsDirty{ true };
	};
}
```

---

### 3.2. Рефакторинг `GameObject`: `src/engine/scene/GameObject.h` и `.cpp`

```cpp
// src/engine/scene/GameObject.h
#pragma once

#include <string>
#include <vector>
#include <memory>
#include "core/utils/Guid.h"
#include "core/templates/SlotMap.h"
#include "engine/scene/Transform.h"

namespace zzz::core
{
	class Script;
}

namespace zzz
{
	/**
	 * @class GameObject
	 * @brief Сущность игрового мира, объединяющая иерархию сцены, Transform, ресурсы и скрипты.
	 */
	class GameObject final
	{
	public:
		explicit GameObject(std::string name = "GameObject");
		GameObject(::zzz::core::Guid guid, std::string name);
		~GameObject() = default;

		Z_NO_COPY_MOVE(GameObject);

		// --- Идентификация ---
		[[nodiscard]] const ::zzz::core::Guid& GetGuid() const noexcept { return m_Guid; }
		[[nodiscard]] const std::string& GetName() const noexcept { return m_Name; }
		void SetName(std::string name) { m_Name = std::move(name); }

		// --- Хэндл в ObjectWorld (O(1) удаление из SlotMap) ---
		[[nodiscard]] ::zzz::core::SlotHandle GetWorldHandle() const noexcept { return m_WorldHandle; }
		void SetWorldHandle(::zzz::core::SlotHandle handle) noexcept { m_WorldHandle = handle; }

		// --- Активность и жизненный цикл в кадре ---
		[[nodiscard]] bool IsActive() const noexcept { return m_IsActive; }
		void SetActive(bool active) noexcept { m_IsActive = active; }

		[[nodiscard]] uint8_t GetRenderFramesRemaining() const noexcept { return m_RenderFramesRemaining; }
		void DecrementRenderFrames() noexcept { if (m_RenderFramesRemaining > 0) --m_RenderFramesRemaining; }
		void ResetRenderFrames(uint8_t bufferCount = 2) noexcept { m_RenderFramesRemaining = bufferCount; }

		// --- Пространственная трансформация ---
		[[nodiscard]] Transform& GetTransform() noexcept { return m_Transform; }
		[[nodiscard]] const Transform& GetTransform() const noexcept { return m_Transform; }

		// --- Иерархия сцены (Parent / Children) ---
		[[nodiscard]] GameObject* GetParent() const noexcept { return m_Parent; }
		void SetParent(GameObject* newParent, bool keepWorldTransform = true) noexcept;

		[[nodiscard]] const std::vector<GameObject*>& GetChildren() const noexcept { return m_Children; }
		[[nodiscard]] size_t GetChildCount() const noexcept { return m_Children.size(); }
		[[nodiscard]] GameObject* GetChild(size_t index) const noexcept;

		// --- Слоты графических ресурсов (для отрисовки меша и материала) ---
		[[nodiscard]] const ::zzz::core::Guid& GetMeshGuid() const noexcept { return m_MeshGuid; }
		void SetMeshGuid(const ::zzz::core::Guid& guid) noexcept { m_MeshGuid = guid; }
		[[nodiscard]] bool HasMesh() const noexcept { return !m_MeshGuid.IsEmpty(); }

		[[nodiscard]] const ::zzz::core::Guid& GetMaterialGuid() const noexcept { return m_MaterialGuid; }
		void SetMaterialGuid(const ::zzz::core::Guid& guid) noexcept { m_MaterialGuid = guid; }
		[[nodiscard]] bool HasMaterial() const noexcept { return !m_MaterialGuid.IsEmpty(); }

		// --- Скрипты поведения ---
		void AddScript(std::shared_ptr<::zzz::core::Script> script);
		void RemoveScript(const std::shared_ptr<::zzz::core::Script>& script);
		void RemoveAllScripts();
		[[nodiscard]] const std::vector<std::shared_ptr<::zzz::core::Script>>& GetScripts() const noexcept { return m_Scripts; }

	private:
		::zzz::core::Guid m_Guid;
		std::string m_Name;
		::zzz::core::SlotHandle m_WorldHandle{};
		bool m_IsActive{ true };
		uint8_t m_RenderFramesRemaining{ 2 }; // Frames in Flight safety

		Transform m_Transform;

		GameObject* m_Parent{ nullptr };
		std::vector<GameObject*> m_Children;

		::zzz::core::Guid m_MeshGuid;
		::zzz::core::Guid m_MaterialGuid;

		std::vector<std::shared_ptr<::zzz::core::Script>> m_Scripts;
	};
}
```

---

### 3.3. Сериализуемый контейнер: `src/core/io/package/GameObjectData.h` и `.cpp`

```cpp
#pragma once

#include <string>
#include <vector>
#include <span>
#include <cstddef>
#include "core/utils/Guid.h"
#include "core/serialize/Serializer.h"
#include "math/vector/Vec3.h"
#include "math/quat/Quat.h"

namespace zzz::core
{
	enum class eObjectDomain : uint8_t
	{
		Object = 0, ///< Классический GameObject с иерархией Transform и собственными скриптами
		Entity = 1  ///< Высокоскоростная пакетная сущность для EntityWorld
	};

	/**
	 * @class GameObjectData
	 * @brief Сериализуемое представление игрового объекта в package.dat / SceneData.
	 */
	class GameObjectData final : public ISerializable
	{
	public:
		GameObjectData() = default;
		GameObjectData(
			Guid guid,
			std::string name,
			eObjectDomain domain,
			bool isActive,
			math::Vec3<zF32> position,
			math::Quat<zF32> rotation,
			math::Vec3<zF32> scale,
			Guid meshGuid,
			Guid materialGuid,
			std::vector<Guid> scriptGuids);

		[[nodiscard]] const Guid& GetGuid() const noexcept { return m_Guid; }
		[[nodiscard]] const std::string& GetName() const noexcept { return m_Name; }
		[[nodiscard]] eObjectDomain GetDomain() const noexcept { return m_Domain; }
		[[nodiscard]] bool IsEntity() const noexcept { return m_Domain == eObjectDomain::Entity; }
		[[nodiscard]] bool IsActive() const noexcept { return m_IsActive; }

		[[nodiscard]] const math::Vec3<zF32>& GetPosition() const noexcept { return m_Position; }
		[[nodiscard]] const math::Quat<zF32>& GetRotation() const noexcept { return m_Rotation; }
		[[nodiscard]] const math::Vec3<zF32>& GetScale() const noexcept { return m_Scale; }

		[[nodiscard]] const Guid& GetMeshGuid() const noexcept { return m_MeshGuid; }
		[[nodiscard]] const Guid& GetMaterialGuid() const noexcept { return m_MaterialGuid; }
		[[nodiscard]] const std::vector<Guid>& GetScriptGuids() const noexcept { return m_ScriptGuids; }

	protected:
		[[nodiscard]] std::expected<void, std::string> Serialize(std::vector<std::byte>& buffer, const Serializer& serializer) const override;
		[[nodiscard]] std::expected<void, std::string> Deserialize(std::span<const std::byte> buffer, std::size_t& offset, const Serializer& serializer) override;

	private:
		Guid m_Guid;
		std::string m_Name;
		eObjectDomain m_Domain{ eObjectDomain::Object };
		bool m_IsActive{ true };

		math::Vec3<zF32> m_Position{ 0.0f, 0.0f, 0.0f };
		math::Quat<zF32> m_Rotation{ 0.0f, 0.0f, 0.0f, 1.0f };
		math::Vec3<zF32> m_Scale{ 1.0f, 1.0f, 1.0f };

		Guid m_MeshGuid;
		Guid m_MaterialGuid;

		std::vector<Guid> m_ScriptGuids;
	};
}
```

---

### 3.4. Обновление `SceneData`: `src/core/io/package/SceneData.h`

**Обновлено 2026-09-06 (пост-ревью после Пункта 9):** объекты сцены хранятся не плоским списком на
уровне `SceneData`, а внутри отдельного сериализуемого класса `LayerData`
(`src/core/io/package/LayerData.h/.cpp`) - имя слоя, его тип (`eLayerType`) и `std::vector<GameObjectData>`
именно этого слоя. `SceneData` хранит `std::vector<LayerData> layers` и отдаёт его через
`GetLayers()`/`SetLayers()` (`GetGameObjects()`/`SetGameObjects()` больше не существует).
`GameObjectData` не хранит `layerName`/`layerType` - эта информация лежит на `LayerData`, а не
дублируется на каждом объекте.

При десериализации сцены `SceneData` предоставляет готовые структуры `LayerData` (каждая - со своими
`GameObjectData` внутри), из которых `Scene::Initialize` заводит по одному `ILayer` на каждый `LayerData`
и сразу наполняет его объектами (см. обновлённый п.8 в `stage_09_package_resource_formats.md`).

---

### 3.5. Архитектура слоев сцены: `src/engine/scene/layer/`

#### 3.5.1. Базовый интерфейс слоя: `src/engine/scene/layer/ILayer.h`
```cpp
#pragma once

#include <string>
#include <vector>
#include "core/utils/Defines.h"

namespace zzz
{
	class ISceneStorage;

	enum class eLayerType : uint8_t
	{
		Layer3D = 0,   ///< 3D пространство (объекты мира, GameObject, свет, меши)
		LayerUI = 1,   ///< Стандартный экранный UI / HUD
		LayerMVVM = 2  ///< Авторский MVVM-фреймворк со связыванием данных и ViewModels
	};

	/**
	 * @class ILayer
	 * @brief Фундаментальный интерфейс слоя сцены.
	 */
	class ILayer
	{
	public:
		virtual ~ILayer() = default;

		[[nodiscard]] virtual const std::string& GetName() const noexcept = 0;
		[[nodiscard]] virtual eLayerType GetType() const noexcept = 0;

		[[nodiscard]] virtual bool IsVisible() const noexcept = 0;
		virtual void SetVisible(bool visible) noexcept = 0;

		[[nodiscard]] virtual bool IsEnabled() const noexcept = 0;
		virtual void SetEnabled(bool enabled) noexcept = 0;

		virtual void Update(float dt) = 0;

		[[nodiscard]] virtual ISceneStorage* GetStorage() noexcept { return nullptr; }
	};
}
```

#### 3.5.2. Реализации слоев:
1. `src/engine/scene/layer/Layer3D.h` и `.cpp`:
   - Владеет миром объектов `ObjectWorld` и пространственным хранилищем `std::unique_ptr<ISceneStorage> m_Storage` (по умолчанию `DefaultSceneStorage`).
2. `src/engine/scene/layer/LayerUI.h` и `.cpp`:
   - Слой классического игрового HUD / UI (спрайты, панели, экранный текст).
3. `src/engine/scene/layer/LayerMVVM.h` и `.cpp`:
   - Авторский UI-слой с поддержкой архитектуры MVVM (ViewModels, привязки данных, дерево элементов).

**Обновлено 2026-09-06 (пост-ревью после Пункта 9):** сигнатура наполнения слоя объектами изменилась.
Показанный выше `ILayer` из первоначального плана уже не описывает актуальный интерфейс (в фактическом
коде также есть `GetObjectWorld()`/`GetEntityWorld()` и своя логика на слой), а метод наполнения
теперь принимает не один объект, а весь `LayerData` целиком:
```cpp
virtual void Populate(
    const ::zzz::core::LayerData& layerData,
    const ::zzz::core::ScriptFactory& scriptFactory) = 0;
```
Слой сам обходит `layerData.GetObjects()` внутри своей реализации `Populate()` - `Scene::Initialize`
вызывает его один раз на слой, а не по разу на объект. `Layer3D`/`LayerUI`/`LayerMVVM` также получают
`std::shared_ptr<ResourceManager>` конструктором (а не параметром на каждый вызов наполнения) -
`Layer3D` хранит и использует его для `LoadDataAsset<MeshData>`, `LayerUI`/`LayerMVVM` пока хранят, но
не используют.

---

### 3.6. Пространственное хранилище сцены: `src/engine/scene/storage/`

#### 3.6.1. Интерфейс `ISceneStorage.h`:
```cpp
#pragma once

#include <vector>

namespace zzz
{
	class GameObject;

	/**
	 * @class ISceneStorage
	 * @brief Абстракция пространственного хранения и выборки объектов сцены.
	 */
	class ISceneStorage
	{
	public:
		virtual ~ISceneStorage() = default;

		virtual void Insert(GameObject* obj) = 0;
		virtual void Remove(GameObject* obj) = 0;
		virtual void Update(GameObject* obj) = 0;
		virtual void Clear() = 0;

		virtual void GetAll(std::vector<GameObject*>& outAll) const = 0;
	};
}
```

#### 3.6.2. Базовая реализация `DefaultSceneStorage.h` и `.cpp`:
- Хранит плоский вектор `std::vector<GameObject*> m_Objects`.
- `GetAll(...)` возвращает все зарегистрированные объекты сцены.
- Метод пространственного отсечения `QueryVisible(const Frustum&, ...)` будет добавлен строго на Шаге 27 («Продвинутые структуры пространственного хранения объектов») после реализации `Camera` (Шаг 13) и `Frustum`.

---

### 3.7. Высокопроизводительный контейнер хранения: `src/core/templates/SlotMap.h`

```cpp
#pragma once

#include <vector>
#include <span>
#include <cstdint>
#include "core/utils/Defines.h"

namespace zzz::core
{
	struct SlotHandle
	{
		uint32_t index{ UINT32_MAX };
		uint32_t generation{ 0 };

		[[nodiscard]] bool IsValid() const noexcept { return index != UINT32_MAX; }
		bool operator==(const SlotHandle&) const = default;
	};

	/**
	 * @class SlotMap
	 * @brief Плотный массив с O(1) добавлением, O(1) удалением (Swap & Pop) и стабильными Handles.
	 *
	 * @details Гарантирует непрерывную укладку активных элементов в памяти без сжатия capacity.
	 */
	template <typename T>
	class SlotMap
	{
	public:
		SlotMap() = default;
		explicit SlotMap(size_t initialCapacity);

		template <typename... Args>
		SlotHandle Emplace(Args&&... args);

		bool Remove(SlotHandle handle);
		[[nodiscard]] T* Get(SlotHandle handle) noexcept;
		[[nodiscard]] const T* Get(SlotHandle handle) const noexcept;
		[[nodiscard]] bool IsAlive(SlotHandle handle) const noexcept;

		[[nodiscard]] std::span<T> GetDenseSpan() noexcept;
		[[nodiscard]] std::span<const T> GetDenseSpan() const noexcept;
		[[nodiscard]] size_t Size() const noexcept;
		void Clear() noexcept;

	private:
		struct Slot
		{
			uint32_t denseIndex{ 0 };
			uint32_t generation{ 1 };
		};

		std::vector<T> m_Dense;
		std::vector<uint32_t> m_DenseToSparse;
		std::vector<Slot> m_Sparse;
		std::vector<uint32_t> m_FreeIndices;
	};
}
```

### 3.8. Мир объектов: `src/engine/scene/ObjectWorld.h` и `.cpp`

```cpp
#pragma once

#include <vector>
#include <memory>
#include <string>
#include "core/utils/Guid.h"
#include "core/templates/SlotMap.h"
#include "engine/scene/GameObject.h"

namespace zzz
{
	class ISceneStorage;

	/**
	 * @class ObjectWorld
	 * @brief Владелец жизненного цикла и логики игровых объектов сцены (GameObject).
	 *
	 * @details Реализует стабильное владение памятью через уникальные указатели
	 * и быстрый непрерывный доступ через плотный массив указателей SlotMap<GameObject*>.
	 */
	class ObjectWorld final
	{
	public:
		ObjectWorld();
		~ObjectWorld();

		Z_NO_COPY_MOVE(ObjectWorld);

		// --- Создание и удаление объектов ---
		GameObject* CreateObject(std::string name = "GameObject");
		GameObject* CreateObject(const ::zzz::core::Guid& guid, std::string name);
		void DestroyObject(GameObject* obj);
		void Clear();

		// --- Доступ и обход ---
		[[nodiscard]] size_t GetObjectCount() const noexcept;
		[[nodiscard]] std::span<GameObject* const> GetObjects() const noexcept;

		// --- Кадровый цикл логики ---
		void Update(float dt);

		// --- Связка с пространственным хранилищем слоя ---
		void SetStorage(ISceneStorage* storage) noexcept { m_Storage = storage; }
		[[nodiscard]] ISceneStorage* GetStorage() const noexcept { return m_Storage; }

	private:
		// Владение памятью: стабильные адреса объектов с гарантированным O(1) удалением
		std::unordered_map<GameObject*, std::unique_ptr<GameObject>> m_AllocatedObjects;

		// Плотный пул активных указателей для кэш-локального обхода Update()
		::zzz::core::SlotMap<GameObject*> m_ActiveObjects;

		// Указатель на пространственное хранилище слоя (для регистрации/дерегистрации)
		ISceneStorage* m_Storage{ nullptr };
	};
}
```

---

## 4. План верификации

1. **Компиляция под MSVC x64 + Ninja:**
   - Сборка целей `core`, `engine`, `EngineTests`, `game_win` (0 ошибок, 0 предупреждений).
2. **Верификация функционала `Transform`, `SlotMap` и `ObjectWorld`:**
   - Проверка корректности пересчета мировой матрицы $TRS$ через `GetWorldMatrix()`.
   - Проверка каскадной инвалидации `dirty flag` при изменении родительского `Transform`.
   - Проверка добавления, удаления через Swap & Pop и стабильности `SlotHandle` в `SlotMap<T>`.
   - Проверка создания, обхода и удаления `GameObject` в `ObjectWorld`.
3. **Сквозной запуск `game_win.exe`:**
   - Создание корневого `GameObject` куба с `Transform` в `Layer3D` через `ObjectWorld`, с привязанными `meshGuid` и `materialGuid`.
   - Чистый запуск и завершение процесса с кодом 0.

---

## 5. Чек-лист Definition of Done (DoD)

- [x] Создать шаблонный контейнер `src/core/templates/SlotMap.h` (Dense/Sparse пул с Swap & Pop)
- [x] Создать `src/engine/scene/Transform.h` и `Transform.cpp` (с ленивым $TRS$ и dirty-флагом)
- [x] Выполнить рефакторинг `src/engine/scene/GameObject.h` и `GameObject.cpp` (GUID, Transform, слоты `meshGuid`/`materialGuid`)
- [x] Создать `src/engine/scene/ObjectWorld.h` и `ObjectWorld.cpp` (владелец объектов и SlotMap<GameObject*>)
- [x] Создать `src/engine/scene/storage/ISceneStorage.h` и `DefaultSceneStorage.h` / `.cpp`
- [x] Создать систему слоев сцены `src/engine/scene/layer/`:
  - `ILayer.h` (базовый интерфейс)
  - `Layer3D.h` / `.cpp` (3D мир с `DefaultSceneStorage` и `ObjectWorld`)
  - `LayerUI.h` / `.cpp` (HUD / Screen UI)
  - `LayerMVVM.h` / `.cpp` (каркас авторского MVVM-фреймворка)
- [x] Создать `src/core/io/package/GameObjectData.h` и `GameObjectData.cpp` (сериализация сущности)
- [x] Обновить `src/core/io/package/SceneData.h` для хранения объектов сцены (2026-09-06: пост-ревью
  перевело хранение на `std::vector<LayerData>` через `LayerData.h/.cpp` вместо плоского
  `std::vector<GameObjectData>` - см. обновлённый п.3.4 выше)
- [x] Интегрировать слои в `src/engine/scene/Scene.h` и `Scene.cpp`
- [x] Зарегистрировать новые файлы в `src/core/CMakeLists.txt` и `src/engine/CMakeLists.txt`
- [x] Собрать `EngineTests.exe` и `game_win` под MSVC + Ninja (чистая сборка)
- [x] Запустить `game_win.exe` (чистый запуск, создание тестового GameObject с Transform в Layer3D, завершение)
- [ ] Запросить утверждение у пользователя
- [ ] Зафиксировать Git-коммит: `feat(scene): completed stage 07 - GameObject, Transform, SlotMap, ObjectWorld, Layers and SceneStorage`
- [ ] Обновить статус Пункта 7 в `general_plan.md` на `✅ Выполнено`
