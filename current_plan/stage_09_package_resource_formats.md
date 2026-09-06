# Этап 09: Сквозной конвейер сетки (`MeshData`), парсинг `.obj`, упаковка в `PackagePacker` и десериализация

## 1. Контекст и цели этапа
- **Номер пункта:** **Пункт 9** (Уровень 2: GAPI-ресурсы, содержимое куба и сквозной рендер).
- **Цель:** Реализовать сквозную вертикаль для геометрии (Mesh):
  1. Создать канонический бинарный формат `MeshData` в `src/core/io/package/` с симметричной сериализацией и десериализацией через `Serializer`.
  2. Добавить парсер стандартного 3D-формата `.obj` (Wavefront OBJ) в `assets_builder_dll` (никаких промежуточных кастомных форматов вроде `.zmsh` — работаем со стандартным форматом).
  3. Расширить `PackagePacker` для импорта `.obj` с чтением `.obj.meta` и упаковки в бинарный архив игровых ресурсов `assets/data/data.dat`.
  4. Добавить в тестовый проект ассетов папку `Assets/Meshes/` с моделью единичного куба со сторонами 2 (координаты от -1.0 до +1.0, опорная точка/центр в `(0, 0, 0)`) и метафайлом `cube_00.obj.meta`.
  5. Проверить сквозную цепочку: чтение `.obj` $\to$ конвертация в `MeshData` $\to$ бинарная упаковка в `data.dat` $\to$ чтение и десериализация движком через `DataAssetsManager` при старте сцены.
- **Статус:** `🔄 В работе`.
- **Зависимости:** `src/core/serialize/Serializer.h`, `src/core/enums/eResourceType.h`, `src/core/enums/eIndexFormat.h`, `src/core/io/package/PackageEntry.h`, `src/tools/assets_builder/assets_builder_dll/PackagePacker.h`.

---

## 2. Архитектурные принципы

1. **Честный конвейер ассетов с первого шага (No Procedural Hacks):**
   - Никакого процедурного создания вершин куба в коде ядра.
   - Меш куба создаётся как стандартный `.obj`, лежит в папке `Assets/Meshes/`, упаковывается `PackagePacker` в `data.dat` и считывается движком.
2. **Отказ от промежуточных/кастомных форматов:**
   - Отказываемся от `.zmsh` (JSON-сеток). Входной файл — чистый стандартный `.obj`, который может быть экспортирован напрямую из Blender/DCC.
3. **Разделение пакетов архивов и стриминговых ресурсов:**
   - **`assets/package.dat`** (Скелет и структура игры): манифест проекта (`project.json`), окна/конфиги (`.zav`, `.zcv`, `.ziv`), сцены (`.zs`) со структурой `GameObject` и GUID-ссылками на ресурсы.
   - **`assets/data/data.dat`** (Игровые ресурсы): префабы (`.zp`), меши (`MeshData`), материалы (`MaterialData`), шейдеры (`ShaderData`), анимации. Чтение выполняется диапазоном байт (`offset` + `size`) через единый I/O-поток `ResourceManager`.
   - **Подпапки стриминга (`assets/data/<subdir>/`):** текстуры (`textures/`), звук (`audio/`), видео (`video/`), шрифты (`fonts/`) хранятся отдельными файлами и читаются/стримятся целиком или поблочно.
4. **Единый источник правды о типах и хранении в `core`:**
   - Единый глобальный enum `eResourceType` (в `core/enums/eResourceType.h`) покрывает все типы контента (`ProjectManifest`, `Scene`, `PrimaryView`, `ChildView`, `IndependentView`, `Prefab`, `Mesh`, `Material`, `Shader`, `Animation`, `Texture2D`, `AudioClip`, `Video`, `Font`).
   - Низкоуровневая структура `PackageEntry` хранит тип ресурса как сырой `zU32 assetType`. Метод `PackageEntry::LogFileBlock()` выводит `type` как сырое число (`assetType`), не делая предположений об enum-е, что гарантирует универсальность и отсутствие конфликтов для обоих архивов (`package.dat` и `data.dat`). Семантическое логирование имени типа выполняют сами менеджеры архивов (`PackageManager` и `DataAssetsManager`).
   - Мета-реестр свойств хранения `ResourceStorageTraits` (в `core/io/ResourceStorageTraits.h`) связывает каждый `eResourceType` с его способом хранения (`PackageArchive`, `DataArchive`, `DedicatedFolder`) и относительным каталогом. И сборщик, и движок используют этот контракт как Single Source of Truth.
5. **Разделение ответственности между `core` и `assets_builder_dll`:**
   - **`core` (Рантайм-ядро):**
     - Структура данных `MeshData` в памяти (число вершин/индексов, stride, байтовые буферы вершин и индексов, формат индексов).
     - Симметричная бинарная сериализация и десериализация (`ISerializable`: `Serialize` в буфер пакета / `Deserialize` из пакета через `Serializer`).
     - `DataAssetsManager`: read-only таблица записей `data.dat`, потокобезопасное чтение ресурсов (`LoadDataByGuid<T>`).
     - Не содержит парсеров `.obj` или других форматов авторинга.
   - **`assets_builder_dll` (Тулчейн / Упаковщик / Редактор):**
     - Чтение и парсинг `.obj` файлов своими методами (парсинг `v`, `vt`, `vn`, `f`, сведение в вершины `Vertex3D` и индексный буфер).
     - Заполнение структуры `MeshData` из `core`.
     - Запись в соответствующий архив (`data.dat` или `package.dat`) согласно `ResourceStorageTraits`.
6. **Мета-файлы с GUID (`<filename>.meta`):**
   - У файла `cube_00.obj` есть парный `cube_00.obj.meta`:
     ```json
     {
       "guid": "00000000-0000-0000-0000-000000000010"
     }
     ```
   - `PackagePacker` считывает этот `guid` и прописывает его в заголовок `PackageEntry`.
7. **Структура `GameObject` в слоях сцены (`MainScene.zs`) и маршрутизация доменов:**
   - Объекты сцены сгруппированы по слоям (`Layer3D`, `LayerUI` и др.).
   - Куб располагается в слое `Layer3D` строго в начале координат `position: [0.0, 0.0, 0.0]`.
   - В JSON сцены хранится тип/домен объекта (`"domain": "Object"`) и опциональные блоки настроек (`"render": { "mesh": "..." }`, массив скриптов `"scripts": [...]` — на одном объекте может быть несколько скриптов, хранящихся и загружаемых списком GUID).
   - Единый глобальный enum `eObjectDomain` выносится в `src/core/enums/eObjectDomain.h` с функциями `ToString(eObjectDomain)` и `ParseObjectDomain(std::string_view)`. Константы строковых полей JSON объявляются в `PackageConstants.h`. C# валидатор `SceneAssetValidator.cs` валидирует допустимость значений `domain` ('Object' или 'Entity').
   - Маршрутизация при загрузке сцены:
     - `domain == eObjectDomain::Object` $\to$ создаётся в `ObjectWorld` целевого слоя текущей сцены.
     - `domain == eObjectDomain::Entity` $\to$ регистрируется в классе-заглушке `EntityWorld` сцены (`CreateEntity`), обеспечивая сквозную цепочку от упаковки в `package.dat` до чтения движком без исключений.
     - При отсутствии блока `"render"` объект создаётся как узел трансформации (Empty).
8. **Архитектура разбора слоёв (Инкапсуляция разборщика в реализации слоя):**
   - Слои (`Layer3D`, `LayerUI`, `LayerMVVM`) принципиально отличаются составом, семантикой данных и поведением.
   - Разбор и наполнение конкретного слоя инкапсулируются в самом слое. **Обновлено 2026-09-06
     (пост-ревью):** сигнатура - `ILayer::Populate(const LayerData& layerData, const ScriptFactory&)` /
     `Layer3D::Populate(...)` и т.д. - слой получает весь `LayerData` (имя, тип, свои объекты) одним
     вызовом и сам обходит `layerData.GetObjects()`, а не разбирает объекты по одному через отдельный
     метод на объект (см. `LayerData` в п.3.4 `stage_07_gameobject_and_transform.md`).
   - `Layer3D` инкапсулирует: создание `GameObject` в своём `ObjectWorld`, настройку трансформаций, скриптов и вычитку `MeshData` через `DataAssetsManager`.
   - `Scene` не знает о внутреннем устройстве конкретных слоёв: она лишь определяет/фабрикует слой по типу и делегирует ему наполнение объектом.
9. **Потоковая модель загрузки на Этапе 09:**
   - `DataAssetsManager` инициализируется при старте движка (таблица `Guid` read-only).
   - Чтение `MeshData` потокобезопасно (независимое открытие/чтение смещений файлового потока).
   - На этапе 09 вычитка `MeshData` куба валидируется при открытии сцены через `DataAssetsManager`, обеспечивая детерминированную проверку сквозного конвейера до создания `ResourceManager` на Этапе 10.
10. **Контракт параметров переходов сцен (`transition`) и валидация:**
    - `PackagePacker` считывает секции `transition` из `project.json` (дефолтные глобальные параметры `SceneTransitionParams`) и `.zs` (параметры конкретной сцены и `transitionSource`).
    - C# валидаторы (`SceneAssetValidator`, `ProjectJsonValidator`) валидируют объекты во всех слоях (`layers[].objects[]`), проверяют валидность GUID мешей (`render.mesh`), а также типы и диапазоны параметров переходов (`type`, `duration`).
11. **Строгая фильтрация расширений ресурсов и отсечение неподдерживаемых файлов (Whitelist):**
    - `AssetFileExtensions.h` и `AssetImporterRegistry` выступают единым источником истины в C++ ядра движка.
    - В `assets_builder_dll` экспортируется нативная функция `IsSupportedAssetExtension(ext)` (`BuilderApi.h`).
    - В `assets_builder_lib` константы `AssetExtensions.cs` синхронизированы с `AssetFileExtensions.h`.
    - Всеядные fallback-классы `DefaultAssetImporter` и `DefaultAssetValidator` удалены.
    - Добавлены специализированные `DataAssetImporter` и `DataAssetValidator` для разрешённых типов (`.obj`, `.png`, `.zmat`, `.hlsl`, `.zp`).
    - Вьюшка первичного окна `.zav` (`PrimaryView`) поддержана в `ViewAssetImporter` и `ViewAssetValidator`.
    - Сканирование `Assets/` и `Assets/Scripts/` отфильтровывает любые файлы не из белого списка с выводом предупреждения в лог сборщика и предотвращением создания `.meta` и GUID для посторонних файлов.
    - Очистка осиротевших `.meta` удаляет метафайлы, если целевой ресурс не существует или его тип не поддерживается.
12. **Формат имени ассета в таблицах архивов (`FixedLengthString32`, UTF-32) и расчёт бинарного размера:**
    - Имена ресурсов в `PackageEntry` хранятся в формате `FixedLengthString32<c_MaxAssetNameLength>` (`char32_t`, UTF-32).
    - Это обеспечивает равное количество символов во всех языках (64 символа, где каждый символ — ровно 4 байта, итого 256 байт на имя).
    - В упаковщике (`ArchiveWriter.cpp`) запрещено тихое усечение: имя валидируется методом `Create`, при превышении лимита логируется ошибка `DOutError` и функция возвращает `false` (не `THROW_RUNTIME` — исключение не должно пересекать границу P/Invoke до C#).
    - Бинарный размер `PackageEntry::BinarySize()` вычисляется динамически из суммы бинарных размеров полей с явным вызовом `Guid::BinarySize()` (16 байт), исключая оверхед указателя виртуальной таблицы `vptr` (8 байт), предотвращая выход за границы буфера при чтении чанков.

---

## 3. Архитектурная спецификация

### 3.1. Структура геометрии: `src/core/io/package/MeshData.h`

```cpp
#pragma once

#include <vector>
#include <cstddef>
#include <expected>
#include <string>
#include <span>
#include "core/serialize/Serializer.h"
#include "core/enums/eIndexFormat.h"

namespace zzz::core
{
	/**
	 * @class MeshData
	 * @brief Сериализуемый бинарный контейнер геометрии меша в data.dat.
	 */
	class MeshData final : public ISerializable
	{
	public:
		MeshData() = default;
		MeshData(
			uint32_t vertexCount,
			uint32_t vertexStride,
			std::vector<std::byte> vertexData,
			uint32_t indexCount,
			eIndexFormat indexFormat,
			std::vector<std::byte> indexData);

		[[nodiscard]] uint32_t GetVertexCount() const noexcept { return m_VertexCount; }
		[[nodiscard]] uint32_t GetVertexStride() const noexcept { return m_VertexStride; }
		[[nodiscard]] const std::vector<std::byte>& GetVertexData() const noexcept { return m_VertexData; }

		[[nodiscard]] uint32_t GetIndexCount() const noexcept { return m_IndexCount; }
		[[nodiscard]] eIndexFormat GetIndexFormat() const noexcept { return m_IndexFormat; }
		[[nodiscard]] const std::vector<std::byte>& GetIndexData() const noexcept { return m_IndexData; }

	protected:
		[[nodiscard]] std::expected<void, std::string> Serialize(std::vector<std::byte>& buffer, const Serializer& serializer) const override;
		[[nodiscard]] std::expected<void, std::string> Deserialize(std::span<const std::byte> buffer, std::size_t& offset, const Serializer& serializer) override;

	private:
		uint32_t m_VertexCount{ 0 };
		uint32_t m_VertexStride{ 0 };
		std::vector<std::byte> m_VertexData;

		uint32_t m_IndexCount{ 0 };
		eIndexFormat m_IndexFormat{ eIndexFormat::UInt16 };
		std::vector<std::byte> m_IndexData;
	};
}
```

---

### 3.2. Формат архива и менеджер игровых данных: `src/core/io/package/DataAssetsManager.h`

1. **Формат и сигнатура архива `assets/data/data.dat` (`src/core/constants/PackageConstants.h`):**
   - Сигнатура Magic Bytes: **`"ZZD"`** (`c_DataPackageHeader`, 3 байта).
   - Версия формата данных (отдельные константы):
     ```cpp
     constexpr zU8 c_DataPackageFileMajorVersion = 1;
     constexpr zU8 c_DataPackageFileMinorVersion = 0;
     constexpr zU8 c_DataPackageFilePatchVersion = 0;
     ```
   - Заголовок архива: `PackageHeader` (`"ZZD"`, Version 1.0.0, `entriesCount`).
   - Таблица оглавления: массив `PackageEntry` (`name`, `guid`, `assetType` = `eResourceType`, `offset`, `size`).
   - Данные: последовательные бинарные блоки ресурсов (`MeshData` и др.).

2. **Контракт `DataAssetsManager` (`src/core/io/package/DataAssetsManager.h`):**
   - Отвечает строго за чтение архива игровых ресурсов **`assets/data/data.dat`** (меши, материалы, шейдеры, префабы).
   - **Обязателен при старте:** если файл `assets/data/data.dat` отсутствует на диске или повреждён — выбрасывает `THROW_RUNTIME("Отсутствует обязательный архив игровых ресурсов: assets/data/data.dat")`.
   - Индексирует заголовки `PackageEntry` архива `data.dat` в быструю хэш-таблицу `m_EntriesByGuid` (read-only после монтирования).
   - Предоставляет строгий шаблонный метод загрузки по паре **`(eResourceType, Guid)`**:
     ```cpp
     template <typename T>
     [[nodiscard]] std::expected<T, std::string> LoadData(eResourceType expectedType, Guid guid);
     ```
   - **Инварианты валидации через `ensure`:**
     - Проверка, что `expectedType` поддерживается архивом `data.dat` (`Mesh`, `Prefab`, `Material`, `Shader`, `Animation`);
     - Поиск по `guid` в таблице записей;
     - Проверка совпадения фактического типа записи `it->second.GetAssetType()` с запрошенным `expectedType`.
   - В будущем (Этап 10) передаётся в `ResourceManager` как подсистема прямого чтения ресурсов из `data.dat`.

---

### 3.3. Архитектура импортёров, константы расширений и реестр (`src/tools/assets_builder/assets_builder_dll/`)

1. **Константы расширений файлов (`src/core/io/AssetFileExtensions.h`):**
   - `.meta`, `.obj`, `.png`, `.zmat`, `.hlsl`, `.zs`, `.zp`, `.zav`, `.zcv`, `.ziv`.
2. **Свойства хранения (`src/core/io/ResourceStorageTraits.h`):**
   - Связывает `eResourceType` с архивом назначения (`PackageArchive` $\to$ `package.dat`, `DataArchive` $\to$ `data.dat`, `DedicatedFolder` $\to$ подпапки) и относительными каталогами.
3. **Интерфейс импортёра (`IAssetImporter.h`) и Реестр (`AssetImporterRegistry.h/.cpp`):**
   - Диспетчеризация файлов по расширению при сканировании проекта.
4. **Парсер `ObjImporter` (C++):**
   - Читает `.obj` $\to$ дедуплицирует в `Vertex3D` (32 байта, CW порядок обхода, опорная точка в начале координат $(0,0,0)$) $\to$ `UInt16` индексы $\to$ `MeshData` $\to$ бинарный блок данных.
5. **Упаковка в `PackagePacker.cpp`:**
   - Формирует два архива: `package.dat` (манифест, сцены, вьюхи) и `data.dat` (меши, префабы).

---

### 3.4. Исходный ассет куба и сцена в `zzz_assets_test_000`

1. **Ассет куба:**
   - Путь: `src/projects/assets_projects/zzz_assets_test_000/Assets/Meshes/cube_00.obj` (сторона 2, центр $(0,0,0)$, 24 вершины, 36 индексов CW).
   - Метафайл: `cube_00.obj.meta` (`guid: "00000000-0000-0000-0000-000000000010"`).
2. **Сцена `MainScene.zs`:**
   - Куб располагается в слое `Layer3D` в начале координат:
     ```json
     {
       "name": "MainScene",
       "version": "1.0.0",
       "layers": [
         {
           "type": "Layer3D",
           "name": "Main3DLayer",
           "objects": [
             {
               "name": "CubeObject",
               "domain": "Object",
               "position": [0.0, 0.0, 0.0],
               "scripts": [
                 "6ea9c238-4c23-4b73-be36-8fed508ea612"
               ],
               "render": {
                 "mesh": "00000000-0000-0000-0000-000000000010"
               }
             }
           ]
         }
       ]
     }
     ```

### 3.5. Класс-заглушка `EntityWorld`: `src/engine/scene/EntityWorld.h`

```cpp
#pragma once

#include <string_view>
#include <vector>
#include <core/utils/Guid.h>
#include <core/utils/Defines.h>

namespace zzz::engine
{
	/**
	 * @class EntityWorld
	 * @brief Легковесная заглушка мира ECS-сущностей для сквозной цепочки domain == Entity.
	 */
	class EntityWorld final
	{
	public:
		EntityWorld() = default;
		~EntityWorld() = default;

		Z_NO_COPY_MOVE(EntityWorld);

		void CreateEntity(const core::Guid& guid, std::string_view name);
		void Update(float dt);

		[[nodiscard]] size_t GetEntityCount() const noexcept { return m_EntityCount; }

	private:
		size_t m_EntityCount{ 0 };
	};
}
```

---

## 4. План верификации

1. **Компиляция под MSVC x64 + Ninja:**
   - Сборка целей `core`, `engine_lib`, `assets_builder_dll`, `EngineTests`, `game_win` без ошибок и предупреждений.
2. **Сборка пакетов:**
   - `PackagePacker::PackProject()` успешно создаёт `assets/package.dat` и `assets/data/data.dat`.
3. **Сквозная проверка через запуск приложения `game_win.exe`:**
   - При старте `DataAssetsManager` открывает `assets/data/data.dat` (валидирует сигнатуру `ZZD` и таблицу записей);
   - `SceneManager` загружает `MainScene`;
   - Движок извлекает `CubeObject` из `Layer3D`, находит `meshGuid` (`00000000-0000-0000-0000-000000000010`), запрашивает его у `DataAssetsManager` и десериализует `MeshData`;
   - Проверяется: `vertexCount == 24`, `indexCount == 36`;
   - Завершение работы с кодом 0, логирование успешной загрузки меша.

---

## 5. Чек-лист Definition of Done (DoD)

- [x] Создать `src/core/io/AssetFileExtensions.h` и `src/core/io/ResourceStorageTraits.h`
- [x] Унифицировать `src/core/enums/eResourceType.h` как единый глобальный enum контента
- [x] Создать `src/core/io/package/MeshData.h` и `src/core/io/package/MeshData.cpp`
- [x] Создать `src/core/io/package/DataAssetsManager.h` и `src/core/io/package/DataAssetsManager.cpp` (с обязательной проверкой наличия `data.dat` и выбросом `THROW_RUNTIME`)
- [x] Зарегистрировать новые файлы в `src/core/CMakeLists.txt`
- [x] Создать `IAssetImporter.h` и `AssetImporterRegistry.h/.cpp` в `assets_builder_dll`
- [x] Реализовать `ObjImporter.h/.cpp` в `assets_builder_dll`
- [x] Обновить чтение сцены в `PackagePacker.cpp`: парсинг слоёв и объектов `GameObjectData` (позиция, domain, render.mesh) в `SceneData` (2026-09-06: пост-ревью перевело результат парсинга на `std::vector<LayerData>` - каждый слой из JSON, включая неявный `"Default3DLayer"` из верхнеуровневых `"objects"`, собирается в свой `LayerData` со своими объектами внутри, а не в общий плоский список)
- [x] Обновить `PackagePacker.cpp` для генерации обоих архивов: `package.dat` и `data.dat`
- [x] Поддержать чтение и сериализацию параметров переходов (`transition`) в `PackagePacker.cpp`
- [x] Обновить C# валидаторы (`SceneAssetValidator.cs`, `ProjectJsonValidator.cs`): обход `layers[].objects[]`, валидация `render.mesh` и `transition`
- [x] Создать `Assets/Meshes/cube_00.obj` и `cube_00.obj.meta` в `zzz_assets_test_000`
- [x] Обновить `MainScene.zs` слоем `Layer3D` и объектом `CubeObject` с блоком `render` в `(0,0,0)`
- [x] Передать структуру сцены в `Scene` при загрузке (2026-09-06: `SceneData::GetGameObjects()`
  заменён на `SceneData::GetLayers()` - `Scene::Initialize` заводит `ILayer` на каждый `LayerData` и
  наполняет его целиком через `ILayer::Populate(layerData, scriptFactory)`)
- [x] Интегрировать проверку загрузки `MeshData` из `DataAssetsManager` в запуск `game_win.exe`
- [x] Собрать проект и успешно прогнать `game_win.exe` (код выхода 0)
- [x] Создать `src/core/enums/eObjectDomain.h` (`ToString`, `ParseObjectDomain`) и зарегистрировать в `src/core/CMakeLists.txt`
- [x] Добавить константы домена в `PackageConstants.h` и валидацию `domain` в `SceneAssetValidator.cs`
- [x] Обновить `GameObjectData.h` и `PackagePacker.cpp` на использование `eObjectDomain.h`
- [x] Создать класс-заглушку `src/engine/scene/EntityWorld.h` и `EntityWorld.cpp`
- [x] Зарегистрировать `EntityWorld` в `src/engine/CMakeLists.txt`
- [x] Интегрировать `EntityWorld` в `Scene.h` и `Scene.cpp` (маршрутизация `objData.IsEntity()`)
- [x] Реализовать `FixedLengthString32` (UTF-32, `char32_t`) для равного количества символов во всех языках (64 символа = 256 байт)
- [x] Добавить `Guid::BinarySize()` (16 байт) и использовать в `PackageEntry::BinarySize()` без `sizeof(Guid)`
- [x] Добавить логирование ошибки (`DOutError`) и возврат `false` при переполнении длины имени в `ArchiveWriter.cpp`
- [x] Экспорт `IsSupportedAssetExtension` в `assets_builder_dll` (`BuilderApi.h`, `BuilderApi.cpp`)
- [x] Расширение `AssetExtensions.cs` и P/Invoke `NativeMethods.cs` строгим белым списком
- [x] Поддержка `.zav` в `ViewAssetImporter.cs` и `ViewAssetValidator.cs`
- [x] Создать `DataAssetImporter.cs` и `DataAssetValidator.cs`
- [x] Удалить всеядный fallback `DefaultAssetImporter.cs` и `DefaultAssetValidator.cs`
- [x] Интегрировать фильтрацию по расширениям и очистку неподдерживаемых `.meta` в `AssetsBuilderEngine.cs`
- [ ] Зафиксировать Git-коммит
- [x] Обновить статус Пункта 9 в `general_plan.md` на `✅ Выполнено`

