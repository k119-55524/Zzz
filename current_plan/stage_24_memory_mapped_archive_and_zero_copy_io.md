# Этап 24. Постоянный archive handle (Memory-Mapped File) и Zero-Copy I/O конвейер архивов

---

## 🎯 Цель этапа

1. **Ликвидация системных вызовов в горячем пути загрузки ассетов:**
   - Устранить цепочку `file_size + open + seek + read + close` на каждый загружаемый ресурс.
   - Исключить обращение к файловой системе для проверки размера файла: размер архива фиксируется один раз при монтировании (`m_ArchiveSize`), а валидация диапазона `IsRangeInside` выполняется как мгновенная проверка в памяти.
   - Заменить открытие файла на каждый чих постоянным отображением в виртуальную память (Memory-Mapped File).

2. **Desktop RAII-примитив `MemoryMappedFile` с сохранением мобильного fallback:**
   - Реализовать `MemoryMappedFile` в `src/core/io/` с чистой поддержкой desktop-платформ:
     * **Windows (Win32):** `CreateFileW` (с флагами `GENERIC_READ`, `FILE_SHARE_READ`), `CreateFileMappingW` (`PAGE_READONLY`), `MapViewOfFile` (`FILE_MAP_READ`).
     * **POSIX (Linux, macOS):** `open` (`O_RDONLY`), `fstat`, `mmap` (`PROT_READ`, `MAP_SHARED`), закрытие `fd`.
   - **Android/iOS:** сохранить существующий ranged-read через платформенный `FileSystem`. Android mmap/noCompress отложен в `TODO.md` до отдельного этапа работ над Android; текущий этап не должен ухудшать мобильный контракт и не должен загружать весь архив в RAM.
   - Методы:
     * `[[nodiscard]] std::span<const std::byte> GetSpan() const noexcept;`
     * `[[nodiscard]] std::span<const std::byte> Subspan(std::size_t offset, std::size_t size) const noexcept;`
     * `[[nodiscard]] std::size_t GetSize() const noexcept;`
     * `[[nodiscard]] bool IsValid() const noexcept;`

3. **Интеграция в `PackageArchive<TType>`:**
   - Хранение постоянного `std::shared_ptr<MemoryMappedFile>` внутри `PackageArchive`: возвращённый payload удерживает mapping и не становится висячим после уничтожения менеджера.
   - В `InitializeArchive`: маппинг архива один раз при старте; чтение заголовка и оглавления прямо из `std::span` mapped-памяти без вызовов `m_FileSystem->ReadBytes`.
   - Добавление `ReadRawPayload`, возвращающего самодостаточный `ArchivePayload`: borrowed span с владельцем mmap на desktop либо owned vector мобильного fallback.
   - Сохранение `ReadRawBytes(entry)` как совместимой копирующей обёртки только для потребителей, которым действительно нужен `std::vector<std::byte>`.

4. **Zero-Copy конвейер в `CpuResourceManager`:**
   - В worker-задаче `CpuResourceManager::GetAsync` исключить прямую зависимость от `FileSystem` и аллокацию промежуточного буфера на desktop.
   - Получение `ArchivePayload` напрямую из `DataAssetsManager::ReadRawPayload(entry)`.
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

### 2. Подсистема архивов пакетов (`PackageArchive<TType>`)
- [x] Добавить постоянный mapping и `std::size_t m_ArchiveSize` в `PackageArchive`.
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
