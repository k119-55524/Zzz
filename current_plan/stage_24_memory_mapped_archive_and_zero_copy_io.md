# Этап 24. Постоянный archive handle (Memory-Mapped File) и Zero-Copy I/O конвейер архивов

---

## 🎯 Цель этапа

1. **Ликвидация системных вызовов в горячем пути загрузки ассетов:**
   - Устранить цепочку `file_size + open + seek + read + close` на каждый загружаемый ресурс.
   - Исключить обращение к файловой системе для проверки размера файла: размер архива фиксируется один раз при монтировании (`m_ArchiveSize`), а валидация диапазона `IsRangeInside` выполняется как мгновенная проверка в памяти.
   - Заменить открытие файла на каждый чих постоянным отображением в виртуальную память (Memory-Mapped File).

2. **Архитектура модуля `src/core/io/storage/` с 3 специализированными классами:**
   - **`ReadOnlyFile`** — неизменяемое отображение физического файла по переданному пути (`std::filesystem::path`), zero-copy `std::span` и `Subspan`, lock-free потокобезопасность «из коробки». Полная изоляция от `FileSystem`.
   - **`MappedFileHandle`** (`storage/platforms/`) — RAII-хэндл системного маппинга без `#ifdef` в коде (выбирается через CMake):
     * **Windows (`MappedFileHandleMSWin.cpp`):** `CreateFileW`, `CreateFileMappingW`, `MapViewOfFile`, `UnmapViewOfFile`, `CloseHandle`.
     * **POSIX (`MappedFileHandlePosix.cpp`):** `open`, `fstat`, `mmap`, `munmap`, `close`.
     * **Android (`MappedFileHandleAndroid.cpp`):** NDK `AAsset` fallback.
   - **`ReadWriteFile`** — потокобезопасный (`std::mutex`) файл для изменяемых данных (`user_settings.json`, сейвы, логи): `Read`, `Write`. Хэндл между вызовами не удерживается. 100% кроссплатформенный код без `#ifdef`. Права на путь проверяет `FileSystem` до создания объекта (см. п. 6 детального плана).
   - **`FileSystemBase` / `FileSystem`** — чистая топология путей и песочниц (`eFileLocation`), создание каталогов, проверки существования файлов. **Полное исключение методов чтения/записи байт (`ReadBytes`, `ReadAllBytes`, `WriteAllBytes`) из `FileSystem`**. Выступает в роли резолвера путей (`GetGamePackagePath`, `GetDataPackagePath`), передавая разрешённые пути потребителям.

3. **Интеграция в `ArchiveReaderBase<TType>`, разделение типизации и `ResourceDatMapping`:**
   - Хранение `ReadOnlyFile m_ReadOnlyFile;` строго по значению внутри `ArchiveReaderBase` (без `std::shared_ptr`, без зависимости от `FileSystem`).
   - Унификация конструктора: `ArchiveReaderBase(const std::filesystem::path&)` принимает исключительно физический путь. Полная ликвидация параметров форматов и рантайм-колбэков валидации `TypeValidator<TType>`.
   - Разделение зон ответственности типизации:
     * `ePackageDatType` — типы записей оглавления для `package.dat`.
     * `eDataDatType` — типы записей оглавления для `data.dat`.
     * `eEngineResourceType` (`eResourceType`) — единый логический реестр ресурсов движка и редактора.
     * `eScriptType` — независимый enum типов исходного кода скриптов (иерархия жизненного цикла, MVVM + UIBehavior, данные/редактор).
     * `ResourceDatMapping.h` — сопоставитель: `GetArchiveLocation`, строгие `ToPackageDatType` / `ToDataDatType` с броском `THROW_RUNTIME`, безопасные небросающие `TryTo...` и операторы `==`.
   - Введение `ArchiveTraits<TType>` в `ArchiveReaderBase.h`: связь между физическим типом архива (`ePackageDatType`, `eDataDatType`), ожидаемым форматом (`c_PackageDatFormat`, `c_DataDatFormat`) и валидацией разрешённых типов диапазоном без ручных свитчей.
   - `PackageManager` и `DataAssetsManager` наследуют `ArchiveReaderBase<ePackageDatType>` и `ArchiveReaderBase<eDataDatType>`, принимают разрешённый `std::filesystem::path`. В `LoadAsset<T>` валидация типов выполняется в `static_assert(ArchiveTraits<...>::IsTypeAllowed(...))`.
   - В `InitializeArchive`: маппинг архива один раз при старте; чтение заголовка и оглавления прямо из `std::span` mapped-памяти без лишних дисковых I/O.
   - Добавление `ReadRawPayload`, возвращающего прямой zero-copy `std::span<const std::byte>`.
   - Сохранение `ReadRawBytes(entry)` как совместимой копирующей обёртки только для потребителей, которым действительно нужен `std::vector<std::byte>`.

4. **Zero-Copy конвейер в `CpuResourceManager`:**
   - В worker-задаче `CpuResourceManager::GetAsync` исключить прямую зависимость от `FileSystem` и аллокацию промежуточного буфера на desktop.
   - Получение `std::span<const std::byte>` напрямую из `DataAssetsManager::ReadRawPayload(entry)`.
   - Передача `std::span<const std::byte>` напрямую в статический фабричный метод `T::CreateCpuResourceFromPackageBytes(entry, span)`.
   - На desktop — отсутствие промежуточной heap-аллокации payload и системных вызовов в фоновых потоках `TaskDispatcher`; мобильный fallback сохраняет прежнее диапазонное чтение.

5. **Тестирование и верификация:**
   - Верификация корректности чтения `package.dat` и `data.dat` через `EngineTests`.
   - Стресс-тест параллельного чтения одного и того же и разных диапазонов из нескольких рабочих потоков.
   - Тестирование граничных условий (пустые ассеты `size == 0`, выход за границы mapped-диапазона).

---

## 🛠️ Детальный план работ

### 1. Ядро (Core I/O) — класс `MemoryMappedFile`
- [x] Создать `src/core/io/MemoryMappedFile.h/.cpp` с платформенно изолированными реализациями.
- [x] Реализовать безопасное открытие, отображение и закрытие дескрипторов (Win32 `CloseHandle` / `UnmapViewOfFile`, POSIX `munmap` / `close`).
- [x] Поддержать получение `std::span<const std::byte>` и безопасного диапазона `Subspan(offset, size)`.
- [x] Добавить в `src/core/CMakeLists.txt`.

### 2. Базовый читатель архивов (`ArchiveReaderBase<TType>`)
- [x] Добавить постоянный mapping и `std::size_t m_ArchiveSize` в `ArchiveReaderBase`.
- [x] Реализовать compile-time `ArchiveTraits<TType>` (`ePackage`, `eResourceType`), устранить runtime-колбэк валидатора и унифицировать конструкторы `ArchiveReaderBase`, `PackageManager` и `DataAssetsManager` с передачей только пути `(const std::filesystem::path&)`.
- [x] Переписать `InitializeArchive`:
  * Открытие `MemoryMappedFile`.
  * Чтение заголовка `DatFileHeader` и таблицы `PackageEntry` прямо из mapped-памяти.
- [x] Реализовать самодостаточный `ReadRawPayload(const PackageEntry&)`.
- [x] Оптимизировать `ReadRawBytes` и `DeserializeEntryRaw` для работы поверх payload span.

### 3. Менеджер данных ассетов (`DataAssetsManager`)
- [x] Унаследовать `ReadRawPayload(const PackageEntry&)`.
- [x] Оптимизировать `LoadAsset<T>` для десериализации напрямую из `std::span<const std::byte>` без аллокации вектора на desktop.

### 4. Менеджер CPU-ресурсов (`CpuResourceManager`)
- [x] В worker-задаче `CpuResourceManager::GetAsync` заменить прямое чтение файла на `m_DataAssetsManager->ReadRawPayload(entry)`.
- [x] Передавать `std::span<const std::byte>` в `T::CreateCpuResourceFromPackageBytes(entry, span)`.
- [x] Полностью исключить зависимость `CpuResourceManager` от `m_FileSystem`.

### 5. QA и верификация
- [x] Добавить в `EngineTests` проверки mapped payload, пустого/некорректного диапазона и удержания данных.
- [x] Проверить end-to-end тест `PackagePackerAndDataAssetsManagerEndToEnd`.
- [x] Проверить многопоточную устойчивость параллельной десериализации одного ресурса.

### 6. Валидация прав файлов и упрощение `ReadWriteFile`
- [ ] Платформенная обёртка `FileAccess` в `src/core/io/storage/platforms/file_access/` по образцу `MappedFileHandle`: один заголовок и реализации, выбираемые через CMake. Две проверки без изменений на диске: можно ли читать и писать существующий файл; можно ли создать файл в каталоге.
  * **Windows (`FileAccessMSWin.cpp`):** открытие хэндла файла (или каталога с правом на добавление файла) с нужными правами и немедленное закрытие.
  * **POSIX (`FileAccessPosix.cpp`, Linux/macOS/iOS/Android):** `access()` с нужными флагами.
- [ ] В `FileSystemBase` приватный метод `ValidateFilePath` проверяет путь изменяемого файла: существующий путь должен быть обычным файлом с доступом на чтение и запись, а для отсутствующего файла непосредственный родительский каталог должен позволять его создание.
- [ ] `GetGamePackagePath` / `GetDataPackagePath` по контракту только читаются: проверяют наличие файла через `FileExists` (на Android — ассеты APK через существующее переопределение). Флаг не принимают.
- [ ] `FileSystemBase::InitializeUserData` переименован в `GetUserConfigPath`: определяет и создаёт каталог пользовательских данных, затем возвращает проверенный путь к файлу настроек. `engine.cpp` при ошибке бросает исключение.
- [ ] `ReadWriteFile`: конструктор только сохраняет подготовленный `FileSystem` путь (пробный файл удалён). `Read` / `Write` выполняют файловые операции напрямую; пустые данные пропускаются с предупреждением в лог.
