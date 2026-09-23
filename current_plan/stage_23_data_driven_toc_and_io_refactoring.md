# Этап 23. Рефакторинг I/O конвейера, Data-Driven Master TOC, ликвидация IoScheduler и ResourceStorageTraits, чистый GUID в PackageEntry и приоритеты ввода-вывода

---

## 🎯 Цель этапа

1. **Двухархивная структура и Data-Driven Master TOC контента:**
   - Формат `PackageEntry` очищен от имени ресурса (`c_MaxAssetNameLength`, `FixedLengthString32<64>`), а также от полей `pakIndex` и `location`, зафиксировав чистый бинарный размер ровно 36 байт (`Guid` 16 + `assetType` 4 + `offset` 8 + `size` 8).
   - Два строго раздельных архива:
     * `c_GamePackageRelativePath` — манифест проекта (`ProjectManifestData`), окна и сцены игры.
     * `c_DataPackageRelativePath` — единый самодостаточный архив игровых ресурсов (Master TOC + полезные нагрузки: меши, материалы, шейдеры, текстуры).
   - Транзакционная двухсторонняя атомарная публикация обоих архивов с защитным откатом при сбое.

2. **Ликвидация устаревших структур и категорийных путей:**
   - Полное удаление класса `IoScheduler` и `std::jthread`.
   - Полное удаление `ResourceStorageTraits.h` и жестких категорийных подкаталогов (`textures/`, `video/`, `audio/`, `fonts/`).
   - Полное удаление `eAssetDirectoryKind` и метода `Path::GetDirectory(eAssetDirectoryKind)`.
   - Перевод интерфейса `FileSystemBase` (`FileExists`, `ReadBytes`, `ReadAllBytes`, `WriteAllBytes`) на `const std::filesystem::path&`.
   - Перенос JSON-константы `c_FieldIsEntity` из ядра в анонимный namespace сборщика `PackagePacker.cpp`.

3. **Оптимизация `AssetLocation` и устранение накладных расходов:**
   - `AssetLocation` хранит константный указатель `const PackageEntry* entry` вместо копирования структур.
   - Устранен критический баг `size == 0`: пустые ресурсы возвращают пустой вектор байт без чтения всего файла через `ReadAllBytes`.

4. **Приоритетная диспетчеризация I/O и автономная безопасность `CpuResourceManager`:**
   - Задачи дискового чтения ставятся напрямую в пулы `TaskDispatcher` с запрошенным приоритетом (`High`, `Normal`, `Background`). Приоритет `Critical` защищен ассертом `ensure`.
   - Ликвидировано узкое место единой очереди, где высокоприоритетные запросы ждали низкоприоритетные.
   - Реализован автономный и потокобезопасный контракт остановки `CpuResourceManager`: атомарный счетчик активных задач `m_ActiveIoTasks` и условная переменная в деструкторе гарантируют завершение всех задач и вызов callback с ошибкой при отмене (контракт No-Hang).

5. **Локальная валидация уникальности GUID в архивах:**
   - В `DataAssetsManager` и `PackageManager` валидация уникальности GUID работает внутри каждого архива.
   - Удалён строковый поиск по именам ресурса (`m_EntriesByName`), все операции опираются строго на 16-байтный `Guid`.
   - Все проверочные вычисления и выражения закрыты под `#if Z_DEBUG_BUILD || Z_DEVELOPMENT_BUILD`, гарантируя нулевой оверхед в Release-сборках.

---

## 🛠️ Детальный план работ

### 1. Ядро (Core I/O) и форматы пакетов
- [x] Удалить `src/engine/resources/io/IoScheduler.h` и `IoScheduler.cpp`.
- [x] Удалить `src/core/io/ResourceStorageTraits.h` и убрать его из `src/core/CMakeLists.txt`.
- [x] Очистить `PackageConstants.h` от устаревших констант путей подпапок (`c_TexturesDirectoryRelativePath` и др.).
- [x] Удалить `c_MaxAssetNameLength` и `FixedLengthString32` из `PackageEntry.h`.
- [x] Очистить `PackageEntry.h` от полей `pakIndex` и `location`, зафиксировав бинарный размер строго 36 байт (`BinarySize`, `Serialize` и `Deserialize`).
- [x] Удалить `src/core/enums/eAssetDirectoryKind.h` и метод `Path::GetDirectory(eAssetDirectoryKind)`.
- [x] Унифицировать `FileSystemBase` под `const std::filesystem::path&`.
- [x] Перенести `c_FieldIsEntity` в `PackagePacker.cpp`.

### 2. Сборщик ассетов (Assets Builder)
- [x] Унифицировать `ArchiveWriter` под единый `WriteBinaryArchive` без лишних дублирующих функций.
- [x] В `PackagePacker.cpp` формировать архивы по путям `c_GamePackageRelativePath` и `c_DataPackageRelativePath`.
- [x] Реализовать атомарную замену двух архивов с защитным откатом.

### 3. Подсистема оглавления и валидации
- [x] В `DataAssetsManager.h` и `.cpp`:
  - `AssetLocation`: использовать `const PackageEntry* entry`.
  - Добавить проверку валидности GUID и дубликатов под `#if Z_DEBUG_BUILD || Z_DEVELOPMENT_BUILD`.
  - Удалить строковые поиски `m_EntriesByName` и `GetEntry(name)`.
  - Чтение данных из `c_DataPackageRelativePath`.
- [x] В `PackageManager.h` и `.cpp`:
  - Добавить проверку валидности GUID и дубликатов под `#if Z_DEBUG_BUILD || Z_DEVELOPMENT_BUILD`.
  - Удалить строковые поиски `m_EntriesByName` и `GetEntry(name)`.
  - Поиск и загрузка ресурсов строго по GUID.

### 4. Менеджер ресурсов (CpuResourceManager)
- [x] Устранить баг `size == 0`: отдавать пустой буфер без чтения всего файла.
- [x] Перевести постановку задач чтения на прямую отправку в `TaskDispatcher` с запрошенным приоритетом.
- [x] Реализовать счетчик активных задач `m_ActiveIoTasks` и ожидание в деструкторе `~CpuResourceManager()`.
- [x] Гарантировать вызов callback при остановке сервиса (No-Hang).

### 5. Чистка интерфейса путей и файловой системы
- [x] Полностью удалить дублирующий enum `eUserDirectoryKind` и перевести `Path::GetDirectory` строго на `eFileLocation`.
- [x] Очистить интерфейс `Path.h`: удалить дублирующие методы `GetExecutableDirectory()`, `GetUserDataDirectory()`, а также мёртвые методы пакетов `GetPackageDatPath()`, `GetUserDatPath()`, `GetDataDatPath()`.
- [x] Упростить `FileSystemBase::ResolvePhysicalPath`: сделать чистым `inline` методом без лишних `try-catch` и `std::expected`.

### 6. Унификация констант, расширений и трёхбуквенных сигнатур пакетов
- [x] Вынести расширение и базовые имена файлов в `c_DatExtension`, `c_PackageName`, `c_DataName` и `c_UserConfigName`.
- [x] Ввести единые трёхбуквенные сигнатуры и версионирование:
  * главный пакет — `c_PackageDatHeader`, `c_PackageDatFileMajorVersion`
  * архив данных — `c_DataDatHeader`, `c_DataDatFileMajorVersion`
  * внешний пакет ассетов — `c_AssetPackageHeader`, `c_AssetPackageFileMajorVersion`
  * пользовательская конфигурация — `c_UserConfigHeader`, `c_UserConfigFileMajorVersion`
- [x] Добавить в начало `PackageConstants.h` архитектурную документацию по ручной настройке структуры хранения данных проекта.

### 7. Изоляция констант и интеграция сборщика (IWYU & PackageConstants.cs)
- [x] Полностью удалить зонтичный заголовок `src/core/constants/Constants.h` и перевести ядро, движок и тесты на принцип IWYU (Include What You Use).
- [x] Создать в `assets_builder_lib` класс-прослойку `PackageConstants.cs`, инкапсулирующий чтение путей, версий, сигнатур и типов пакетов из нативной DLL.
- [x] Экспортировать `GetAssetsDirectoryName()`, `GetGamePackageRelativePath()`, `GetDataPackageRelativePath()` из `assets_builder_dll`.
- [x] Перенести `package.dat` внутрь каталога `assets/` (`c_GamePackageRelativePath = c_AssetsDirectoryName / c_GamePackageFileName`), обеспечив консистентность со всеми платформами (Windows, Linux, Android, iOS, macOS) и корректную очистку GUI сборщика.

### 8. Унификация сопоставления типов и очистка интерфейсов подсистемы пакетов
- [x] Унификация сопоставления типов контента со статическими константами (Способ 2):
  * Добавлены статические константы `static constexpr ePackage c_PackageType` в классы данных пакетов (`ProjectManifestData`, `PrimaryViewData`, `SceneData`, `ChildViewData`, `IndependentViewData`, `PrefabData`).
  * Добавлены статические константы `static constexpr eResourceType c_ResourceType` в классы данных (`MeshData`, `MaterialData`, `ShaderData`, `PrefabData`) и CPU-ресурсов (`CpuMesh`, `CpuMaterial`, `CpuTexture2D`, `CpuShader`).
  * Удалены внешние специализации шаблонов-структур (`PackageAssetType` в `PackageManager.h`, `GetCpuResourceType<T>()` в `CpuResourceManager.h`, `DataAssetResourceType` в `DataAssetsManager.h`). Концепт `ParsableCpuResource` валидирует `T::c_ResourceType`.
- [x] Ревизия и оптимизация `PackageManager`:
  * Устранён избыточный вызов `GetEntry` перед `LoadAsset` в `SceneManager.cpp`.
  * Удалён устаревший `friend class SceneManager;` из `PackageManager.h`, восстановлена строгая инкапсуляция класса.
  * Метод `ReadRawBytes` скрыт в `private`.
  * Удалён избыточный метод `DeserializeEntryFromMemory` и 6 его явных инстанцирований в `PackageManager.cpp`, логика десериализации объединена внутри `DeserializeEntry`.
  * Удалены неиспользуемые рудименты публичного интерфейса `GetHeader()` и `GetBuildTime()`.
- [x] Полная очистка устаревших артефактов `.dat`:
  * Удалены старые рудименты `package.dat`, `data/data.dat`, `paks/package_0.dat` из `dist/Debug/assets/` и каталогов платформенных проектов.
  * Подтверждена кроссплатформенная готовность путей CMake и `FileSystemAndroid` к структуре, заданной `c_GamePackageRelativePath` и `c_DataPackageRelativePath`.

### 9. Сквозная валидация архивов, чистый 36-байтный PackageEntry и имена сцен
- [x] Очистить `PackageEntry` от `pakIndex` и `location`, зафиксировав бинарный размер строго 36 байт (`Guid` 16 + `assetType` 4 + `offset` 8 + `size` 8).
- [x] Перевести `DataAssetsManager` на плоскую таблицу `std::unordered_map<Guid, PackageEntry> m_Entries;` и сквозную валидацию уникальности GUID по всему архиву данных.
- [x] В `PackageManager` реализовать проверку уникальности GUID по всему главному пакету.
- [x] В `ProjectManifestData` добавить структуру `SceneManifestEntry { std::string name, Guid guid; }` и хранить сцены как пары имя/GUID.
- [x] В `PackageManager` и `SceneManager` построить быстрый прозрачный хэш-индекс `m_SceneGuidsByName` для поиска сцен по имени за $O(1)$ без лишних аллокаций и реализовать метод `SceneManager::LoadSceneAsync(std::string_view sceneName, ...)`.
- [x] В `AssetsBuilderEngine.cs` и `PackagePacker.cpp` сохранить проверку уникальности имён сцен в проекте для исключения коллизий при загрузке по имени.

### 10. Надёжность I/O конвейера и устранение дедлоков
- [x] В `TaskDispatcher::Submit` возвращать `bool` (успех `Enqueue`) для предотвращения зависания счетчика `m_ActiveIoTasks` при закрытом пуле, корректно оповещать callback в `CpuResourceManager` и `SceneManager`.
- [x] В `CpuResourceManager::GetAsync` декрементировать `m_ActiveIoTasks` ДО вызова пользовательского коллбэка `onLoaded` для исключения self-deadlock при разрушении менеджера из коллбэка.
- [x] В `CMakeLists.txt` платформ добавить автоматическое удаление старой директории `${CMAKE_COMMAND} -E rm -rf "$<TARGET_FILE_DIR>/assets"` перед `copy_directory`.
- [x] Актуализировать `docs/ARCHITECTURE.md` и `general_plan.md`.

---

## 🔍 Критерии приёмки (DoD)
1. Проект собирается полностью без ошибок компилятора и линковщика (`cmake --build build --config Debug --target assets_builder_dll game_win editor_dll`).
2. Размер `PackageEntry::BinarySize()` строго равен 36 байтам.
3. Валидация GUID гарантирует глобальную уникальность записей внутри архивов `package.dat` и `data.dat`.
4. Скрипты могут загружать сцены как по GUID, так и по строковому имени через быстрый хэш-индекс $O(1)$.
5. Исключены зависания при shutdown `CpuResourceManager` и self-deadlock при вызове `onLoaded`.
6. Каталог `assets/` при сборке платформенных проектов очищается от старых файлов и поддиректорий.
7. Документация `ARCHITECTURE.md` полностью синхронизирована с фактическим кодом.
