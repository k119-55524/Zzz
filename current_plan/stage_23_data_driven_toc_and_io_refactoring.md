# Этап 23. Рефакторинг I/O конвейера, Data-Driven Master TOC, ликвидация IoScheduler и ResourceStorageTraits, чистый GUID в PackageEntry и приоритеты ввода-вывода

---

## 🎯 Цель этапа

1. **Двухархивная структура и Data-Driven Master TOC контента:**
   - Формат `PackageEntry` переведён на чистый 16-байтный `Guid` (удалён `c_MaxAssetNameLength` и буфер `FixedLengthString32<64>`, размер записи сжат с 292 до 36 байт).
   - `PackageEntry` расширен полями `pakIndex` и `location` (`eFileLocation::App` / `Cache`).
   - Два строго раздельных архива:
     * `./package.dat` — манифест проекта (`ProjectManifestData`), окна и сцены игры.
     * `./assets/data.dat` — единый самодостаточный архив игровых ресурсов (Master TOC + полезные нагрузки: меши, материалы, шейдеры, текстуры).
   - Транзакционная двухсторонняя атомарная публикация (`package.dat`, `assets/data.dat`) с защитным откатом при сбое.

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
- [x] Расширить `PackageEntry.h` полями `pakIndex` и `location`, обновить `BinarySize`, `Serialize` и `Deserialize`.
- [x] Удалить `src/core/enums/eAssetDirectoryKind.h` и метод `Path::GetDirectory(eAssetDirectoryKind)`.
- [x] Унифицировать `FileSystemBase` под `const std::filesystem::path&`.
- [x] Перенести `c_FieldIsEntity` в `PackagePacker.cpp`.

### 2. Сборщик ассетов (Assets Builder)
- [x] Унифицировать `ArchiveWriter` под единый `WriteBinaryArchive` без лишних дублирующих функций.
- [x] В `PackagePacker.cpp` формировать `package.dat` и `assets/data.dat`.
- [x] Реализовать атомарную замену двух архивов с защитным откатом.

### 3. Подсистема оглавления и валидации
- [x] В `DataAssetsManager.h` и `.cpp`:
  - `AssetLocation`: использовать `const PackageEntry* entry`.
  - Добавить проверку валидности GUID и дубликатов под `#if Z_DEBUG_BUILD || Z_DEVELOPMENT_BUILD`.
  - Удалить строковые поиски `m_EntriesByName` и `GetEntry(name)`.
  - Чтение данных из `c_DataPackageRelativePath` (`assets/data.dat`).
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
- [x] Вынести расширение `.dat` в константу `c_DatExtension` и базовые имена (`c_PackageName = "package"`, `c_DataName = "data"`, `c_UserConfigName = "user"`).
- [x] Ввести единые трёхбуквенные сигнатуры и версионирование:
  * `package.dat` — `c_PackageDatHeader` (`"ZPD"`), `c_PackageDatFileMajorVersion`
  * `data.dat` — `c_DataDatHeader` (`"ZDD"`), `c_DataDatFileMajorVersion`
  * `{guid}.dat` — `c_AssetPackageHeader` (`"ZAP"`), `c_AssetPackageFileMajorVersion`
  * `user.dat` — `c_UserConfigHeader` (`"ZUD"`), `c_UserConfigFileMajorVersion`
- [x] Добавить в начало `PackageConstants.h` архитектурную документацию по ручной настройке структуры хранения данных проекта.

### 7. Изоляция констант и интеграция сборщика (IWYU & PackageConstants.cs)
- [x] Полностью удалить зонтичный заголовок `src/core/constants/Constants.h` и перевести ядро, движок и тесты на принцип IWYU (Include What You Use).
- [x] Создать в `assets_builder_lib` класс-прослойку `PackageConstants.cs`, инкапсулирующий чтение путей, версий, сигнатур и типов пакетов из нативной DLL.
- [x] Экспортировать `GetAssetsDirectoryName()`, `GetGamePackageRelativePath()`, `GetDataPackageRelativePath()` из `assets_builder_dll`.
- [x] Перенести `package.dat` внутрь каталога `assets/` (`c_GamePackageRelativePath = c_AssetsDirectoryName / c_GamePackageFileName`), обеспечив консистентность со всеми платформами (Windows, Linux, Android, iOS, macOS) и корректную очистку GUI сборщика.

---

## 🔍 Критерии приёмки (DoD)
1. Проект собирается полностью без ошибок компилятора и линковщика (`cmake --build build --config Debug --target assets_builder_dll game_win editor_dll`).
2. Сборщик формирует чистую двухархивную структуру внутри `assets/`: `assets/package.dat` (манифест, декларации представлений и сцены) и `assets/data.dat` (TOC и ресурсы контента).
3. В коде движка отсутствуют привязки расширений файлов, строковых имён ресурсов в TOC и категорийных путей.
4. В рантайме гарантируется уникальность GUID внутри архивов.
5. В Release-сборках проверочный код валидации полностью исключается.
