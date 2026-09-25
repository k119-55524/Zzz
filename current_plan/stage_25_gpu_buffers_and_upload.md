# Этап 25. Подготовка данных меша для будущего GPU upload

**Статус:** ⏳ В процессе

> [!IMPORTANT]
> Этот файл — источник истины для этапа 25. Текущая подзадача заканчивается получением проверенных данных, готовых для помещения в staging. Создание GPU-буферов, копирование в staging/device-local memory, submit, barriers, fence и настоящий `GpuReady` — будущее продолжение этапа, не текущая реализация.

---

## Цель текущей подзадачи

Сохранить существующий маршрут запроса:

`GpuResourceManager → CpuResourceManager → DataAssetsManager → data.dat/.pak`.

`CpuResourceManager` возвращает результат, из которого графический путь получает проверенные метаданные и стабильный указатель/`std::span` на vertex/index payload. Время жизни памяти обязано покрывать всё будущее копирование потребителем.

- Для несжатого payload указатель готов к прямому копированию в staging buffer.
- Для сжатого payload данные сначала должны пройти через unpacker, после чего указатель на распакованные данные передаётся в staging. Codec, checksum и сама компрессия относятся к этапу 49; сейчас payload несжатый и unpacker не реализуется.
- Текущая подзадача не вызывает GAPI и не выдаёт подготовленность CPU-данных за готовность GPU-ресурса.

## Границы

В текущую подзадачу входят:

- трёхсекционный TOC `data.dat` и внешние `{Guid}.pak`;
- индексируемый массив файлов в `DataAssetsManager`: у `data.dat` и каждого `.pak` свой `ReadOnlyFile`;
- получение ресурса только через `CpuResourceManager`, без прямой зависимости `GpuResourceManager → DataAssetsManager`;
- единый парсер layout меша и проверенные vertex/index spans;
- lifetime результата до окончания его использования вызывающей стороной;
- проверки writer↔reader и inline/external payload.

Не входят в текущую подзадачу:

- `GPUBuffer`, staging buffer и device-local/default buffer;
- `GpuUploadScheduler`, copy-команды и GPU memory allocation;
- DX12/Vulkan queue synchronization, barriers, ownership transfer и fence;
- настоящий `GpuMesh` с native buffers и семантика `GpuReady`;
- сжатие, unpacker, checksum и выбор codec.

## Архитектурные контракты

### 1. `data.dat` и внешние паки

`data.dat` использует три секции TOC:

1. `PackManifestTable`: `[0]` обозначает `data.dat`, `[1..N]` — внешние `{Guid}.pak`.
2. `InlinePackageEntryTable`: payload внутри `data.dat`, неявный `packIndex = 0`.
3. `ExternalPackageEntryTable`: payload во внешнем `.pak`, сериализованный `packIndex` находится в диапазоне `[1, packCount)`.

`DataAssetsManager` владеет индексируемым массивом хранилищ. Массив хранит владельцев `ReadOnlyFile`, а не сами non-copyable/non-movable объекты по значению. Доступ выполняется по `packIndex`; повторный mapping одного файла не допускается.

Проверяются GUID паков, уникальность, `packIndex`, наличие файлов и диапазон `offset/size` относительно выбранного storage. Внешний `.pak` создаётся до публикации ссылающегося на него `data.dat`.

`data.dat` ещё находится в разработке. Новый layout заменяет прежний черновик в той же версии: номер версии не меняется, отдельная миграция черновика не поддерживается, writer/reader/test packages обновляются вместе.

### 2. Получение данных через CPU-контур

- `GpuResourceManager` продолжает запрашивать данные у `CpuResourceManager`.
- `CpuResourceManager` отвечает за поиск записи, выбор `ReadOnlyFile`, валидацию layout и возврат результата с метаданными и указателями/spans.
- Результат удерживает владельца backing memory либо сам владеет распакованным буфером; голый указатель без lifetime-контракта запрещён.
- Для текущего несжатого формата vertex/index spans ссылаются на проверенный payload и готовы стать источником будущего staging-copy.
- Наличие будущей ветки unpacker не является основанием писать codec или универсальную систему распаковки сейчас.

### 3. Единый layout меша

`MeshData` остаётся источником истины для упаковки и CPU-десериализации. GPU-потребителю нужен zero-allocation view/parser над тем же layout, а не второй независимый `MeshResourceHeader`.

Парсер проверяет stride/count/index format, переполнения при вычислении размеров, границы vertex/index областей и отсутствие пересечений или необъяснённого хвоста.

### 4. Граница готовности

Получение upload-ready spans означает только готовность CPU-источника копирования. Оно не означает, что данные находятся в VRAM или могут использоваться `DrawIndexed`.

Настоящий `ResourceTable<GpuMesh>::Resolve(success)` после GPU fence вводится вместе с будущим upload-кодом. До этого существующие RAM-заглушки не называются `GpuReady`.

---

## План текущей реализации (разбиение на подшаги)

### Подшаг 25.1. Трёхсекционный TOC `data.dat` и структуры данных
- [ ] Обновить `DatFileHeader`:
  * Для `c_DataDatFormat` вместо одного `entryCount` хранить три счётчика: `packCount` (кол-во записей манифеста паков), `inlineCount` (кол-во встроенных ресурсов), `externalCount` (кол-во внешних ресурсов).
  * Для `c_PackageDatFormat` заголовок остаётся неизменным (`entryCount`).
  * Номер версии формата сохраняется (in-place обновление черновика).
- [ ] Реализовать структуры записей в `src/core/io/package/`:
  * Дисковая Таблица 1: Массив GUID внешних файлов-пакетов (`[0] = Guid::Empty()`, `[1..N] = Guid` паков `{Guid}.pak`).
  * Дисковая Таблица 2 (`InlinePackageEntry`): локальные ресурсы `data.dat` (без `packIndex`, 52 байта).
  * Дисковая Таблица 3 (`ExternalPackageEntry`): внешние ресурсы (с полем `packIndex`, 52 байта).
  * In-memory структура (`DataPackageEntry`): единая компактная структура для runtime-таблиц `m_Tables` с полем `packIndex`.

### Подшаг 25.2. Обновление ArchiveWriter и PackagePacker
- [ ] Расширить `ArchiveWriter` для поддержки формирования трёхсекционного `data.dat` и внешних файлов `{Guid}.pak`.
- [ ] Адаптировать `PackagePacker`: разделение ресурсов на встроенные (`data.dat`, `packIndex = 0`) и внешние (`.pak`, `packIndex = 1..N`).
- [ ] Синхронизировать запись и выравнивание полезной нагрузки (4 КБ для начала файлов / payload area, 16 байт для записей).

### Подшаг 25.3. Монтирование и индексация в ArchiveReaderBase и DataAssetsManager
- [ ] Параметризовать `ArchiveReaderBase` типом записи через `ArchiveTraits<TType>::EntryType`:
  * `ArchiveTraits<ePackageDatType>::EntryType = PackageEntry` (36 байт, без изменений).
  * `ArchiveTraits<eDataDatType>::EntryType = DataPackageEntry`.
- [ ] Добавить в `DataAssetsManager` индексируемый массив владельцев хранилищ:
  `std::vector<std::unique_ptr<ReadOnlyFile>> m_PackageFiles`.
- [ ] При монтировании `data.dat`:
  * Считывать Таблицу 1; индекс `[0]` инициализировать открытым `data.dat`.
  * Для внешних паков `[1..N]` открывать соответствующие `{Guid}.pak` рядом с `data.dat` и сохранять в `m_PackageFiles[packIndex]`.
  * Валидировать наличие всех файлов, уникальность GUID и корректность `packIndex`.
  * Считывать Таблицу 2 (Inline) и нормализовать в `m_Tables` с `packIndex = 0`.
  * Считывать Таблицу 3 (External) и нормализовать в `m_Tables` с `packIndex` из файла.
- [ ] Обновить `DataAssetsManager::ReadRawPayload(entry)`: прямое чтение `m_PackageFiles[entry.GetPackIndex()]->Read(entry.GetOffset(), entry.GetStoredSize())`.

### Подшаг 25.4. Zero-allocation parser/view для MeshData
- [ ] Реализовать легковесный парсер/представление `MeshPayloadView` над бинарным layout `MeshData`:
  * Парсинг `vertexCount`, `vertexStride`, `indexCount`, `indexFormat` без динамических аллокаций памяти CPU.
  * Защита от переполнений (`SafeMath` / `CheckedMul`).
  * Полная валидация границ: проверка того, что диапазоны `vertexData` и `indexData` строго укладываются в переданный span полезной нагрузки без пересечений и без остаточного невалидного хвоста байтов.
  * Предоставление стабильных `std::span<const std::byte>` на вершинные и индексные данные.

### Подшаг 25.5. Сквозной маршрут через CpuResourceManager и Lifetime-контракт
- [ ] Сформировать CPU-результат подготовки данных меша (например, `PreparedMeshData` или расширение `CpuMesh`), который:
  * Содержит проверенные метаданные (`vertexCount`, `vertexStride`, `indexCount`, `indexFormat`);
  * Содержит стабильные `std::span` на вершины и индексы;
  * Гарантирует сохранение времени жизни backing memory (через удержание `ReadOnlyFile` / guard), пока вызывающая сторона владеет результатом.
- [ ] Маршрут запроса: `GpuResourceManager → CpuResourceManager → DataAssetsManager`.
  * `GpuResourceManager` запрашивает подготовленные данные у `CpuResourceManager`.
  * Запрещено прямое обращение `GpuResourceManager → DataAssetsManager`.
  * Данные возвращаются готовыми к будущему прямому копированию в staging buffer (без вызова GAPI на текущем подшаге).

### Подшаг 25.6. Тестовые пакеты и комплексная верификация
- [ ] Сформировать тестовые пакеты (сценарий `data.dat` + внешние `.pak` с текстурами/данными).
- [ ] Реализовать тесты в `EngineTests` (`SerializationTests` / `ResourceTests`):
  * Консистентность writer ↔ reader на новом трёхсекционном TOC;
  * Сквозное чтение inline-ресурса (`data.dat`, `packIndex = 0`) и внешних ресурсов (`{Guid}.pak`, `packIndex = 1..N`);
  * Проверка $O(1)$ прямого доступа по `packIndex` без повторных поисков по имени/GUID пака;
  * Проверка валидации и парсинга `MeshPayloadView` на валидном меше;
  * Edge cases:
    - Отсутствующий файл внешнего пака на диске;
    - Недопустимый `packIndex` (0 в таблице внешних ресурсов или $\ge packCount$);
    - Выход диапазонов `offset/size` за границы соответствующего файла;
    - Повреждённый layout меша (`vertexStride == 0`, битый формат индекса, переполнение размера, лишние байты в хвосте).

## Будущее продолжение этапа

После завершения текущей подзадачи отдельно реализуются:

- GPU/staging buffers;
- копирование несжатого span напрямую в staging и распакованного span после unpacker;
- DX12/Vulkan submit, barriers и fence;
- lifetime staging и ограничение памяти;
- настоящий `GpuMesh` и `GpuReady` только после завершения GPU-команд.

Детальные backend-решения фиксируются перед написанием соответствующего кода, а не в текущей подзадаче.

---

## Критерии приёмки текущей подзадачи

1. Запрос графического ресурса получает данные через `CpuResourceManager`, а не обращается к `DataAssetsManager` напрямую.
2. Для mesh доступны проверенные метаданные и стабильные vertex/index pointers/spans с явным lifetime.
3. Несжатый payload готов к будущему прямому копированию в staging без повторного парсинга.
4. Каждый `packIndex` выбирает свой `ReadOnlyFile`; inline и external ranges валидируются относительно правильного файла.
5. Writer и reader одинаково понимают новый layout при неизменной версии.
6. В рамках текущей подзадачи не добавлены GPU buffers, submit, fence или фиктивная семантика `GpuReady`.
