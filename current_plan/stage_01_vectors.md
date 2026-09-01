# Этап 01: Векторная математика (`Vec2`, `Vec3`, `Vec4`) и унификация `Point2D`, `Size2D`, `Rect2D`

## 1. Контекст и цели этапа
- **Цель:** Создать современный, типобезопасный и производительный фундамент векторной алгебры движка `Zzz`, необходимый для матричных преобразований (`Mat4`), кватернионов (`Quat`), вершинных буферов (`Vertex3D`), трансформаций сущностей (`Transform`), камер и системы координат.
- **Статус:** `✅ Выполнено`
- **Зависимости:** `src/math/Types.h`, `src/math/MathIncludes.h`, `src/core/serialize/Serializer.h`.

---

## 2. Архитектурные требования и стандарты движка

### 2.1. Система координат
- **Левосторонняя система координат (Left-Handed: Y-up, Z-forward)**:
  - `Up`: (0, 1, 0)
  - `Down`: (0, -1, 0)
  - `Left`: (-1, 0, 0)
  - `Right`: (1, 0, 0)
  - `Forward`: (0, 0, 1)
  - `Back`: (0, 0, -1)
- **Векторное произведение (Cross Product):** Right x Up = Forward, т.е. (1, 0, 0) x (0, 1, 0) = (0, 0, 1).

### 2.2. Типобезопасность и движковые типы (Strict Engine Types)
- Векторы параметризуются концептом `Arithmetic` со значением по умолчанию `zF32`.
- Запрещено использовать сырые `float`, `double`, `int32_t`, `uint32_t` в интерфейсах.
- **Псевдонимы (Typedefs):**
  - `Vec2f` = `Vec2<zF32>`, `Vec3f` = `Vec3<zF32>`, `Vec4f` = `Vec4<zF32>`
  - `Vec2d` = `Vec2<zF64>`, `Vec3d` = `Vec3<zF64>`, `Vec4d` = `Vec4<zF64>`
  - `Vec2i` = `Vec2<zI32>`, `Vec3i` = `Vec3<zI32>`, `Vec4i` = `Vec4<zI32>`
  - `Vec2u` = `Vec2<zU32>`, `Vec3u` = `Vec3<zU32>`, `Vec4u` = `Vec4<zU32>`

### 2.3. Расположение в памяти (Standard Layout / POD)
- `static_assert(std::is_standard_layout_v<Vec2<zF32>>);` (`sizeof == 8`)
- `static_assert(std::is_standard_layout_v<Vec3<zF32>>);` (`sizeof == 12`)
- `static_assert(std::is_standard_layout_v<Vec4<zF32>>);` (`sizeof == 16`)
- Поля открытые (`public`), доступ к сырому указателю через `data()` и доступ по индексу через `operator[](size_t)`.

### 2.4. Правило №26 (POD Value Objects с открытыми полями)
- Поля `Point2D<T>` открыты (`x, y`), добавлены `data()` и `operator[]`.
- Поля `Size2D<T>` открыты (`width, height`), добавлены `data()` и `operator[]`.
- Поля `Rect2D<T>` открыты (`position, size`).
- Для 100% обратной совместимости сохранены инлайновые методы доступа (`GetX()`, `GetWidth()`, `GetPosition()` и т.д.).

### 2.5. Правило №27 (Организация QA тестов и TestsConfig.h)
- Тесты разложены по подпапкам `src/qa/tests/math/` и `src/qa/tests/core/`.
- Включение/отключение групп тестов управляется централизованно через `src/qa/tests/TestsConfig.h`.

---

## 3. Детальная спецификация интерфейсов

### 3.1. Функционал `Vec2`, `Vec3`, `Vec4`
1. **Конструкторы:**
   - По умолчанию (`constexpr = default`, нулевая инициализация);
   - Из одного скаляра (`explicit constexpr Vec(T scalar)`);
   - Покомпонентные конструкторы;
   - Конструкторы расширения размерности (`Vec3(Vec2, z)`, `Vec4(Vec3, w)`, `Vec4(Vec2, z, w)`);
   - Конвертирующий конструктор `template<Arithmetic U> requires SafelyConvertibleTo<U, T>`.
2. **Арифметика:**
   - Покомпонентные операторы: `+`, `-`, `*`, `/`, `+=`, `-=`, `*=`, `/=`, унарный минус `-v`;
   - Скалярные операции: `v * s`, `s * v`, `v / s`;
   - Сравнение: `==`, `!=` (с эпсилоном для `std::floating_point`).
3. **Геометрия:**
   - `Dot(a, b)` / `v.Dot(other)`;
   - `Cross(a, b)` / `v.Cross(other)` (для `Vec3`, левосторонняя система);
   - `LengthSquared()`, `Length()`, `Distance(a, b)`;
   - `Normalize()`, `Normalized()` (защита от деления на 0);
   - `Lerp(a, b, t)`, `Min(a, b)`, `Max(a, b)`, `Clamp(v, min, max)`;
   - `Reflect(normal)` (для `Vec2` и `Vec3`; для `Vec4` опущено как геометрически не применимое).
4. **Константы направлений (`Vec3`):** `Zero()`, `One()`, `Up()`, `Down()`, `Left()`, `Right()`, `Forward()`, `Back()`.
5. **Сериализация и строковое представление:**
   - `ToString()` и специализация `std::formatter`;
   - Перегрузки `Serialize` / `Deserialize` в `Serializer.h` для `Vec2`, `Vec3`, `Vec4`, `Point2D`, `Size2D`, `Rect2D`.

---

## 4. Структура файлов в проекте

```
src/math/
├── Math.h                      # Подключение math/vector/Vec2.h, Vec3.h, Vec4.h
├── MathIncludes.h              # Концепты Arithmetic, математические заголовки
├── Point2D.h                   # Унификация Point2D (открытые поля x, y, data(), operator[])
├── Size2D.h                    # Унификация Size2D (открытые поля width, height, data(), operator[])
├── Rect2D.h                    # Унификация Rect2D (открытые поля position, size)
└── vector/
    ├── Vec2.h                  # Шаблонная структура Vec2<T>
    ├── Vec3.h                  # Шаблонная структура Vec3<T>
    └── Vec4.h                  # Шаблонная структура Vec4<T>
src/core/serialize/
└── Serializer.h                # Добавление перегрузок Serialize/Deserialize для Vec2, Vec3, Vec4, Point2D, Size2D, Rect2D
src/qa/tests/
├── TestsConfig.h               # Единый конфигуратор включения/отключения наборов тестов
├── CMakeLists.txt              # Сборка тестов по подпапкам
├── math/
│   ├── VectorTests.cpp         # Юнит-тесты Vec2, Vec3, Vec4 (арифметика, LH Cross, геометрия)
│   ├── Point2DTest.cpp         # Юнит-тесты Point2D
│   ├── Size2DTest.cpp          # Юнит-тесты Size2D
│   └── Rect2DTest.cpp          # Юнит-тесты Rect2D
└── core/
    └── SerializationTests.cpp  # Бинарный round-trip в Serializer для всех типов
```

---

## 5. Чек-лист реализации и Definition of Done (DoD)

- [x] Создать `src/math/vector/Vec2.h`
- [x] Создать `src/math/vector/Vec3.h`
- [x] Создать `src/math/vector/Vec4.h`
- [x] Подключить новые заголовки в `src/math/Math.h`
- [x] Обновить `src/math/Point2D.h` (открытые поля `x, y`, `data()`, `operator[]`, обратная совместимость `GetX`/`SetX`)
- [x] Обновить `src/math/Size2D.h` и `src/math/Rect2D.h` согласно Правилу №26 (открытые поля `width/height`, `position/size`)
- [x] Добавить перегрузки `Serialize` и `Deserialize` для `Vec2`, `Vec3`, `Vec4`, `Point2D`, `Size2D`, `Rect2D` в `src/core/serialize/Serializer.h`
- [x] Создать единый конфигуратор `src/qa/tests/TestsConfig.h` и разложить тесты по подпапкам `src/qa/tests/math/` и `src/qa/tests/core/` (Правило №27)
- [x] Собрать тестовый таргет `EngineTests` через CMake / MSVC x64
- [x] Успешно прогнать все 28 тестов (проверка `Dot`, `Cross`, `Normalize`, `Lerp`, `Reflect`, `Length`, деление на 0, бинарный round-trip в `Serializer`, Point2D, Size2D, Rect2D)
- [x] Запросить утверждение у пользователя
- [x] Сделать Git commit: `feat(math): completed stage 01 - Vec2, Vec3, Vec4 and Point2D/Size2D/Rect2D unification`
- [x] Перевести статус Пункта 1 в `✅ Выполнено` в `general_plan.md`
- [ ] Создать спецификацию следующего шага `stage_02_mat4.md`