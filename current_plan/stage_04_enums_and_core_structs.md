# Этап 04: Базовые перечисления (Enums) и структуры ядра (`eResourceType`, `ePixelFormat`, `eIndexFormat`, `ConverterGAPITypes`, `eVertexSemantic`, `Vertex3D`, `AttributeRange`, выравнивание CBV 256б, `EnumToString`)

## 1. Контекст и цели этапа
- **Цель:** Заложить фундаментальные перечисления, форматы индексов и пикселей, базовую геометрию, кроссплатформенные конвертеры GAPI и легковесный интерфейс чтения атрибутов вершин, связывающие математический базис (Векторы, Матрицы, Кватернионы, Цвет) с низкоуровневым GAPI-конвейером (DirectX 12, Vulkan, Metal), файловой подсистемой VFS и системой сериализации ассетов.
- **Статус:** `✅ Выполнено`
- **Зависимости:** `src/math/Math.h`, `src/math/utils/Types.h`, `src/math/utils/color/Color.h`, `src/core/utils/Defines.h`, `src/core/serialize/Serializer.h`, `src/core/enums/eEnumToString.h`, `src/core/enums/platforms/`.

> [!NOTE]
> **Структура каталогов `src/math/utils/` (зафиксировано на Этапе 03):**  
> Базовые типы, скалярные утилиты и цвет сгруппированы в `src/math/utils/` (`Types.h`, `MathConstants.h`, `MathUtils.h`, `color/Color.h`, `color/Color3.h`, `color/Color4.h`, `color/Color3Palette.h`, `color/Color4Palette.h`). В корне `src/math/` сохранены forwarder-заголовки `Types.h` и `Color.h` для полной обратной совместимости.

---

## 2. Архитектурные требования и проектирование

### 2.1. Перечисление типов ресурсов `eResourceType` (`src/core/enums/eResourceType.h`)
Служит единым 1-байтовым идентификатором типа ассета в мета-файлах `.meta`, кэше ассетов и VFS:
```cpp
enum class eResourceType : zU8
{
    Unknown = 0,
    Texture2D,      ///< 2D Текстура (DDS, PNG, JPEG, RGBA)
    Mesh,           ///< 3D Сетка/Геометрия (Vertex/Index buffers, Submeshes)
    Material,       ///< Материал (.zmat)
    Shader,         ///< Шейдер (скомпилированный байткод DXIL / SPIR-V / MetalLib)
    AudioClip,      ///< Аудиофайл (WAV, OGG, MP3)
    Font,           ///< Шрифт (TTF, OTF, SDF/MSDF текстурные атласы)
    Scene,          ///< Сцена (дерево GameObject, компоненты)
    Prefab,         ///< Префаб объекта
    BinaryData      ///< Произвольный бинарный буфер
};
```

> [!IMPORTANT]
> **Разграничение `ePackage` и `eResourceType` (Правило 22):**
> - **`ePackage` (`zU32`, `src/core/enums/ePackage.h`)** — классифицирует **контейнеры и файлы пакетов верхнего уровня** (`ProjectManifest`, `Scene`, `PrimaryView`, `ChildView`, `IndependentView`, `Prefab`, `BinaryAsset`), с которыми работает менеджер пакетов `PackageManager`.
> - **`eResourceType` (`zU8`, `src/core/enums/eResourceType.h`)** — классифицирует **гранулярные типы контента/ассетов** внутри пакетов и ресурсов движка (`Texture2D`, `Mesh`, `Material`, `Shader`, `Font`, `AudioClip`).
> 
> Эти перечисления не дублируют друг друга, а представляют два разных уровня абстракции (контейнер пакета vs единичный ассет).

---

### 2.2. Перечисление пиксельных форматов `ePixelFormat` (`src/core/enums/ePixelFormat.h`)
Унифицированный кроссплатформенный формат для текстур, буферов кадра (RenderTarget, Swapchain, DepthStencil) и атрибутов вершин (на 100% поддержан в DX12, Vulkan, Metal):
```cpp
enum class ePixelFormat : zU16
{
    Unknown = 0,

    // --- 1. Универсальные 8-битные цветовые форматы (100% везде) ---
    R8_UNORM,           ///< 1 байт: монохромные маски, шрифты, альфа
    RGBA8_UNORM,        ///< 4 байта: стандартные текстуры (DX12 / Vulkan / Metal)
    RGBA8_SRGB,         ///< 4 байта: текстуры с гамма-коррекцией sRGB
    BGRA8_UNORM,        ///< 4 байта: нативный формат Swapchain (Windows, Android, Apple)
    BGRA8_SRGB,         ///< 4 байта: sRGB Swapchain

    // --- 2. HDR и данные с плавающей запятой (100% везде) ---
    RGBA16_FLOAT,       ///< 8 байт: HDR буферы кадра, постобработка
    R32_FLOAT,          ///< 4 байта: скалярные данные, карты высот

    // --- 3. Буферы глубины и трафарета (100% везде) ---
    D32_FLOAT,          ///< 32-бит глубина (универсальный стандарт)
    D24_UNORM_S8_UINT,  ///< 24-бит глубина + 8-бит трафарет (дефолт DX12)
    D32_FLOAT_S8_UINT,  ///< 32-бит float глубина + 8-бит трафарет (дефолт Vulkan/Metal: VK_FORMAT_D32_SFLOAT_S8_UINT / MTLPixelFormatDepth32Float_Stencil8)

    // --- 4. Сжатые форматы Desktop (Windows / Linux / macOS) ---
    BC1_UNORM,          ///< DXT1 (RGB без альфы / 1-бит альфа)
    BC3_UNORM,          ///< DXT5 (RGBA с альфой)
    BC7_UNORM,          ///< Высококачественное сжатие RGBA

    // --- 5. Сжатые форматы Mobile (Android / iOS) ---
    ASTC_4x4_UNORM,     ///< Универсальный мобильный стандарт качества (Android + iOS)
    ETC2_RGBA8_UNORM    ///< Базовый мобильный стандарт OpenGL ES 3.0 / Vulkan
};
```
**Вспомогательные constexpr-функции (`PixelFormatUtils`):**
- `[[nodiscard]] constexpr zU32 GetPixelFormatBytesPerPixel(ePixelFormat format) noexcept;`
- `[[nodiscard]] constexpr bool IsDepthFormat(ePixelFormat format) noexcept;`
- `[[nodiscard]] constexpr bool IsStencilFormat(ePixelFormat format) noexcept;`
- `[[nodiscard]] constexpr bool IsSRGBFormat(ePixelFormat format) noexcept;`
- `[[nodiscard]] constexpr bool IsCompressedFormat(ePixelFormat format) noexcept;`

---

### 2.3. Перечисление форматов индексов `eIndexFormat` (`src/core/enums/eIndexFormat.h`)
Определяет размерность элементов индексного буфера (`IIndexBuffer`):
```cpp
enum class eIndexFormat : zU8
{
    UInt16 = 0, ///< 2 байта (zU16): до 65 535 вершин — для куба, UI и базовых мешей (экономит 50% VRAM)
    UInt32 = 1  ///< 4 байта (zU32): для высокополигональных мешей (> 65k вершин)
};

namespace IndexFormatUtils
{
    [[nodiscard]] constexpr zU32 GetIndexFormatBytes(eIndexFormat format) noexcept
    {
        return (format == eIndexFormat::UInt16) ? sizeof(zU16) : sizeof(zU32);
    }
}
```

---

### 2.4. Кроссплатформенный конвертер форматов `ConverterGAPITypes` (`src/core/enums/platforms/ConverterGAPITypes.h`)

Размещается в `src/core/enums/platforms/` в едином стиле с `ConverterMSWinTypes.h`. Использует стандартные макросы движка из `src/core/utils/Defines.h` (`Z_D3D12`, `Z_VULKAN`, `Z_METAL`), при этом все псевдонимы типов строго инкапсулированы в `namespace zzz::core`.

> [!NOTE]
> **Именование методов `ToEngine`:**
> На D3D12 типы `NativePixelFormat` и `NativeIndexFormat` совпадают (оба равны `DXGI_FORMAT`). Перегрузка одной функции `ToEngine(DXGI_FORMAT)` с разными типами возврата запрещена стандартом C++. Поэтому методы имеют явные имена `ToEnginePixelFormat` и `ToEngineIndexFormat`, а для шаблонного обобщенного вызова предоставляется `ToEngine<T>(nativeFormat)`.

```cpp
#pragma once

#include <type_traits>
#include "core/utils/Defines.h"
#include "core/enums/ePixelFormat.h"
#include "core/enums/eIndexFormat.h"

#if Z_D3D12
    #include <dxgiformat.h>
#elif Z_VULKAN
    #include <vulkan/vulkan.h>
#elif Z_METAL
    #import <Metal/Metal.h>
#endif

namespace zzz::core
{
#if Z_D3D12
    using NativePixelFormat = DXGI_FORMAT;
    using NativeIndexFormat = DXGI_FORMAT;
#elif Z_VULKAN
    using NativePixelFormat = VkFormat;
    using NativeIndexFormat = VkIndexType;
#elif Z_METAL
    using NativePixelFormat = MTLPixelFormat;
    using NativeIndexFormat = MTLIndexType;
#else
    using NativePixelFormat = zU32;
    using NativeIndexFormat = zU32;
#endif

    class ConverterGAPITypes final
    {
    public:
        ConverterGAPITypes() = delete;

        // ePixelFormat <-> NativePixelFormat
        [[nodiscard]] static constexpr NativePixelFormat ToNative(ePixelFormat format) noexcept;
        [[nodiscard]] static constexpr ePixelFormat ToEnginePixelFormat(NativePixelFormat nativeFormat) noexcept;

        // eIndexFormat <-> NativeIndexFormat
        [[nodiscard]] static constexpr NativeIndexFormat ToNative(eIndexFormat format) noexcept;
        [[nodiscard]] static constexpr eIndexFormat ToEngineIndexFormat(NativeIndexFormat nativeFormat) noexcept;

        // Обобщенный шаблонный конвертер
        template<typename T>
        [[nodiscard]] static constexpr T ToEngine(auto nativeFormat) noexcept;
    };
}
```

---

### 2.5. Семантики атрибутов и канонический `Vertex3D` (`src/math/geometry/Vertex3D.h`)

> [!NOTE]
> **Прямое использование `Color4<zF32>`:**
> Для предотвращения раздувания типов движка алиасы цвета не вводятся — используется канонический шаблон `Color4<zF32>` (Palette4::White).

#### `eVertexSemantic` (`src/core/enums/eVertexSemantic.h`):
```cpp
enum class eVertexSemantic : zU8
{
    Position,       ///< Позиция вершины (X, Y, Z)
    Normal,         ///< Нормаль поверхности (NX, NY, NZ)
    TexCoord,       ///< Текстурные координаты (U, V)
    Color,          ///< Цвет вершины (RGBA)
    Tangent,        ///< Касательный вектор (XYZ + W знак бинормали)
    Bitangent,      ///< Бикасательный вектор / Бинормаль
    BlendWeight,    ///< Веса костей для скелетной анимации
    BlendIndices,   ///< Индексы костей
    Count           ///< Количество семантик для быстрого фиксированного массива
};
```

#### Каноническая 3D вершина `Vertex3D` (`src/math/geometry/Vertex3D.h`):
```cpp
namespace zzz::math
{
    struct alignas(16) Vertex3D
    {
        Vec3f        position{ 0.0f, 0.0f, 0.0f };       ///< 12б: Позиция (X, Y, Z)
        Vec3f        normal{ 0.0f, 0.0f, 1.0f };         ///< 12б: Нормаль (NX, NY, NZ)
        Vec2f        texCoord{ 0.0f, 0.0f };             ///< 8б:  UV-координаты (U, V)
        Color4<zF32> color{ Palette4::White };           ///< 16б: Цвет (RGBA [0..1])
        Vec4f        tangent{ 1.0f, 0.0f, 0.0f, 1.0f };  ///< 16б: Касательный вектор (XYZ + W)

        constexpr Vertex3D() noexcept = default;
        constexpr Vertex3D(const Vec3f& pos, const Vec3f& norm, const Vec2f& uv, 
                           const Color4<zF32>& col = Palette4::White, const Vec4f& tan = {1.f, 0.f, 0.f, 1.f}) noexcept
            : position(pos), normal(norm), texCoord(uv), color(col), tangent(tan) {}

        [[nodiscard]] constexpr bool operator==(const Vertex3D&) const noexcept = default;
    };

    static_assert(sizeof(Vertex3D) == 64, "Vertex3D must be exactly 64 bytes (1 CPU cache line)");
    static_assert(alignof(Vertex3D) == 16, "Vertex3D must be 16-byte aligned");
    static_assert(std::is_standard_layout_v<Vertex3D>, "Vertex3D must be standard layout");
}
```

---

### 2.6. Легковесный стридовый диапазон `AttributeRange<T>` (`src/math/geometry/AttributeRange.h`)

Служит фундаментальным примитивом для безопасного обхода произвольных массивов данных со страйдом на CPU (Zero-Copy):
```cpp
namespace zzz::math
{
    template<typename T>
    class AttributeRange
    {
        const std::byte* m_Start = nullptr;
        std::size_t      m_Stride = 0;
        std::size_t      m_Count = 0;

    public:
        constexpr AttributeRange() noexcept = default;
        constexpr AttributeRange(const std::byte* start, std::size_t stride, std::size_t count) noexcept
            : m_Start(start), m_Stride(stride), m_Count(count) {}

        [[nodiscard]] const T& operator[](std::size_t index) const noexcept
        {
            return *reinterpret_cast<const T*>(m_Start + (index * m_Stride));
        }

        [[nodiscard]] std::size_t size() const noexcept { return m_Count; }
        [[nodiscard]] bool empty() const noexcept { return m_Count == 0; }

        struct Iterator
        {
            const std::byte* ptr;
            std::size_t stride;

            const T& operator*() const noexcept { return *reinterpret_cast<const T*>(ptr); }
            Iterator& operator++() noexcept { ptr += stride; return *this; }
            bool operator!=(const Iterator& other) const noexcept { return ptr != other.ptr; }
        };

        Iterator begin() const noexcept { return { m_Start, m_Stride }; }
        Iterator end() const noexcept   { return { m_Start + (m_Count * m_Stride), m_Stride }; }
    };
}
```

---

### 2.7. Выравнивание константных буферов CBV 256 байт (`src/core/utils/Alignment.h`)
Аппаратное требование DirectX 12 (`D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT = 256`), Vulkan и Metal:
- Константа: `constexpr zU32 c_ConstantBufferAlignment = 256;` (в `src/core/constants/GAPIConstants.h`).
- Функция:
  ```cpp
  template<std::integral T>
  [[nodiscard]] constexpr T AlignUp(T value, T alignment) noexcept
  {
      assert((alignment > 0) && ((alignment & (alignment - 1)) == 0) && "AlignUp: alignment must be a power of two");
      return (value + alignment - static_cast<T>(1)) & ~(alignment - static_cast<T>(1));
  }
  ```

---

### 2.8. Поддержка `EnumToString` и бинарной сериализации `Serializer`

1. **`EnumToString` (`src/core/enums/eEnumToString.h`):**
   - `static constexpr std::string_view ToString(eResourceType type);`
   - `static constexpr std::string_view ToString(ePixelFormat format);`
   - `static constexpr std::string_view ToString(eIndexFormat format);`
   - `static constexpr std::string_view ToString(eVertexSemantic semantic);`

2. **Сериализация (`src/core/serialize/Serializer.h`):**
   - Перегрузки `Serialize` / `Deserialize` для `eResourceType`, `ePixelFormat`, `eIndexFormat`, `eVertexSemantic` (через концепт `SerializablePrimitive`).
   - Перегрузки `Serialize` / `Deserialize` для структуры `Vertex3D`.

---

## 3. Файловая структура этапа

```
src/core/
├── constants/
│   └── GAPIConstants.h             # Определение c_ConstantBufferAlignment = 256
├── utils/
│   └── Alignment.h                 # [NEW] constexpr AlignUp(val, align) с проверкой power-of-two
├── enums/
│   ├── eResourceType.h             # [NEW] eResourceType (Unknown, Texture2D, Mesh, Material, Shader...)
│   ├── ePixelFormat.h              # [NEW] ePixelFormat и PixelFormatUtils (BytesPerPixel, IsDepth, IsStencil, IsSRGB...)
│   ├── eIndexFormat.h              # [NEW] eIndexFormat (UInt16, UInt32, GetIndexFormatBytes)
│   ├── eVertexSemantic.h           # [NEW] eVertexSemantic (Position, Normal, TexCoord, Color, Tangent...)
│   ├── eEnumToString.h             # Поддержка ToString для всех новых перечислений
│   └── platforms/
│       └── ConverterGAPITypes.h    # [NEW] ToNative/ToEngine под макросами Z_D3D12/Z_VULKAN/Z_METAL в namespace zzz::core
├── serialize/
│   └── Serializer.h                # Перегрузки Serialize/Deserialize для перечислений и Vertex3D
src/math/
└── geometry/
    ├── Vertex3D.h                  # [NEW] Каноническая структура Vertex3D (64 байта)
    └── AttributeRange.h            # [NEW] Шаблонный стридовый диапазон доступа AttributeRange<T>
src/qa/tests/
├── TestsConfig.h                   # Включение #define Z_TEST_CORE_ENUMS_STRUCTS
└── core/
    └── EnumsAndStructuresTests.cpp  # [NEW] Комплексные юнит-тесты (AlignUp, Vertex3D layout, AttributeRange, EnumToString, Serialization, Converters)
```

---

## 4. Чек-лист реализации и Definition of Done (DoD)

- [x] Создать `src/core/utils/Alignment.h` с функцией `AlignUp` и проверкой power-of-two
- [x] Добавить `c_ConstantBufferAlignment = 256` в `src/core/constants/GAPIConstants.h`
- [x] Создать `src/core/enums/eResourceType.h` с комментариями о различии с `ePackage`
- [x] Создать `src/core/enums/ePixelFormat.h` с `PixelFormatUtils` (включая `D32_FLOAT_S8_UINT` и `IsStencilFormat`)
- [x] Создать `src/core/enums/eIndexFormat.h`
- [x] Создать `src/core/enums/platforms/ConverterGAPITypes.h` (`ToNative` / `ToEnginePixelFormat` / `ToEngineIndexFormat` / `ToEngine<T>`)
- [x] Создать `src/core/enums/eVertexSemantic.h`
- [x] Создать `src/math/geometry/Vertex3D.h` (канонический `Vertex3D` на 64 байта с `Color4<zF32>` и `static_assert`)
- [x] Создать `src/math/geometry/AttributeRange.h` (strided-диапазон и итератор для безопасного обхода)
- [x] Подключить новые заголовки в `src/math/Math.h` и `src/core/Core.h`
- [x] Обновить `src/core/enums/eEnumToString.h` (методы `ToString` для `eResourceType`, `ePixelFormat`, `eIndexFormat`, `eVertexSemantic`)
- [x] Добавить перегрузки сериализации в `src/core/serialize/Serializer.h`
- [x] Добавить тумблер `Z_TEST_CORE_ENUMS_STRUCTS` в `src/qa/tests/TestsConfig.h`
- [x] Создать `src/qa/tests/core/EnumsAndStructuresTests.cpp`
- [x] Собрать и прогнать все тесты (100% PASS: 64/64 тестов)
