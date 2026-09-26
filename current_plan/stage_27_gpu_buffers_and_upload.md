# Этап 27. Подготовка данных меша для будущего GPU upload

**Статус:** 🚀 В работе

> [!IMPORTANT]
> Этот файл — источник истины для этапа 27. В этом этапе реализуются: монтирование трёхсекционного архива `data.dat` и внешних паков `0.dat`, `1.dat` в `DataAssetsManager`, zero-allocation парсер `MeshData` (`MeshPayloadView`), маршрут через `CpuResourceManager` с гарантированным lifetime backing memory, и подготовка данных для будущего GPU upload. Запекание текстур реализовано в **Этапе 25**, сборка архивов и упаковщик — в **Этапе 26**.

---

## Цель текущей подзадачи

Сохранить существующий маршрут запроса:

`GpuResourceManager → CpuResourceManager → DataAssetsManager → data.dat / N.dat`.

`CpuResourceManager` возвращает результат, из которого графический путь получает проверенные метаданные и стабильный указатель/`std::span` на vertex/index payload. Время жизни памяти обязано покрывать всё будущее копирование потребителем.

- Для несжатого payload указатель готов к прямому копированию в staging buffer.
- Для сжатого payload данные сначала должны пройти через unpacker, после чего указатель на распакованные данные передаётся в staging. Codec, checksum и сама компрессия относятся к этапу 49; сейчас payload несжатый и unpacker не реализуется.
- Текущая подзадача не вызывает GAPI и не выдаёт подготовленность CPU-данных за готовность GPU-ресурса.

## Границы

В текущую подзадачу входят:

- монтирование трёхсекционного TOC `data.dat` (Таблица 1: типы внешних паков, Таблица 2: inline-записи, Таблица 3: внешние записи) и внешних паков `N.dat` (`0.dat`, `1.dat`...);
- сопоставление хранилищ `ReadOnlyFile` в `DataAssetsManager`: у `data.dat` и каждого внешнего пака (`eDataDatType`) свой `ReadOnlyFile`;
- получение ресурса только через `CpuResourceManager`, без прямой зависимости `GpuResourceManager → DataAssetsManager`;
- единый zero-allocation парсер layout меша и проверенные vertex/index spans;
- lifetime результата (`PreparedMeshData` / guard) до окончания его использования вызывающей стороной;
- проверки writer ↔ reader, валидация смещений и границ inline/external payload.

Не входят в текущую подзадачу:

- `GPUBuffer`, staging buffer и device-local/default buffer;
- `GpuUploadScheduler`, copy-команды и GPU memory allocation;
- DX12/Vulkan queue synchronization, barriers, ownership transfer и fence;
- настоящий `GpuMesh` с native buffers и семантика `GpuReady`;
- сжатие, unpacker, checksum и выбор codec.

## Архитектурные контракты

### 1. `data.dat` и внешние паки (Формат Этапа 26)

Архив `data.dat` использует три секции TOC:

1. **Таблица 1 (`PackTypesTable`):** компактный массив типов внешних паков `eDataDatType` (`packCount` элементов по 4 байта). Содержит только реально существующие внешние паки (например, `Texture2D` $\to$ `0.dat`, `AudioClip` $\to$ `1.dat`), строго по возрастанию без дубликатов.
2. **Таблица 2 (`InlinePackageEntryTable`):** массив встроенных записей `PackageEntry` (`inlineCount` элементов). Полезная нагрузка размещается внутри `data.dat` со смещением $\text{offset} \ge \text{payloadBegin}$ (выравнивание по 16 байтам). Включает типы `Mesh`, `Material`, `Shader`, `Animation`.
3. **Таблица 3 (`ExternalPackageEntryTable`):** массив внешних записей `PackageEntry` (`externalCount` элементов). Полезная нагрузка размещается в соответствующем внешнем файле `N.dat` со смещением $\text{offset} \ge 4096$ (выравнивание по 16 байтам).

Имя внешнего пака вычисляется детерминированно через `core::GetPakFileName(type)`:
- `Texture2D` $\to$ `"0.dat"`
- `AudioClip` $\to$ `"1.dat"`
- `Video` $\to$ `"2.dat"`
- `Font` $\to$ `"3.dat"`
- `BinaryData` $\to$ `"4.dat"`

Путь к внешнему файлу резолвится через `core::Path::ResolvePakPath(type)`.

`DataAssetsManager` владеет хранилищами `ReadOnlyFile` (для `data.dat` и для каждого внешнего пака по `eDataDatType`). Доступ к файлу выполняется по типу ассета `entry.GetAssetType()`; повторное открытие одного файла не допускается.

Проверяются наличие файлов на диске, соответствие типов секциям, уникальность GUID, диапазон `offset/size` относительно выбранного файла и валидность заголовков (`DataDatHeader` с `"ZDD"` и `PakFileHeader` с `"ZPK"`).

### 2. Записи `PackageEntry` и метаданные

Единая дисковая и runtime-структура `PackageEntry` (68 байт):
- `Guid guid` (16 байт)
- `zU32 assetType` (4 байта: `eDataDatType`)
- `zU64 offset` (8 байт: смещение в `data.dat` для Таблицы 2, в `N.dat` для Таблицы 3)
- `zU64 size` (8 байт: размер полезной нагрузки в байтах)
- `AssetMetadata metadata` (32 байта: union метаданных ресурса)

Для ресурсов геометрии метаданные `MeshMetadata` доступны напрямую из `PackageEntry` без чтения полезной нагрузки:
- `vertexCount`, `indexCount`;
- `vertexStride`, `indexFormat`;
- `vertexOffset`, `indexOffset`.

### 3. Получение данных через CPU-контур

- `GpuResourceManager` запрашивает данные меша исключительно у `CpuResourceManager`.
- `CpuResourceManager` отвечает за поиск записи в `DataAssetsManager`, чтение из соответствующего `ReadOnlyFile`, валидацию layout и возврат результата с метаданными и spans.
- Результат удерживает владельца backing memory (через shared/guard на `ReadOnlyFile`), пока вызывающая сторона работает с данными. Голый указатель без lifetime-контракта запрещён.
- Для текущего несжатого формата vertex/index spans ссылаются непосредственно на проверенный payload и готовы стать источником будущего staging-copy.

### 4. Zero-allocation layout меша

`MeshData` остаётся источником истины для структуры бинарного блоба меша.
Парсер `MeshPayloadView`:
- проверяет `vertexCount`, `vertexStride`, `indexCount`, `indexFormat`;
- использует безопасную арифметику с защитой от переполнений при вычислении размеров;
- валидирует границы: диапазоны вершинных и индексных данных обязаны строго укладываться в переданный span полезной нагрузки без пересечений и без остаточного невалидного хвоста байтов;
- возвращает `std::span<const std::byte>` на вершины и индексы с нулевыми аллокациями в куче.

### 5. Граница готовности

Получение upload-ready spans означает только готовность CPU-источника копирования. Оно не означает, что данные находятся в VRAM или могут использоваться `DrawIndexed`.
Настоящий `ResourceTable<GpuMesh>::Resolve(success)` после GPU fence вводится вместе с будущим upload-кодом. До этого существующие RAM-заглушки не называются `GpuReady`.

---

## План реализации Этапа 27 (разбиение на подшаги)

### Подшаг 27.1. Монтирование трёхсекционного data.dat и внешних паков в DataAssetsManager
- [ ] Расширить `DataAssetsManager` для работы с трёхсекционным TOC:
  * Чтение заголовка `DataDatHeader` (`"ZDD"`, версия `1.0.0`, `headerSize = 64`).
  * Считывание Таблицы 1 (`externalPackTypes`): для каждого типа `eDataDatType` открытие соответствующего файла `N.dat` (`0.dat`, `1.dat`...) через `core::Path::ResolvePakPath(type)` и сохранение `std::unique_ptr<ReadOnlyFile>`.
  * Валидация заголовка каждого открытого внешнего пака (`PakFileHeader`, сигнатура `"ZPK"`, версия `1.0.0`, размер 4096 байт).
  * Считывание Таблицы 2 (`InlinePackageEntryTable`): регистрация записей в `m_Tables` с привязкой к основному файлу `data.dat`.
  * Считывание Таблицы 3 (`ExternalPackageEntryTable`): регистрация записей в `m_Tables` с привязкой к соответствующему `ReadOnlyFile` внешнего пака по `assetType`.
- [ ] Валидация смещений и границ при монтировании:
  * Для inline-записей: $\text{offset} \ge \text{payloadBegin}$ и $\text{offset} + \text{size} \le \text{fileSize}(\texttt{data.dat})$.
  * Для внешних записей: $\text{offset} \ge 4096$ и $\text{offset} + \text{size} \le \text{fileSize}(N\texttt{.dat})$.
  * Проверка на отсутствие взаимных пересечений диапазонов полезной нагрузки внутри каждого файла.

### Подшаг 27.2. Zero-allocation parser/view для MeshData
- [ ] Реализовать легковесный парсер/представление `MeshPayloadView` над бинарным блобом `MeshData`:
  * Парсинг заголовка меша, `vertexCount`, `vertexStride`, `indexCount`, `indexFormat` без динамических аллокаций памяти CPU.
  * Защита от переполнений целочисленной арифметики при расчёте размеров буферов вершин и индексов.
  * Полная валидация границ: проверка того, что диапазоны вершинных данных и индексных данных строго укладываются в переданный span полезной нагрузки без пересечений и без остаточного невалидного хвоста байтов.
  * Предоставление стабильных `std::span<const std::byte>` на вершинные и индексные данные.

### Подшаг 27.3. Сквозной маршрут через CpuResourceManager и Lifetime-контракт
- [ ] Сформировать результат CPU-подготовки данных меша (`PreparedMeshData`):
  * Содержит проверенные метаданные (`vertexCount`, `vertexStride`, `indexCount`, `indexFormat`);
  * Содержит стабильные `std::span<const std::byte>` на вершины и индексы;
  * Гарантирует сохранение времени жизни backing memory (через удержание `ReadOnlyFile` / guard), пока вызывающая сторона владеет объектом `PreparedMeshData`.
- [ ] Маршрут запроса: `GpuResourceManager → CpuResourceManager → DataAssetsManager`.
  * `GpuResourceManager` запрашивает подготовленные данные у `CpuResourceManager`.
  * Прямое обращение `GpuResourceManager → DataAssetsManager` запрещено архитектурным контрактом.
  * Данные возвращаются готовыми к будущему прямому копированию в staging buffer (без вызова GAPI на текущем этапе).

### Подшаг 27.4. Комплексная верификация и интеграционные проверки
- [ ] Проверка чтения реального пакета, собранного сборщиком:
  * Проверка чтения встроенных мешей (например, куба из `zzz_assets_test_000`) из `data.dat`.
  * Проверка доступа к внешним текстурам из `0.dat` и аудио из `1.dat`.
  * Валидация прямого $O(1)$ поиска записей по GUID и чтение полезной нагрузки из корректного `ReadOnlyFile`.
- [ ] Верификация защиты от некорректных данных:
  * Отсутствующий файл внешнего пака на диске;
  * Неизвестный или невалидный `eDataDatType` в Таблице 1 или Таблице 3;
  * Выход диапазонов `offset/size` за границы соответствующего файла;
  * Повреждённый layout меша (`vertexStride == 0`, битый формат индекса, переполнение размера, лишние байты в хвосте).

---

## Будущее продолжение (последующие этапы)

После завершения текущей подзадачи отдельно реализуются:

- GPU/staging buffers;
- копирование несжатого span напрямую в staging и распакованного span после unpacker;
- DX12/Vulkan submit, barriers и fence;
- lifetime staging и ограничение памяти;
- настоящий `GpuMesh` и `GpuReady` только после завершения GPU-команд.

---

## Критерии приёмки этапа 27

1. Запрос графического ресурса получает данные через `CpuResourceManager`, а не обращается к `DataAssetsManager` напрямую.
2. Для mesh доступны проверенные метаданные и стабильные vertex/index spans с явным lifetime-контрактом.
3. Несжатый payload готов к будущему прямому копированию в staging без повторного парсинга.
4. Монтирование `data.dat` открывает внешние паки `N.dat` по типам из Таблицы 1; inline и external ranges валидируются относительно правильного `ReadOnlyFile`.
5. Writer и reader одинаково понимают трёхсекционный layout `data.dat` и заголовки `PakFileHeader`.
6. В рамках этапа 27 не добавляются GPU buffers, submit, fence или фиктивная семантика `GpuReady`.
