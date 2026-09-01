# Этап 02: Матрица `Mat4` (базовые операции, аффинные трансформации и проекции)

## 1. Контекст и цели этапа
- **Цель:** Реализовать фундаментальный класс матрицы $4 \times 4$ (`Mat4`) для линейных и аффинных преобразований в 3D-пространстве, формирования матриц вида (View) и проекции (Projection) в левосторонней системе координат (LH) с диапазоном глубины NDC Z $[0, 1]$.
- **Статус:** `🔄 В работе`
- **Зависимости:** `src/math/Types.h`, `src/math/MathIncludes.h`, `src/math/vector/Vec3.h`, `src/math/vector/Vec4.h`, `src/core/serialize/Serializer.h`.

---

## 2. Архитектурные требования и стандарты движка

### 2.1. Система координат и проекции
- **Левосторонняя система координат (Left-Handed: Y-up, Z-forward, X-right)**:
  - Направление взгляда камеры по умолчанию: вдоль оси $+Z$.
- **Диапазон глубины NDC (Normalized Device Coordinates):**
  - $Z \in [0, 1]$ (современный стандарт DirectX 12, Vulkan, Metal).
  - Near plane отображается в $Z = 0$, Far plane — в $Z = 1$.
- **Функции проекции и вида:**
  - `LookAtLH(const Vec3<T>& eye, const Vec3<T>& target, const Vec3<T>& up)`
  - `PerspectiveFovLH(T fovYRadians, T aspect, T nearZ, T farZ)`
  - `OrthographicLH(T width, T height, T nearZ, T farZ)`
  - `OrthographicOffCenterLH(T left, T right, T bottom, T top, T nearZ, T farZ)`

### 2.2. Расположение в памяти и хранение элементов (Standard Layout POD)
- Матрица хранится как плоский массив из 16 элементов типа `T` (Row-Major в памяти: `m[0]`..`m[15]` или `m[4][4]`):
  ```cpp
  union
  {
      T m[4][4];
      T elements[16];
      struct
      {
          T _11, _12, _13, _14;
          T _21, _22, _23, _24;
          T _31, _32, _33, _34;
          T _41, _42, _43, _44;
      };
  };
  ```
- `static_assert(std::is_standard_layout_v<Mat4<zF32>>);`
- `static_assert(sizeof(Mat4<zF32>) == 64);`
- Прямой доступ к сырому указателю через `data()` для передачи в константные буферы GAPI (`CBV`, `PushConstants`).

### 2.3. Строгая типизация движка (Strict Engine Types)
- Шаблон `template<Arithmetic T = zF32> struct Mat4`.
- **Запрещено** использовать сырые типы `float`, `double`, `int` в заголовочных файлах.
- **Псевдонимы (Typedefs):**
  - `using Mat4f = Mat4<zF32>;`
  - `using Mat4d = Mat4<zF64>;`
  - `using Mat4i = Mat4<zI32>;`

---

## 3. Детальная спецификация интерфейса `Mat4`

### 3.1. Конструкторы и статические фабрики
- `constexpr Mat4() noexcept = default;` (единичная матрица по умолчанию — Identity);
- `explicit constexpr Mat4(T diagonal) noexcept;` (диагональная матрица);
- Поэлементный конструктор `constexpr Mat4(T m00, T m01, ... T m33) noexcept;`;
- Из массива `explicit constexpr Mat4(const T elements[16]) noexcept;`;
- Из векторов строк или столбцов;
- Фабрики:
  - `static constexpr Mat4 Identity() noexcept;`
  - `static constexpr Mat4 Zero() noexcept;`
  - `static constexpr Mat4 Translation(const Vec3<T>& translation) noexcept;`
  - `static constexpr Mat4 Translation(T x, T y, T z) noexcept;`
  - `static constexpr Mat4 Scaling(const Vec3<T>& scale) noexcept;`
  - `static constexpr Mat4 Scaling(T x, T y, T z) noexcept;`
  - `static constexpr Mat4 Scaling(T uniformScale) noexcept;`
  - `static Mat4 RotationX(T radians) noexcept;`
  - `static Mat4 RotationY(T radians) noexcept;`
  - `static Mat4 RotationZ(T radians) noexcept;`
  - `static Mat4 RotationAxis(const Vec3<T>& axis, T radians) noexcept;`
  - `static Mat4 TRS(const Vec3<T>& translation, const Vec3<T>& rotationEuler, const Vec3<T>& scale) noexcept;`

### 3.2. Матричные операции
- Умножение матриц `operator*(const Mat4& other) const` и `operator*=(const Mat4& other)`;
- Умножение на скаляр `operator*(T scalar) const` и `operator*=(T scalar)`;
- Транспонирование: `[[nodiscard]] Mat4 Transpose() const noexcept;`
- Определитель: `[[nodiscard]] T Determinant() const noexcept;`
- Обратная матрица: `[[nodiscard]] Mat4 Inverse(bool* outInvertible = nullptr) const noexcept;` (с безопасной обработкой вырожденных матриц).

### 3.3. Трансформация векторов и точек
- Умножение на 4D-вектор: `[[nodiscard]] Vec4<T> operator*(const Vec4<T>& v) const noexcept;`
- Трансформация точки (Point): `[[nodiscard]] Vec3<T> TransformPoint(const Vec3<T>& point) const noexcept;` ($W = 1$, с делением на результирующий $W$ при перспективном делении);
- Трансформация направления (Vector/Normal): `[[nodiscard]] Vec3<T> TransformVector(const Vec3<T>& vec) const noexcept;` ($W = 0$, без учета сдвига);
- Доступ по индексам: `operator()(size_t row, size_t col)` и `operator[](size_t index)`.

### 3.4. Сериализация и вывод
- Метод `ToString() const` и специализация `std::formatter<zzz::math::Mat4<T>>`;
- Перегрузки `Serialize` / `Deserialize` в `zzz::core::Serializer`.

---

## 4. Структура файлов в проекте

```
src/math/
├── Math.h                      # Подключение math/matrix/Mat4.h
└── matrix/
    └── Mat4.h                  # Шаблонная структура Mat4<T>
src/core/serialize/
└── Serializer.h                # Перегрузки Serialize/Deserialize для Mat4<T>
src/qa/tests/
├── TestsConfig.h               # Раскомментирование #define Z_TEST_MATH_MATRICES
└── math/
    └── MatrixTests.cpp         # Юнит-тесты Mat4 (Identity, TRS, Inverse, Transpose, LookAtLH, PerspectiveFovLH, Serialization)
```

---

## 5. Чек-лист реализации и Definition of Done (DoD)

- [ ] Создать `src/math/matrix/Mat4.h` (структура Standard Layout POD, 64 байта)
- [ ] Реализовать базовую алгебру (Identity, Zero, умножение, определитель, Inverse, Transpose)
- [ ] Реализовать аффинные трансформации (Translation, Scaling, RotationX/Y/Z, RotationAxis, TRS)
- [ ] Реализовать функции проекции и вида для LH системы координат с NDC Z $[0, 1]$ (`LookAtLH`, `PerspectiveFovLH`, `OrthographicLH`, `OrthographicOffCenterLH`)
- [ ] Реализовать трансформацию векторов `TransformPoint` и `TransformVector`
- [ ] Подключить заголовок `Mat4.h` в `src/math/Math.h`
- [ ] Добавить перегрузки `Serialize` / `Deserialize` для `Mat4` в `src/core/serialize/Serializer.h`
- [ ] Создать `src/qa/tests/math/MatrixTests.cpp` и раскомментировать `Z_TEST_MATH_MATRICES` в `src/qa/tests/TestsConfig.h`
- [ ] Собрать тестовый таргет `EngineTests` через CMake / MSVC x64
- [ ] Успешно прогнать все тесты (проверка ассоциативности умножения, `M * M^-1 == I`, `Transpose(Transpose(M)) == M`, проекция точки внутри `PerspectiveFovLH` в NDC $[0, 1]$, бинарный round-trip в `Serializer`)
- [ ] Запросить утверждение у пользователя
- [ ] Сделать Git commit: `feat(math): completed stage 02 - Mat4 matrix, transforms and projections`
- [ ] Перевести статус Пункта 2 в `✅ Выполнено` в `general_plan.md`
- [ ] Подготовить спецификацию следующего шага `stage_03_quat.md`