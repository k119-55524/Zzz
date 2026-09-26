# Этап 25. Библиотека запекания и компрессии текстур (`texture_processor_lib`)

**Статус:** ⏳ В процессе

> [!IMPORTANT]
> Этот файл — источник истины для этапа 25. Этап целиком посвящён созданию **отдельной библиотеки обработки и компрессии текстур `texture_processor_lib`**. Упаковка в архивы `0004.pak` и `data.dat` выполняется в **Этапе 26**, а чтение архивов и GPU Upload — в **Этапе 27**.

---

## 🎯 Цель этапа

1. Создать отдельную кроссплатформенную библиотеку **`texture_processor_lib`** (в каталоге `src/tools/texture_processor/`).
2. Подключить через CMake `FetchContent`:
   - **`stb` (`stb_image.h`, `stb_image_resize2.h`):** декодирование исходных форматов художников (PNG, JPG, TGA, BMP, HDR) в память `RGBA8` и качественная генерация цепочки мипмапов;
   - **`DirectXTex` (Microsoft):** профессиональное аппаратное сжатие в форматы **BC7** (цвет), **BC5** (нормали), **BC4** (маски) под Windows и Linux;
   - Подготовить структуру для интеграции **`astc-encoder` (ARM)** под Android/iOS.
3. Реализовать строгое правило валидации: **стороны текстуры обязаны быть кратны 2** (`width % 2 == 0 && height % 2 == 0`), иначе выбрасывается ошибка с указанием имени файла и размеров.
4. Сформировать бинарный блоб текстуры с заголовком **`TextureBlobHeader` (ровно 64 байта)** в `src/core/io/package/assets/texture/TextureBlobHeader.h`:
   - Метаданные: `width`, `height`, `depth`, `arraySize`, `mipCount`, `ePixelFormat`, `eTextureType`, `flags`;
   - Выравнивание 16 байт для полезной нагрузки мипов (`dataOffset = 64`);
   - Непрерывная укладка мип-уровней (Mip 0..Mip N), готовая к заливке в GPU Staging Buffer без декомпрессии на CPU.
5. Реализовать C++ класс `TextureProcessor` с публичным API:
   ```cpp
   struct TextureProcessInput
   {
       std::span<const std::byte> sourceData;
       std::string                assetName;
       core::eTargetPlatform      targetPlatform{ core::eTargetPlatform::Windows };
       TextureImportSettings      settings;
   };

   struct ProcessedTextureBlob
   {
       TextureBlobHeader          header;
       std::vector<std::byte>     blobData; // 64B заголовок + 16B-выровненные мипы
   };

   class TextureProcessor
   {
   public:
       static std::expected<ProcessedTextureBlob, std::string> Process(const TextureProcessInput& input);
   };
   ```
6. Написать юнит-тесты в `src/qa/tests/tools/TextureProcessorTests.cpp`:
   - Проверка ошибки на нечётных размерах (например, $513 \times 256$);
   - Проверка загрузки PNG в `RGBA8`;
   - Проверка генерации цепочки мип-уровней до $1\times 1$;
   - Проверка сжатия в `BC7` через `DirectXTex` и валидности `TextureBlobHeader` (64 байта, выравнивание 16 байт).

---

## 📐 Архитектурные спецификации

### 1. Заголовок блоба `TextureBlobHeader` (64 байта)
Располагается в `src/core/io/package/assets/texture/TextureBlobHeader.h`:
```cpp
namespace zzz::core
{
    enum class eTextureType : uint8_t
    {
        Texture2D = 0,
        TextureCube = 1,  // 6 граней
        Texture3D = 2,    // Объемная
        Texture2DArray = 3
    };

    struct TextureBlobHeader
    {
        // 1. Размеры текстуры (16 байт)
        uint32_t width{ 0 };        // Ширина в пикселях (кратна 2)
        uint32_t height{ 0 };       // Высота в пикселях (кратна 2)
        uint32_t depth{ 1 };        // Глубина (1 для 2D)
        uint32_t arraySize{ 1 };    // 1 для 2D, 6 для Cube

        // 2. Формат и мипы (8 байт)
        uint32_t     mipCount{ 1 };            // Кол-во мип-уровней (от 1 до 16)
        ePixelFormat format{ ePixelFormat::Unknown }; // 2 байта (BC7, BC5, RGBA8, etc.)
        eTextureType textureType{ eTextureType::Texture2D }; // 1 байт
        uint8_t      flags{ 0 };               // 1 байт (бит 0: sRGB, бит 1: IsNormalMap)

        // 3. Смещение и размер данных (8 байт)
        uint32_t dataOffset{ 64 };  // Смещение пикселей от начала блоба (кратно 16, = 64)
        uint32_t dataSize{ 0 };     // Полный размер всех мипов в байтах (кратно 16)

        // 4. Резерв / выравнивание заголовка до 64 байт (32 байта)
        uint8_t reserved[32]{ 0 };  // Задел под тайлинг, стриминг и сжатие
    };
    static_assert(sizeof(TextureBlobHeader) == 64, "TextureBlobHeader must be exactly 64 bytes");
}
```

### 2. Настройки импорта текстуры (`TextureImportSettings`)
Считываются предсборщиком из `.meta` файла ассета:
```cpp
namespace zzz::builder
{
    enum class eTextureCompressionMode : uint8_t
    {
        Auto = 0,   // Выбор формата по targetPlatform (BC7/BC5 для ПК, ASTC для мобилок)
        None = 1,   // Без сжатия (сырой RGBA8 / R8)
        BC7 = 2,    // Принудительно BC7
        BC5 = 3,    // Принудительно BC5 (для Normal Map)
        ASTC = 4    // Принудительно ASTC
    };

    enum class eTextureQuality : uint8_t
    {
        Fast = 0,       // Быстрое сжатие для разработки и тестов
        Normal = 1,     // Сбалансированное
        Production = 2  // Максимальное качество для финального релиза
    };

    struct TextureImportSettings
    {
        bool                     sRGB{ true };
        bool                     generateMips{ true };
        bool                     isNormalMap{ false };
        eTextureCompressionMode  compression{ eTextureCompressionMode::Auto };
        eTextureQuality          quality{ eTextureQuality::Normal };
        uint32_t                 maxSize{ 4096 };
    };
}
```

### 3. Библиотеки и зависимости (CMake FetchContent)
- **`stb` (Sean Barrett):**
  * `stb_image.h` — декодирование PNG/JPG/TGA/BMP/HDR в память `RGBA8`;
  * `stb_image_resize2.h` — качественный даунскейл для генерации мипмапов с гамма-коррекцией sRGB.
- **`DirectXTex` (Microsoft):**
  * Компрессия в `BC7_UNORM`, `BC7_UNORM_SRGB`, `BC5_UNORM`, `BC4_UNORM` с многопоточностью.
- **Лицензирование:**
  * Соблюдение правил MIT (`stb`, `DirectXTex`) и Apache 2.0: текст лицензий зафиксирован в `docs/THIRD_PARTY_LICENSES.md` и отображается по Правилу 20 `RULES.md`.

---

## 🛠️ План реализации этапа 25

### Шаг 25.1. Структура TextureBlobHeader и ePixelFormat утилиты
- [ ] Создать `src/core/io/package/assets/texture/TextureBlobHeader.h` (64 байта).
- [ ] Добавить статические проверки `static_assert` на размер и выравнивание.

### Шаг 25.2. Создание CMake-таргета `texture_processor_lib` и FetchContent
- [ ] Создать каталог `src/tools/texture_processor/` и `src/tools/texture_processor/CMakeLists.txt`.
- [ ] Подключить `stb` через `FetchContent`.
- [ ] Подключить `DirectXTex` через `FetchContent` (с опциями сборки без устаревших утилит).
- [ ] Связать `texture_processor_lib` с `core_lib`.

### Шаг 25.3. Реализация TextureProcessor
- [ ] Реализовать декодирование исходных байтов через `stb_image` в буфер `RGBA8`.
- [ ] Реализовать валидацию: проверка кратности 2 для ширины и высоты.
- [ ] Реализовать генерацию цепочки мипмапов через `stb_image_resize2` (или `DirectXTex::GenerateMipMaps`).
- [ ] Реализовать сжатие в целевой формат:
  * Ветка `None` $\to$ запись сырых `RGBA8` мипов с выравниванием по 16 байтам;
  * Ветка `BC7` / `BC5` (для Windows/Linux) $\to$ вызов `DirectXTex::Compress` с заданным качеством (`Fast`, `Normal`, `Production`);
  * Подготовка структуры-заглушки для `ASTC` под мобильные платформы.
- [ ] Сборка единого бинарного буфера: 64B заголовок + непрерывный блок мипмапов.

### Шаг 25.4. Юнит-тесты и верификация
- [ ] Создать тесты в `src/qa/tests/tools/TextureProcessorTests.cpp`:
  * Тест на отказ при нечётных размерах;
  * Тест импорта тестового PNG (генерация мипов, проверка размеров и формата);
  * Тест сжатия в BC7 (проверка валидности `TextureBlobHeader`, соответствия размеров блоков 4x4, выравнивания 16 байт).

---

## 🏆 Критерии приёмки этапа 25

1. Библиотека `texture_processor_lib` успешно компилируется через CMake со всеми зависимостями.
2. Текстуры с нечётными сторонами гарантированно отклоняются с понятной ошибкой.
3. Поддерживается генерация цепочки мипмапов вплоть до $1\times 1$.
4. Поддерживается сжатие в нативный формат видеокарт ПК (**BC7** для цвета, **BC5** для нормалей) и сырой **RGBA8**.
5. На выходе формируется валидный бинарный блоб с 64-байтным заголовком `TextureBlobHeader`, готовый к упаковке в `0004.pak` на следующем этапе.
6. 100% юнит-тестов проходят успешно.
