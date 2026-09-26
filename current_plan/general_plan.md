# Генеральный план разработки: Вращающийся текстурированный 3D-куб (Textured Spinning Cube)

---

## 📜 Правила ведения и исполнения плана (Обязательно к соблюдению)

> [!IMPORTANT]
> Все работы, планирование этапов, архитектурные инварианты, потоковая модель и критерии приёмки (DoD) регламентируются единым сводом правил:
> 👉 **[RULES.md](RULES.md)**
>
> **Обязательно к исполнению перед любым планированием:**
> 1. Внимательно прочитать и соблюдать все правила из **[RULES.md](RULES.md)**;
> 2. Провести обязательный аудит существующих примитивов в `src/core/templates/` (`CallbackQueue`, `DoubleBufferedVector`, `SwapQueue`, `QueueArray`, `SlotMap`, `ThreadPool`, `CountdownTrigger` и др.) перед проектированием любых синхронизаций, очередей или контейнеров (Правило 4.1).
>
> Свод правил включает 7 блоков:
> 1. **Процесс разработки, этапы и верификация (Workflow & DoD)** (Правила 1–5);
> 2. **Архитектура модулей, владение кодом и C++ стандарт** (Правила 6–8.3);
> 3. **Кроссплатформенность и изоляция платформ (Zero Platform Leaks)** (Правила 9–12);
> 4. **Производительность, Concurrency и Data-Oriented Design (DOD)** (Правила 13–15);
> 5. **Минимализм API и соблюдение YAGNI** (Правило 16);
> 6. **Форматы данных, версионирование и QA-инфраструктура** (Правила 17–19);
> 7. **Лицензирование сторонних библиотек и юридическая чистота** (Правило 20).

---

## 📚 Референсный проект с примерами реализации
> **Путь к референсному проекту:** [`c:\Workspaces\DZzz`](file:///c:/Workspaces/DZzz)  
> Содержит примеры шаблонного буфера вершин (`CPUVertexBuffer.cppm`), маппера форматов (`VertexFormatMapper.cppm`) и другие реализации подсистем Zzz.

---

## 🎯 Финальная цель проекта
Обеспечить полный цикл работы графической и ресурсной подсистем ZzzEngine: от добавления исходных ассетов в проект до отображения и **вращения текстурированного 3D-куба пользовательским скриптом** в окне приложения через кроссплатформенный GAPI (DirectX 12 / Vulkan / Metal).  
Целевая кадровая схема: **CPU готовит слот кадра $N$, пока ранее подготовленный слот $N-1$ отправляется на GPU и выводится на экран**. Это номера кадров, не потоков; доступ к сценовым буферам и повторное использование GPU-памяти регулируются разными барьерами (правило 14).

> **💡 Главная миссия Special GUI Layer (ZzzGUI в `gui_lib`):** Создание высокопроизводительного автономного MVVM UI-фреймворка на нативном GAPI с возможностью прямого бесшовного встраивания интерактивных **3D объектов и GPU-эффектов (частицы, свечение, Glass/Acrylic блюр) прямо в элементы интерфейса** с On-Demand рендерингом десктопного уровня.


## 🧭 Приоритеты MVP (уточнено пользователем, 2026-08-31)

1. **Критерий приёмки контрольной точки — проверенное поведение на платформах, уже включённых в роллаут.** Сначала Windows DX12/Vulkan, затем Linux, затем Apple/mobile. Остальные платформы остаются целями, но их отсутствие не блокирует каждый ранний этап; непроверенные конфигурации отмечаются явно.
2. **ZzzGUI (кроссплатформенный WPF-подобный UI-фреймворк) важнее полноты 3D-графики.** При конфликте приоритетов между «доделать графический эффект» и «довести GUI до одинаковой работы на всех платформах» — приоритет у GUI.
3. **Графика на первом проходе — минимально достаточная, а не максимально красивая.** Технологии, не критичные прямо сейчас (конкретная реализация шрифтов, блюр/частицы, переходы сцен), не фиксируются заранее — решаются в момент подхода к пункту (правило 16). Ничего не исключается из плана заранее без явного решения пользователя.

## 🗺️ Платформенный роллаут (уточнено пользователем, 2026-08-31)

Платформы вводятся по очереди, не все сразу — со своими контрольными точками (КП) в таблице ниже:

1. **Windows** (DirectX12 + Vulkan) — основная платформа разработки; все КП-1..КП-3 проверяются здесь.
2. **Linux** (Vulkan) — следующая цель сразу после подтверждения куба и ZzzGUI-MVP на Windows (КП-4).
3. **macOS и мобильные (Android/iOS, Metal)** — позже (КП-5). *Предположение: macOS сгруппирован с мобильными платформами, т.к. явно не назван пользователем отдельно и разделяет бэкенд Metal с iOS — уточнить при необходимости.*

---

### 📌 Текущее состояние разработки

> [!IMPORTANT]
> **Текущий активный пункт:** `Пункт 25. Библиотека обработки и сжатия текстур (texture_processor_lib)`
>
> **Статус:** ⏳ В процессе
> **Список открытых сквозных задач / технического долга:** [`docs/ARCHITECTURE.md`](../docs/ARCHITECTURE.md) §4  
> **Текущая подзадача:** Выделенная статическая библиотека для декодирования (PNG, JPG, TGA, BMP, HDR через stb_image), валидации кратности 2, генерации mipmap-уровней (stb_image_resize2 sRGB) и компрессии в GPU-форматы (BC7/BC5/BC4 через DirectXTex для Desktop, ASTC для Mobile). Формирование бинарного блоба с `TextureBlobHeader` (64 байта).
> 
---

## 📋 Сквозная таблица этапов реализации (Строгая последовательность зависимостей)

> 💡 **Примечание к выполненным этапам:**<br/>
> По завершённым пунктам (1–15, 17, 18, 20–24) в таблице генплана оставлены только номер, название, статус и ссылка на файл этапа.<br/>
> **Вся исчерпывающая техническая спецификация, сигнатуры, структуры, архитектурные решения и чек-листы бережно сохранены в файлах `current_plan/stage_XX_...md`**. Они будут использованы в этапе 64 для формирования итоговой архитектурной документации в `docs/`.

### Уровень 1: Математический, файловый и платформенный фундамент
| № | Этап разработки | Статус | Документ этапа |
|---|---|---|---|
| **1** | Векторная математика (`Vec2`, `Vec3`, `Vec4`) и унификация `Point2D` | ✅ Выполнено | [`stage_01_vectors.md`](stage_01_vectors.md) |
| **2** | Матрицы `Mat4` и `Mat3` (аффинные трансформации, базис, проекции) | ✅ Выполнено | [`stage_02_mat4.md`](stage_02_mat4.md) |
| **3** | Кватернионы `Quat` (3D вращения, `Slerp`, углы Эйлера) | ✅ Выполнено | [`stage_03_quat.md`](stage_03_quat.md) |
| **4** | Базовые перечисления (Enums) и структуры ядра (`Vertex3D`, CBV и др.) | ✅ Выполнено | [`stage_04_enums_and_core_structs.md`](stage_04_enums_and_core_structs.md) |
| **5** | Кроссплатформенная файловая система (`FileSystem`, `eFileLocation`) | ✅ Выполнено | [`stage_05_filesystem.md`](stage_05_filesystem.md) |
| **6** | Рефакторинг подсистемы аппаратуры (`HardwareManager`, `HardwareState`) | ✅ Выполнено | [`stage_06_hardware_manager.md`](stage_06_hardware_manager.md) |

### Уровень 2: GAPI-ресурсы, содержимое куба и сквозной рендер
| № | Этап разработки | Статус | Документ этапа | Краткое описание / Задачи |
|---|---|---|---|---|
| **7** | Исходная модель GameObject, Transform, SlotMap и слоёв | ✅ Выполнено | [`stage_07_gameobject_and_transform.md`](stage_07_gameobject_and_transform.md) | Историческая основа; Transform и иерархия заменены в этапах 12–15 |
| **8** | Конвейер смены сцен, асинхронная загрузка и переходы | ✅ Выполнено | [`stage_08_scene_transition_pipeline.md`](stage_08_scene_transition_pipeline.md) | SceneTransitionParams, View::SetScene, CallbackQueue |
| **9** | CPU-конвейер MeshData, OBJ и хранилища ресурсов | ✅ Выполнено | [`stage_09_package_resource_formats.md`](stage_09_package_resource_formats.md) | MeshData, OBJ-парсер, хранилища package.dat / data.dat / файлы |
| **10** | Менеджер проектов Сборщика Ассетов (CLI, пресеты) | ⏳ В процессе | [`stage_10_assets_builder_presets_and_scaffolding.md`](stage_10_assets_builder_presets_and_scaffolding.md) | Пресеты и конфиги реализованы; приёмка сценариев открыта до КП-1 (этап 28) |
| **11** | Каркас ResourceManager и ResourceGarbageCollector | ✅ Выполнено | [`stage_11_resource_manager.md`](stage_11_resource_manager.md) | IResource, таблицы кэшей, I/O-поток, GC; загрузчики и GPU-готовность доводятся в этапах 16, 20, 25 |
| **12** | Базовые доменные интерфейсы и хранилище Transform | ✅ Выполнено | [`stage_12_domain_interfaces_and_spatial_storage.md`](stage_12_domain_interfaces_and_spatial_storage.md) | Layer2D, ILayerDomain, ISpatialStorage, DefaultSpatialStorage |
| **13** | 64-арное битовое дерево изменений (`BitTreeTracker`) | ✅ Выполнено | [`stage_13_bit_tree_tracker.md`](stage_13_bit_tree_tracker.md) | Иерархический битовый трекер грязных узлов для сцены |
| **14** | SoA-узлы сцены и SceneTreeContainer, удаление Transform | ✅ Выполнено | [`stage_14_scene_node_and_tree_container.md`](stage_14_scene_node_and_tree_container.md) | Плоский SoA-контейнер иерархии, Primary/Secondary, DestroySubtree |
| **15** | Интеграция слоёв, кадра и Populate | ✅ Выполнено | [`stage_15_layer_integration_and_populate.md`](stage_15_layer_integration_and_populate.md) | SpatialLayer, LayerMVVM, Handover Barrier, Populate |
| **16** | ResourceManager, Mesh и подготовка к загрузке в GPU | ✅ Выполнено | [`stage_16_mesh_and_gpu_upload.md`](stage_16_mesh_and_gpu_upload.md) | GUID -> правильное хранилище (DataAssetsManager / PackageManager), загрузчик и типизированный кэш, дедупликация in-flight запросов; формирование ресурса Mesh из MeshData до состояния готовности к отправке на GPU |
| **17** | GUID, JSON-иерархия и упаковка сцен | ✅ Выполнено | [`stage_17_guid_json_hierarchy_and_scene_packing.md`](stage_17_guid_json_hierarchy_and_scene_packing.md) | Единое глобальное пространство неизменяемых GUID проекта: сцены/представления/скрипты/ресурсы из `.meta`, слои/объекты из JSON; GUID в `LayerData` и `GameObjectData`; нативный общий реестр и полная предсборочная проверка уникальности/типизированных ссылок; слои сохраняют JSON-порядок, деревья объектов рекурсивно уплощаются в preorder с локальными `parentIndex` |
| **18** | Линейное runtime-хранилище сцены, домены и spatial | ✅ Выполнено | [`stage_18_linear_scene_storage_domains_and_spatial.md`](stage_18_linear_scene_storage_domains_and_spatial.md) | Parent-before-child topology и линейная инициализация `NodeStorage`; dirty-иерархия пропорциональна числу узлов, перекрывающиеся dirty-поддеревья не пересчитываются; `NodeStorage` — единственный источник transform, плоский spatial хранит только handles узлов с мешем; живые `NodeBindings` (`domainKind/domainHandle/spatialHandle`) и взаимоисключающая маршрутизация `isEntity`. `isActive` только хранится |
| **19** | Очистка NodeStorage для иерархии и матриц, очистка GameObject | ✅ Выполнено | [`stage_19_visual_descriptors_soa_and_gameobject_cleanup.md`](stage_19_visual_descriptors_soa_and_gameobject_cleanup.md) | Изоляция `NodeStorage` строго под SoA-иерархию и трансформы; удаление устаревших структур; наполнение командных буферов возложено на пространственное хранилище; constexpr битовые флаги `Active/Visible` в `m_Flags` |
| **20** | Асинхронная загрузка Mesh и реактивная готовность сцены | ✅ Выполнено | [`stage_20_async_mesh_loading_and_gameobject_lifecycle.md`](stage_20_async_mesh_loading_and_gameobject_lifecycle.md) |
| **21** | Правки сборщика ассетов (единый импорт-пайплайн) | ✅ Выполнено | [`stage_21_assets_builder_pipeline_fixes.md`](stage_21_assets_builder_pipeline_fixes.md) | Единый реестр `AssetImporterRegistry`, ошибка на незарегистрированном файле, `type` обязателен только для скриптов, атомарная публикация `.tmp` + rename, общий `AssetScanner`, очистка рудимента `zzz_assets_test_000_build` |
| **22** | Разделение ResourceManager (CPU/GPU), прямая типизация без IResourceLoader, OneShotEvent, ResourceTable/ResourceRef и трёхзвенный конвейер | ✅ Выполнено | [`stage_22_resource_manager_ownership_and_ready_events.md`](stage_22_resource_manager_ownership_and_ready_events.md) | Полный демонтаж `IResourceLoader`, `RegisterLoader`, стирания типов и `OwnerToken`/`IsOwnerAlive`; шаблоны `ResourceTable<T>` и `ResourceRef<T>`; разделение на `CpuResourceManager` и `GpuResourceManager` (изолирован от диска, зависит от `CpuResourceManager`); примитив `OneShotEvent`; изоляция графических ресурсов от ресурсов других систем; контракт No-Hang гарантированного разрешения ошибок/исключений; приоритеты `eTaskPriority` |
| **23** | Рефакторинг I/O конвейера, Data-Driven Master TOC, ликвидация IoScheduler и ResourceStorageTraits, чистый GUID в PackageEntry и приоритеты ввода-вывода | ✅ Выполнено | [`stage_23_data_driven_toc_and_io_refactoring.md`](stage_23_data_driven_toc_and_io_refactoring.md) | Ликвидация `IoScheduler`, `ResourceStorageTraits` и `eAssetDirectoryKind`; двухархивная модель с путями `c_GamePackageRelativePath` и `c_DataPackageRelativePath`; сжатие `PackageEntry` под чистый 16-байтный GUID (строго 36 байт); загрузка сцен по имени через прозрачный индекс `m_SceneGuidsByName`; устранение бага `size == 0`; приоритетная диспетчеризация в `TaskDispatcher` и автономный shutdown `CpuResourceManager`; сквозная валидация GUID; унификация путей через `std::filesystem::path` |
| **24** | Постоянный archive handle (Memory-Mapped File) и Zero-Copy I/O конвейер архивов | ✅ Выполнено | [`stage_24_memory_mapped_archive_and_zero_copy_io.md`](stage_24_memory_mapped_archive_and_zero_copy_io.md) | Ликвидация системных вызовов в горячем пути; `ReadOnlyFile` и платформенный `MappedFileHandle` (Win32 mapping / POSIX `mmap` / Android `AAsset`), потокобезопасный `ReadWriteFile` и `FileSystemBase` без байтового I/O; интеграция в `ArchiveReaderBase`, `PackageManager` и `DataAssetsManager`; прямой `ReadRawPayload` как `std::span`; архивы неизменяемы до остановки движка, hot reload не поддерживается |
| **25** | Библиотека обработки и сжатия текстур (`texture_processor_lib`) | ⏳ В процессе | [`stage_25_texture_processor_library.md`](stage_25_texture_processor_library.md) | Выделенная статическая библиотека для декодирования (PNG, JPG, TGA, BMP, HDR через `stb_image`), валидации кратности 2, генерации mipmap-уровней (`stb_image_resize2` sRGB) и компрессии в GPU-форматы (BC7/BC5/BC4 через DirectXTex для Desktop, ASTC для Mobile). Формирование бинарного блоба с `TextureBlobHeader` (64 байта). |
| **26** | Трёхсекционный TOC data.dat, внешние пакеты и сборщик PackagePacker | 🚀 В работе | [`stage_26_multipak_toc_and_package_packer.md`](stage_26_multipak_toc_and_package_packer.md) | Разрабатываемый `data.dat` 1.0.0 с PackManifestTable, InlinePackageEntryTable и ExternalPackageEntryTable; имя внешнего пака выводится из `eDataDatType` (`0.dat` для Texture2D); builder-side проверка формата; recoverable directory swap без предварительного удаления рабочего `assets/` |
| **27** | Подготовка данных меша и последующий GPU upload | ⏳ Не начато | [`stage_27_gpu_buffers_and_upload.md`](stage_27_gpu_buffers_and_upload.md) | Чтение трёхсекционного `data.dat` и внешних `N.dat` (`0.dat`, `1.dat`...); хранилища `ReadOnlyFile` по `eDataDatType` в `DataAssetsManager` и $O(1)$ доступ по типу; чтение 32 байт `MeshMetadata` из `PackageEntry`; zero-allocation view/парсер `MeshPayloadView` над layout `MeshData`; запрос через `CpuResourceManager` с гарантированным lifetime; последующее создание GPUBuffer, staging-copy, submit/fence и настоящий GpuMesh |
| **28** | Базовые Shader и Material | ⏳ Не начато | — | После завершения GPU-upload этапа 27: компиляция минимальных шейдеров DXIL/SPIR-V, рабочий pipeline для куба, MaterialData + Texture2D + параметры; разрешение всех обязательных зависимостей сцены и OnStart только после полной CPU/GPU-готовности |
| **29** | Минимальный сквозной рендер куба | ⏳ Не начато | — | Базовая Camera, View/Projection и aspect при resize; обход DefaultSpatialStorage без BVH; игровой поток после update публикует неизменяемый набор команд кадра N, render-поток исполняет только набор N-1 без чтения живой сцены; DrawIndexed на DX12/Vulkan и раздельные CPU/GPU-барьеры |
| **30** | КП-1 / КП-2: вращающийся текстурированный куб на Windows | ⏳ Не начато | — | Проверка всей цепочки ассет -> пакет/файл -> ресурс -> скрипт -> Draw/Present под DX12 и Vulkan. Resize, minimize/restore, смена сцены и закрытие при загрузке, ошибка обязательного ресурса, отсутствие преждевременного GPU-release; закрытие применимых проверок этапа 10 |

### Уровень 3: ZzzGUI-MVP и первый перенос

Этапы выполняются строго последовательно от **31** до **39**. Расширенные 3D-возможности вынесены в Уровень 4 и не блокируют GUI-MVP.

| № | Этап разработки | Статус | Файл этапа | Краткое описание |
|---|---|---|---|---|
| **31** | Зона рендеринга и DPI | ⏳ Не начато | — | Расширение базового viewport из этапа 29: Scissor, логические единицы DIP/Canvas Units, DPI/размер окна и Safe Area; обновление WindowCommon::OnDpiChanged (TODO 11) |
| **32** | Дерево и разметка ZzzGUI (Measure/Arrange) | ⏳ Не начато | — | gui_lib, размеры и координаты элементов, базовые контейнеры StackPanel/Grid/Canvas/Border, clipping и основа ScrollViewer; геометрия для hit-testing |
| **33** | 2D GPU GUI Batcher | ⏳ Не начато | — | GuiVertex2D, динамические буферы, текстуры и Scissor Rects; вывод результатов layout, lifetime кадровых данных по готовому контракту GAPI |
| **34** | Рендеринг текста | ⏳ Не начато | — | Выбор технологии в момент реализации; измерение текста для layout, глифы и интеграция в batcher, основа TextBlock/TextBox |
| **35** | Ввод и маршрутизация по слоям | ⏳ Не начато | — | Hit-testing по результатам layout, focus/capture, мышь/клавиатура; интерфейсы touch/gamepad с проверкой на соответствующих платформах; поглощение событий GUI и проброс в 3D |
| **36** | Базовые контролы | ⏳ Не начато | — | Button, TextBlock, Image, Slider, ProgressBar, CheckBox, TextBox с фокусом, кареткой и вводом; доведение ScrollViewer на готовых layout, вводе и тексте |
| **37** | Data Binding и On-Demand GUI | ⏳ Не начато | — | ObservableProperty<T>, ICommand, MVVM, инвалидирование layout/рисования и обновление только изменившегося интерфейса; без зависимости от полного LayerScript этапа 47 |
| **38** | КП-3: GUI-MVP на Windows | ⏳ Не начато | — | Одинаковое поведение GUI на DX12/Vulkan: ввод, фокус, текст, привязки, resize/DPI и совместный вывод куба и UI |
| **39** | КП-4: Linux (Vulkan) | ⏳ Не начато | — | Перенос и проверка куба + ZzzGUI-MVP; это первая обязательная Linux-приёмка, а не повтор этапа 30 |

### Уровень 4: Расширение рендера и скриптов

| № | Этап разработки | Статус | Файл этапа | Краткое описание |
|---|---|---|---|---|
| **40** | Расширение Camera и Frustum | ⏳ Не начато | — | Развитие базовой камеры из этапа 29, плоскости видимости и необходимые режимы камеры |
| **41** | Пространственные индексы и Frustum Culling | ⏳ Не начато | — | AABB, развитие ISpatialStorage вместо плоского DefaultSpatialStorage; выбор BVH/Octree по задаче и замерам, без обещания O(log N) для любого запроса видимости |
| **42** | Оптимизация кадровой очереди | ⏳ Не начато | — | Расширение работающих команд из этапа 29: результаты culling, сортировка и batching; не первая реализация Draw/RenderQueue |
| **43** | Развитие шейдеров и кэша PSO | ⏳ Не начато | — | Кэш и инвалидация поверх рабочего pipeline этапа 28: VkPipelineCache/ID3D12PipelineLibrary; офлайн-упаковка шейдеров относится к этапу 51 |
| **44** | Фасады движка для скриптов | ⏳ Не начато | — | EngineFacade/SceneFacade/ViewFacade, границы доступа поверх минимального API этапа 28; герметизация Engine.h и SDK-изоляция (TODO 24) |
| **45** | Изоляция и видимость скриптов | ⏳ Не начато | — | Политика доступа между уровнями скриптов; не обязательная зависимость базового CubeRotatorScript |
| **46** | Сериализуемые поля и ссылки (FieldRef) | ⏳ Не начато | — | Ссылки на GameObject, скрипты и ассеты, двухфазное разрешение; основа сохранения состояния при hot-reload |
| **47** | LayerScript и независимая кадровая частота слоёв | ⏳ Не начато | — | Layer3DScript/Layer2DScript/LayerMVVMScript, жизненный цикл, независимый FPS; развитие On-Demand из этапа 37. Здесь же определяется будущее поведение сохранённого `isActive` и эффективной активности в иерархии; 3D и эффекты в UI относятся к этапу 60 |

### Уровень 5: Ассеты, инструментарий и надёжность

| № | Этап разработки | Статус | Файл этапа | Краткое описание |
|---|---|---|---|---|
| **48** | Параметры импорта в .meta | ⏳ Не начато | — | Типоспецифичные параметры: mipmaps, сжатие, фильтрация, sRGB, настройки мешей и материалов; версия и миграция формата |
| **49** | Развитие пресетов сборки | ⏳ Не начато | — | Расширять реализованные в этапе 10 `build_settings/presets.json` и `platforms/<platform>/*.json` только по появившимся требованиям; не вводить заново старую `project_<platform>.json`-схему |
| **50** | Платформенные форматы ресурсов | ⏳ Не начато | — | Развитие общего MVP-формата и схемы выбора ресурсов; платформенные варианты без дублирования идентичности ассетов |
| **51** | Офлайн-компиляция ресурсов | ⏳ Не начато | — | Выбор и интеграция codec/checksum для payload `data.dat`/`.pak` с явной миграцией формата, упаковка скомпилированных шейдеров, расширенная оптимизация, зависимости и инвалидация результатов импорта (базовая компрессия текстур реализована в этапе 25) |
| **52** | Студия проектов | ⏳ Не начато | — | tools/editor и assets_builder: инспектор .zmat, FieldRef, безопасность unload/reload scripts.dll, состояние скриптов, режим сборки; долг TODO 2, 12–19 по актуальности |
| **53** | Доставка ресурсов в игровые таргеты | ⏳ Не начато | — | Развитие базовой Windows-доставки `package.dat`/`data.dat`/внешних `.pak`, необходимой уже в этапах 26–30: полнота отдельных файлов и поддержка остальных desktop/mobile таргетов |
| **54** | Развитие асинхронной загрузки сцен и отмена запросов | ⏳ Не начато | — | Базовая загрузка всех видов окон уже в этапе 8. Здесь дорабатываются владение задачами и безопасное завершение `SceneManager`, кооперативная отмена устаревших I/O-запросов ресурсов конвейера при смене/выгрузке сцены (TODO 26), состояния active/paused/unloading и диагностика |

### Уровень 6: Apple/mobile и полировка

| № | Этап разработки | Статус | Файл этапа | Краткое описание |
|---|---|---|---|---|
| **55** | КП-5: macOS и мобильные | ⏳ Не начато | — | Куб + ZzzGUI-MVP на macOS/iOS (Metal), Android (Vulkan); реализация недостающих GPU-ресурсов/pipeline, touch, DPI/Safe Area и Android resize (TODO 10) |
| **56** | SIMD для Mat4 | ⏳ Не начато | — | SSE2/AVX и NEON по замерам без изменения публичного API |
| **57** | CPU-ядра и пулы потоков (TaskDispatcher) | ⏳ В процессе | [stage_53_task_dispatcher_preview.md](stage_53_task_dispatcher_preview.md) | TaskDispatcher, P/E-ядра, affinity и приоритеты; CpuTopology |
| **58** | Прозрачность окна | ⏳ Не начато | — | Альфа окна и GAPI; Acrylic/Mica как нативная надстройка |
| **59** | Платформенный сплэшскрин | ⏳ Не начато | — | Win/Linux/macOS frameless splash, Android SplashScreen, iOS Storyboard |
| **60** | 3D и GPU-эффекты в UI | ⏳ Не начато | — | Viewport3DControl, интерактивные объекты, частицы, Glass/Acrylic blur |
| **61** | Адаптивный ZzzGUI | ⏳ Не начато | — | Desktop/mobile, стабильные Element ID, платформенные шаблоны и UI-скрипты |
| **62** | Высоконагруженный стриминг ресурсов (NVMe I/O и шардирование ResourceTable) | ⏳ Не начато | — | Развитие mapped-span и базового GPU-upload пути до платформенного асинхронного I/O/DirectStorage по измерениям; шардирование `ResourceTable<T>`; выделенная copy-очередь DX12, адаптивный backpressure, исследование Staging Ring-Buffer, chunked upload и стриминга открытого мира. Отложенная загрузка не меняет правило полной готовности обязательных ресурсов сцены без нового решения |
| **63** | Визуальные переходы сцен | ⏳ Не начато | — | Развитие каркаса из этапа 8: Fade/Cross-Fade/Dissolve/Wipe, маски, захват кадра |

### Уровень 7: Финал

| № | Этап разработки | Статус | Файл этапа | Краткое описание |
|---|---|---|---|---|
| **64** | Итоговая документация | ⏳ Не начато | — | Перенос актуальных решений в docs/, проверка ссылок и сохранности истории перед согласованным удалением current_plan/ |
