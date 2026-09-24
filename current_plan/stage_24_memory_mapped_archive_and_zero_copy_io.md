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
   - **`ReadWriteFile`** — потокобезопасный (`mutable std::mutex`) файловый поток C++23 для изменяемых данных (`user_settings.json`, сейвы, логи): `ReadAll`, `WriteAll`, `Read`, `Write`, `Flush`. 100% кроссплатформенный код без `#ifdef`.
   - **`FileSystemBase` / `FileSystem`** — чистая топология путей и песочниц (`eFileLocation`), создание каталогов, проверки существования и удаление файлов. **Полное исключение методов чтения/записи байт (`ReadBytes`, `ReadAllBytes`, `WriteAllBytes`) из `FileSystem`**. Выступает в роли резолвера путей (`GetGamePackagePath`, `GetDataPackagePath`), передавая разрешённые пути потребителям.

3. **Интеграция в `ArchiveReaderBase<TType>`:**
   - Хранение `ReadOnlyFile m_ReadOnlyFile;` строго по значению внутри `ArchiveReaderBase` (без `std::shared_ptr`, без зависимости от `FileSystem`).
   - `PackageManager` и `DataAssetsManager` наследуют `ArchiveReaderBase` и принимают разрешённый `std::filesystem::path`, исключая прокидывание ссылки на всю структуру `FileSystem`.
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
