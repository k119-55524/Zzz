# Этап 02: Матрицы `Mat4` и `Mat3` (базовые операции, аффинные трансформации, базис и проекции)

## 1. Контекст и цели этапа
- **Цель:** Реализовать фундаментальные классы матриц $4 \times 4$ (`Mat4`) и $3 \times 3$ (`Mat3`) для линейных и аффинных преобразований в 3D/2D-пространстве, формирования матриц вида (View) и проекции (Projection) в левосторонней системе координат (LH) с диапазоном глубины NDC Z $[0, 1]$, извлечения базиса ориентации и вычисления матрицы нормалей (Normal Matrix).
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

### 2.2. Расположение в памяти, выравнивание и хранение элементов (Standard Layout POD)
- **`Mat4`**: выравнивается по строке GPU/SIMD-слота: `alignas(sizeof(T) * 4)` (16 байт для `zF32`, 32 байта для `zF64`), 64 байта для `zF32`.
- **`Mat3`**: плоская компактная структура из 9 элементов (`m[3][3]`, `elements[9]`, `_11.._33`), 36 байт для `zF32`.
- Прямой доступ к сырому указателю через `data()`.

### 2.3. Строгая типизация движка (Strict Engine Types)
- Шаблоны `template<Arithmetic T = zF32> struct Mat4` и `template<Arithmetic T = zF32> struct Mat3`.
- **Псевдонимы (Typedefs):**
  - `using Mat4f = Mat4<zF32>;`, `using Mat4d = Mat4<zF64>;`, `using Mat4i = Mat4<zI32>;`
  - `using Mat3f = Mat3<zF32>;`, `using Mat3d = Mat3<zF64>;`, `using Mat3i = Mat3<zI32>;`

---

## 3. Детальная спецификация интерфейса

### 3.1. Соглашение об умножении векторов (Row-Vector Convention: $v \cdot M$)
В движке принят единый стандарт векторов-строк (Row-Vector / Post-multiplication, аналогично DirectXMath):
- Вектор умножается **слева**: `v * M` (`Vec4 operator*(const Vec4& v, const Mat4& m)`);
- В строках `_41.._43` матрицы хранится вектор трансляции;
- Матрицы вида (`LookAtLH`) и проекции (`PerspectiveFovLH`, `OrthographicLH`) построены строго под применение `v * M`.

### 3.2. Порядок углов Эйлера в `TRS` ($Z \to X \to Y$)
Комбинированная матрица вращения в методе `Mat4::TRS` вычисляется как:
$$R = R_z \cdot R_x \cdot R_y$$
При строчном умножении вектора $v \cdot (S \cdot R_z \cdot R_x \cdot R_y \cdot T)$ это задаёт порядок применения: **$Z \to X \to Y$ (Roll $\to$ Pitch $\to$ Yaw)**.  
Этот же порядок строго фиксируется для кватернионов `Quat::FromEuler` / `ToEuler` на Этапе 03.

### 3.3. Спецификация `Mat3`
- **Структура**: `template<Arithmetic T = zF32> struct Mat3` (9 элементов, 36 байт для `zF32`).
- **Конструкторы**: Identity (по умолчанию), Zero, Diagonal, 9 элементов, из 3 векторов `Vec3<T>`, конвертирующий `Mat3<U>`.
- **Доступ**: `data()`, `operator()(r, c)`, `operator[](i)`, `GetRow`/`SetRow`, `GetColumn`/`SetColumn`.
- **Операции**: `operator*`, `operator*=`, `operator*=(scalar)`, `operator*(scalar)`, `Determinant()`, `Inverse()`, `Transpose()`, `operator==`, `operator!=`.
- **Трансформации**: `TransformVector(const Vec3<T>& v)` и оператор `v * M`.
- **Форматирование**: `ToString()` и специализация `std::formatter<Mat3<T>>`.

### 3.4. Спецификация `Mat4`
- **Структура**: `template<Arithmetic T = zF32> struct alignas(sizeof(T) * 4) Mat4` (16 элементов, 64 байта для `zF32`).
- **Конструкторы**: Identity (по умолчанию), Zero, Diagonal, 16 элементов, из 4 векторов `Vec4<T>`, из `Mat3<T>` + translation, конвертирующий `Mat4<U>`.
- **Доступ**: `data()`, `operator()(r, c)`, `operator[](i)`, `GetRow`/`SetRow`, `GetColumn`/`SetColumn`.
- **Базис и нормали**:
  - `[[nodiscard]] constexpr Mat3<T> ToMat3() const noexcept` (извлечение верхнего левого блока $3 \times 3$).
  - `[[nodiscard]] Mat3<T> GetNormalMatrix() const noexcept` (`Transpose(Inverse(ToMat3()))`).
- **Аффинные трансформации**: `Translation`, `Scaling`, `RotationX/Y/Z`, `RotationAxis`, `TRS`.
- **Проекции LH $[0, 1]$**: `LookAtLH`, `PerspectiveFovLH`, `OrthographicLH`, `OrthographicOffCenterLH`.
- **Трансформации векторов**:
  - `TransformPoint(const Vec3<T>& pt)` ($W=1$, перспективное деление $X/W, Y/W, Z/W$);
  - `TransformVector(const Vec3<T>& vec)` ($W=0$, без смещения);
  - `TransformVector4(const Vec4<T>& v)` и `operator*(const Vec4<T>& v, const Mat4<T>& m)` ($v \cdot M$).
- **Базовая алгебра**: `operator*`, `operator*=`, `operator*=(scalar)`, `operator*(scalar)`, `Determinant()`, `Inverse()`, `Transpose()`, `operator==`, `operator!=`.
- **Форматирование**: `ToString()` и специализация `std::formatter<Mat4<T>>`.

---

## 4. Структура файлов в проекте

```
src/math/
├── Math.h                      # Подключение math/matrix/Mat3.h и math/matrix/Mat4.h
└── matrix/
    ├── Mat3.h                  # Шаблонная структура Mat3<T> (базис, ориентация, нормали)
    └── Mat4.h                  # Шаблонная структура Mat4<T> (аффинные трансформации, проекции)
src/core/serialize/
└── Serializer.h                # Перегрузки Serialize/Deserialize для Mat3<T> и Mat4<T>
src/qa/tests/
├── TestsConfig.h               # Раскомментирование #define Z_TEST_MATH_MAT3 и Z_TEST_MATH_MAT4
└── math/
    ├── Mat3Tests.cpp           # Юнит-тесты Mat3 (Identity, Transpose, Inverse, NormalMatrix, Serialization)
    └── Mat4Tests.cpp           # Юнит-тесты Mat4 (Identity, TRS, Inverse, Projections LH [0, 1], Serialization)
```

---

## 5. Чек-лист реализации и Definition of Done (DoD)

- [x] Создать `src/math/matrix/Mat4.h` (структура Standard Layout POD, 64 байта, выравнивание `alignas(sizeof(T) * 4)`)
- [x] Создать `src/math/matrix/Mat3.h` (структура Standard Layout POD, 36 байт, 9 элементов)
- [x] Добавить в `Mat4.h` методы `ToMat3()` и `GetNormalMatrix()`
- [x] Реализовать базовую алгебру `Mat4` (Identity, Zero, умножение $v \cdot M$, определитель, Inverse, Transpose, GetRow/SetRow/GetColumn/SetColumn, operator==/!=)
- [x] Реализовать аффинные трансформации (Translation, Scaling, RotationX/Y/Z, RotationAxis, TRS в порядке $Z \to X \to Y$)
- [x] Реализовать функции проекции и вида для LH системы координат с NDC Z $[0, 1]$ (`LookAtLH`, `PerspectiveFovLH`, `OrthographicLH`, `OrthographicOffCenterLH`)
- [x] Реализовать трансформацию векторов `TransformPoint` и `TransformVector`
- [x] Подключить заголовки `Mat3.h` и `Mat4.h` в `src/math/Math.h`
- [x] Добавить перегрузки `Serialize` / `Deserialize` для `Mat3` и `Mat4` в `src/core/serialize/Serializer.h`
- [x] Создать раздельные файлы тестов `src/qa/tests/math/Mat3Tests.cpp` и `src/qa/tests/math/Mat4Tests.cpp` под отдельными тумблерами `Z_TEST_MATH_MAT3` и `Z_TEST_MATH_MAT4`
- [x] Собрать тестовый таргет `EngineTests` через CMake / MSVC x64
- [x] Успешно прогнать все 46 тестов (100% успех)
- [ ] Запросить утверждение у пользователя
- [ ] Сделать Git commit: `feat(math): completed stage 02 - Mat4 and Mat3 matrices, normal matrix, transforms and projections`
- [ ] Перевести статус Пункта 2 в `✅ Выполнено` в `general_plan.md`
- [ ] Подготовить спецификацию следующего шага `stage_03_quat.md`