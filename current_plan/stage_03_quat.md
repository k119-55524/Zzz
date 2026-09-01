# Этап 03: Кватернионы `Quat` (3D-вращения, Slerp, матрицы, углы Эйлера и сериализация)

## 1. Контекст и цели этапа
- **Цель:** Реализовать фундаментальный класс кватернионов `Quat` для описания, композиции и гладкой интерполяции 3D-вращений без эффекта шарнирного замка (Gimbal Lock), с полной поддержкой принятой в движке **левосторонней системы координат (Left-Handed: Y-up, Z-forward, X-right)** и порядка углов Эйлера **$Z \to X \to Y$ (Roll $\to$ Pitch $\to$ Yaw)**.
- **Статус:** `✅ Выполнено`
- **Зависимости:** `src/math/utils/Types.h`, `src/math/MathIncludes.h`, `src/math/utils/MathConstants.h`, `src/math/utils/MathUtils.h`, `src/math/vector/Vec3.h`, `src/math/matrix/Mat3.h`, `src/math/matrix/Mat4.h`, `src/core/serialize/Serializer.h`.

---

## 2. Архитектурные требования и концепты C++23

### 2.1. Ограничение типов концептами (Floating-Point Only)
Кватернионы описывают непрерывные 3D-вращения и используют тригонометрию ($\sin, \cos, \text{acos}, \text{asin}, \text{atan2}$), вычисление норм ($\sqrt{x^2+y^2+z^2+w^2}$) и сферическую интерполяцию ($\text{Slerp}$). Поэтому `Quat` строго ограничен концептом вещественных чисел с плавающей запятой:
- **Концепт:** `template<std::floating_point T = zF32>` (или `template<FloatingPoint T = zF32>`);
- **Поддерживаемые типы движка:**
  - `Quatf = Quat<zF32>` (`float`, 32-bit);
  - `Quatd = Quat<zF64>` (`double`, 64-bit);
- Попытка инстанцирования целочисленными типами (`int`, `uint32_t` и т.д.) отсекается компилятором на этапе проверки концепта.

### 2.2. Расположение в памяти и Standard Layout POD
- Кватернион `Quat<T>` хранит 4 открытых компонента: `T x = T(0)`, `T y = T(0)`, `T z = T(0)`, `T w = T(1)`.
- Выравнивание структуры под векторный слот GPU/SIMD: `alignas(sizeof(T) * 4)` (16 байт для `zF32`, 32 байта для `zF64`).
- Размер структуры: ровно 16 байт для `zF32` (`sizeof(Quatf) == 16`), стандартный макет памяти (Standard Layout POD).
- Инварианты размера, выравнивания и POD-структуры проверяются через `static_assert` непосредственно в заголовке `Quat.h`.
- Прямой доступ к сырому массиву компонентов через `data()`.

### 2.3. Стандарт системы координат и соглашение углов Эйлера ($Z \to X \to Y$)
- **Левосторонняя система координат (Left-Handed: Y-up, Z-forward, X-right)**:
  - Положительное вращение вокруг оси определяется по правилу левой руки (по часовой стрелке при взгляде вдоль направления положительной оси).
- **Порядок композиции углов Эйлера в `FromEuler` / `ToEuler`**:
  - $Z \to X \to Y$ (Roll $\to$ Pitch $\to$ Yaw), строго согласовано с матрицами `Mat4::TRS` (Этап 02).
  - Углы задаются в радианах: `pitch` (вокруг оси $X$), `yaw` (вокруг оси $Y$), `roll` (вокруг оси $Z$).

### 2.4. Вращение векторов и соглашение векторов-строк ($v \cdot M$)
- Поворот 3D-вектора $v$ кватернионом $q$:
  $$v' = q \cdot (v_x, v_y, v_z, 0) \cdot q^*$$
- **Порядок композиции вращений (Hamilton Product vs Row-Vector Matrices):**
  - При умножении матриц строк $v \cdot (M_1 \cdot M_2)$ порядок применения слева направо ($M_1$ затем $M_2$).
  - При стандартном Гамильтоновом умножении кватернионов $(q_1 \cdot q_2) \cdot v \cdot (q_1 \cdot q_2)^* = q_1 \cdot (q_2 \cdot v \cdot q_2^*) \cdot q_1^*$ — правый кватернион ($q_2$) применяется к вектору **первым**, а левый ($q_1$) — **вторым** (согласовано со стандартом движков: $q_{world} = q_{parent} \cdot q_{local}$).
- Операторы:
  - `Vec3<T> RotateVector(const Vec3<T>& v) const noexcept;`
  - `friend constexpr Vec3<T> operator*(const Vec3<T>& v, const Quat<T>& q) noexcept;` (согласно Row-Vector соглашению $v \cdot q$).

### 2.5. Структурные уточнения модуля `src/math/`
- **Геометрия:** `Point2D`, `Size2D`, `Rect2D` расположены в каталоге `src/math/geometry/`.
- **Утилиты и константы (`src/math/utils/`):**
  - `src/math/utils/Types.h`: Базовые типы `zU8`..`zU64`, `zI8`..`zI64`, `zF32`, `zF64`.
  - `src/math/utils/MathConstants.h`: Чистые константы `namespace zzz::math::Constants` (`Epsilon<T>`, `PI<T>`, `TwoPI<T>`, `HalfPI<T>`, `Deg2Rad<T>`, `Rad2Deg<T>`).
  - `src/math/utils/MathUtils.h`: Скалярные функции `namespace zzz::math::Math` (`Abs`, `Epsilon()`, `PI()`, `Deg2Rad()`, `Rad2Deg()`, `ToRadians()`, `ToDegrees()`, `Clamp()`, `Lerp()`).
  - `src/math/utils/color/`: Подсистема цвета (`Color.h`, `Color3.h`, `Color4.h`, `Color3Palette.h`, `Color4Palette.h`).
  - В корнях `src/math/Types.h` и `src/math/Color.h` оставлены forwarder-заголовки для обратной совместимости.

---

## 3. Детальная спецификация интерфейса `Quat<T>`

```cpp
template<std::floating_point T = zF32>
struct alignas(sizeof(T) * 4) Quat
{
    T x = T(0);
    T y = T(0);
    T z = T(0);
    T w = T(1);
    
    // ...
};
```

### 3.1. Конструкторы и фабричные методы
- `constexpr Quat() noexcept = default;` — инициализирует единичный кватернион (Identity: $x=0, y=0, z=0, w=1$).
- `constexpr Quat(T inX, T inY, T inZ, T inW) noexcept;`
- `constexpr Quat(const Vec3<T>& v, T inW) noexcept;`
- `template<std::floating_point U> constexpr explicit Quat(const Quat<U>& other) noexcept;`
- **Статические фабрики:**
  - `[[nodiscard]] static constexpr Quat Identity() noexcept;` — $(0, 0, 0, 1)$
  - `[[nodiscard]] static constexpr Quat Zero() noexcept;` — $(0, 0, 0, 0)$
  - `[[nodiscard]] static Quat FromAxisAngle(const Vec3<T>& axis, T angleRadians) noexcept;`
  - `[[nodiscard]] static Quat FromEuler(T pitchX, T yawY, T rollZ) noexcept;`
  - `[[nodiscard]] static Quat FromEuler(const Vec3<T>& eulerRadians) noexcept;`
  - `[[nodiscard]] static Quat FromRotationMatrix(const Mat3<T>& m) noexcept;`
  - `[[nodiscard]] static Quat FromRotationMatrix(const Mat4<T>& m) noexcept;`
  - `[[nodiscard]] static Quat FromToRotation(const Vec3<T>& from, const Vec3<T>& to) noexcept;`
  - `[[nodiscard]] static Quat LookRotation(const Vec3<T>& forward, const Vec3<T>& up = Vec3<T>::Up()) noexcept;`

### 3.2. Доступ к компонентам и свойствам
- Открытые поля `T x`, `T y`, `T z`, `T w`.
- `[[nodiscard]] constexpr T* data() noexcept;` / `[[nodiscard]] constexpr const T* data() const noexcept;`
- `[[nodiscard]] constexpr T& operator[](usize index) noexcept;` / `[[nodiscard]] constexpr const T& operator[](usize index) const noexcept;`
- `[[nodiscard]] constexpr Vec3<T> GetVectorPart() const noexcept;` (возвращает `Vec3<T>{x, y, z}`)
- `[[nodiscard]] constexpr T GetScalarPart() const noexcept;` (возвращает `w`)

### 3.3. Базовая алгебра и операции
- Сложение и вычитание: `operator+`, `operator-`, `operator+=`, `operator-=`.
- Умножение на скаляр: `operator*`, `operator*=`, `operator/`, `operator/=`.
- Композиция вращений (умножение кватернионов):
  - `constexpr Quat operator*(const Quat& rhs) const noexcept;`
  - `constexpr Quat& operator*=(const Quat& rhs) noexcept;`
- Унарный минус: `constexpr Quat operator-() const noexcept;`
- Сравнение: `constexpr operator==`, `constexpr operator!=` с учетом машинной погрешности `Math::Epsilon<T>()` и двойного покрытия ($q \equiv -q$).

### 3.4. Геометрические методы
- Скалярное произведение: `[[nodiscard]] static constexpr T Dot(const Quat& a, const Quat& b) noexcept;`
- Длина / норма:
  - `[[nodiscard]] constexpr T LengthSquared() const noexcept;`
  - `[[nodiscard]] T Length() const noexcept;`
- Нормализация:
  - `[[nodiscard]] Quat Normalized() const noexcept;`
  - `void Normalize() noexcept;`
  - `[[nodiscard]] constexpr bool IsNormalized(T tolerance = Math::Epsilon<T>()) const noexcept;`
- Сопряжение и обратный кватернион:
  - `[[nodiscard]] constexpr Quat Conjugate() const noexcept;` ($-x, -y, -z, w$)
  - `[[nodiscard]] Quat Inverse() const noexcept;` (`Conjugate() / LengthSquared()`)

### 3.5. Интерполяции (Lerp, Slerp)
- `[[nodiscard]] static Quat Lerp(const Quat& a, const Quat& b, T t) noexcept;` (линейная интерполяция с нормализацией).
- `[[nodiscard]] static Quat Slerp(const Quat& a, const Quat& b, T t) noexcept;` (сферическая интерполяция с выбором кратчайшего пути при $\text{Dot}(a, b) < 0$).

### 3.6. Конвертация в матрицы и углы
- `[[nodiscard]] Mat3<T> ToMat3() const noexcept;` — матрица вращения $3 \times 3$.
- `[[nodiscard]] Mat4<T> ToMat4() const noexcept;` — матрица вращения $4 \times 4$.
- `[[nodiscard]] Vec3<T> ToEuler() const noexcept;` — извлечение углов в радианах в порядке $Z \to X \to Y$ (pitch, yaw, roll).
- `void ToAxisAngle(Vec3<T>& outAxis, T& outAngleRadians) const noexcept;`

### 3.7. Сериализация и строковое представление
- `[[nodiscard]] std::string ToString() const;`
- Специализация `std::formatter<Quat<T>>`.
- Перегрузки `Serializer::Serialize` и `Serializer::Deserialize` в `src/core/serialize/Serializer.h`:
  - Последовательная бинарная запись и чтение `q.x`, `q.y`, `q.z`, `q.w`.

---

## 4. Структура файлов в проекте

```
src/math/
├── Math.h                      # Подключение math/quat/Quat.h, utils/MathConstants.h, utils/MathUtils.h, utils/color/Color.h
├── MathIncludes.h              # Концепты Arithmetic, SafelyConvertibleTo, подключение utils/MathConstants.h, utils/MathUtils.h
├── Types.h                     # Forwarder -> utils/Types.h
├── Color.h                     # Forwarder -> utils/color/Color.h
├── utils/                      # Базовые скалярные утилиты, типы, константы и цвет
│   ├── Types.h
│   ├── MathConstants.h
│   ├── MathUtils.h
│   ├── Color.h
│   └── color/
│       ├── Color.h
│       ├── Color3.h
│       ├── Color4.h
│       ├── Color3Palette.h
│       └── Color4Palette.h
├── geometry/                   # 2D-геометрия (Point2D, Size2D, Rect2D)
├── vector/                     # Векторы (Vec2, Vec3, Vec4)
├── matrix/                     # Матрицы (Mat3, Mat4)
└── quat/
    └── Quat.h                  # Шаблонная структура Quat<T> (вращения, Slerp, ToMat4, Euler)
src/core/serialize/
└── Serializer.h                # Перегрузки Serialize/Deserialize для Quat<T>
src/qa/tests/
├── TestsConfig.h               # Раскомментирование #define Z_TEST_MATH_QUAT
└── math/
    └── QuatTests.cpp           # Комплексные юнит-тесты Quat (Identity, Slerp, Euler, ToMat4, RotateVector, Serialization, Constexpr)
```

---

## 5. Чек-лист реализации и Definition of Done (DoD)

- [x] Добавить концепт `FloatingPoint` в `src/math/utils/MathConstants.h`
- [x] Создать `src/math/quat/Quat.h` (структура Standard Layout POD, 16 байт для `zF32`, выравнивание `alignas(sizeof(T) * 4)`, концепт `std::floating_point<T>`)
- [x] Реализовать конструкторы, Identity, Zero, покомпонентные операции, умножение кватернионов
- [x] Реализовать `Conjugate()`, `Inverse()`, `Length()`, `Normalized()`, `Dot()`
- [x] Реализовать трансформацию векторов `RotateVector(v)` и оператор `v * q`
- [x] Реализовать `FromAxisAngle`, `ToAxisAngle`
- [x] Реализовать `FromEuler` и `ToEuler` с порядком $Z \to X \to Y$
- [x] Реализовать `FromRotationMatrix` (метод Шеппарда) и `ToMat3()` / `ToMat4()`
- [x] Реализовать сферическую интерполяцию `Slerp` с кратчайшим путем и `Lerp`
- [x] Реализовать `LookRotation` и `FromToRotation`
- [x] Подключить `Quat.h` в `src/math/Math.h`
- [x] Добавить перегрузки `Serialize` / `Deserialize` для `Quat<T>` в `src/core/serialize/Serializer.h`
- [x] Создать `src/qa/tests/math/QuatTests.cpp` под тумблером `Z_TEST_MATH_QUAT` в `TestsConfig.h`
- [x] Реорганизовать вспомогательную структуру в `src/math/utils/` (`Types.h`, `MathConstants.h`, `MathUtils.h`, `color/`)
- [x] Собрать тестовый таргет `EngineTests` и успешно пройти 100% тестов (57/57)
- [x] Запросить утверждение у пользователя
- [x] Зафиксировать Git-коммит: `feat(math): completed stage 03 - Quat class, rotations, slerp, euler and serialization`
- [x] Обновить статус Пункта 3 в `general_plan.md` на `✅ Выполнено`
