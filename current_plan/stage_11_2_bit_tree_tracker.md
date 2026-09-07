# Этап 11.2: 64-арное битовое дерево изменений (`BitTreeTracker`)

## 1. Контекст и цели подэтапа
- **Номер подпункта:** **Пункт 11.2** (часть Этапа 11: Структура хранения GameObject в сцене).
- **Цель:** Разработать фундаментальную, не зависящую от движка структуру данных `BitTreeTracker` в модуле `core`:
  1. $O(1)$ пометка битов изменений (Set/MarkDirty).
  2. 64-арная иерархическая организация: каждый уровень редуцирует 64 бита нижележащего уровня в 1 бит родителя (`uint64_t`).
  3. Обход установленных битов без холостых циклов с помощью аппаратной SIMD/CPU-инструкции `_BitScanForward64` / `std::countr_zero`.
  4. Быстрая очистка за $O(1)$ при отсутствии изменений (проверка корня на 0) или блочным `memset` за <0.05 мс.
  5. Покрытие 100% функционала модульными тестами.
- **Статус:** ⏳ В процессе разработки.
- **Зависимости:** `src/core/CoreIncludes.h`, компиляторные интринсики `_BitScanForward64` (MSVC) / `__builtin_ctzll` (GCC/Clang).

### Физическое размещение файлов
```text
src/
├── core/
│   └── containers/
│       ├── BitTreeTracker.h                 <-- Класс 64-арного битового дерева
│       └── BitTreeTracker.cpp               <-- Реализация методов роста и обхода
│
tests/ (или src/qa/tests/)
└── core/
    └── BitTreeTrackerTests.cpp              <-- Модульные тесты
```

---

## 2. Архитектурная спецификация `BitTreeTracker`

### 2.1. Математическая модель 64-арного дерева
Для отслеживания изменений в $N$ узлах (листьях) дерево формирует уровни (Levels):
- **Уровень 0 (Листья):** битовая маска узлов. $K_0 = \lceil N / 64 \rceil$ слов `uint64_t`. Бит $i$ означает, что узел $i$ изменён (`isDirty`).
- **Уровень 1:** $K_1 = \lceil K_0 / 64 \rceil$ слов. Бит $j$ означает, что в слове $j$ уровня 0 есть хотя бы один установленный бит.
- **Уровень $L$ (Корень):** ровно 1 слово `uint64_t`. Если корень равен 0, во всём дереве гарантированно нет ни одного изменения ($O(1)$ проверка).

Все уровни упакованы в **один непрерывный массив** `std::vector<uint64_t> m_Words`. Для каждого уровня предрассчитывается смещение в словах `m_LevelOffsets[level]`.

### 2.2. Спецификация класса `BitTreeTracker`

```cpp
namespace zzz::core
{
    class BitTreeTracker final
    {
    public:
        BitTreeTracker() = default;
        explicit BitTreeTracker(size_t initialCapacity);
        ~BitTreeTracker() = default;

        BitTreeTracker(const BitTreeTracker&) = default;
        BitTreeTracker& operator=(const BitTreeTracker&) = default;
        BitTreeTracker(BitTreeTracker&&) noexcept = default;
        BitTreeTracker& operator=(BitTreeTracker&&) noexcept = default;

        /// @brief Динамическое расширение дерева под требуемое число элементов.
        void EnsureCapacity(size_t capacity);

        /// @brief Установка бита по индексу узла (O(1) по числу уровней <= 3).
        void Set(uint32_t index) noexcept;

        /// @brief Сброс конкретного бита (O(1)).
        void Reset(uint32_t index) noexcept;

        /// @brief Проверка, установлен ли бит.
        [[nodiscard]] bool Test(uint32_t index) const noexcept;

        /// @brief Проверка, есть ли хоть один установленный бит в дереве (O(1) по корню).
        [[nodiscard]] bool Any() const noexcept
        {
            return !m_Words.empty() && (m_Words[m_RootWordIndex] != 0ULL);
        }

        /// @brief Очистка всех битов (быстрый сброс memset).
        void Clear() noexcept;

        /// @brief Итеративный обход всех установленных битов через BitScanForward.
        /// @param visitor Функция обратного вызова void(uint32_t index).
        template<typename Func>
        void ForEachSetBit(Func&& visitor) const;

        [[nodiscard]] size_t GetCapacity() const noexcept { return m_Capacity; }
        [[nodiscard]] size_t GetWordCount() const noexcept { return m_Words.size(); }

    private:
        size_t                m_Capacity{ 0 };
        size_t                m_RootWordIndex{ 0 };
        std::vector<size_t>   m_LevelOffsets; // Смещения уровней в m_Words
        std::vector<size_t>   m_LevelWordCounts;
        std::vector<uint64_t> m_Words;        // Единый непрерывный буфер всех уровней дерева
    };
}
```

### 2.3. Алгоритм быстрого обхода (`ForEachSetBit`)
Обход начинается с корня. На каждом шаге:
1. Берётся текущее ненулевое слово `uint64_t val`.
2. Извлекается младший единичный бит: `uint32_t bit = std::countr_zero(val)`.
3. Позиция пересчитывается в индекс дочернего блока на следующем уровне.
4. Снятие бита: `val &= (val - 1)`.
5. На уровне листьев вызывается `visitor(nodeIndex)`.
Такой спуск пропускает пустые блоки по 64, 4096 и 262144 элементов за один такт процессора!

---

## 3. Чек-лист Definition of Done (DoD)

- [ ] Реализовать `src/core/containers/BitTreeTracker.h` и `.cpp`
- [ ] Зарегистрировать `BitTreeTracker` в `src/core/CMakeLists.txt`
- [ ] Реализовать модульные тесты в `tests/core/BitTreeTrackerTests.cpp`:
  - Инициализация и `EnsureCapacity`
  - Пометка одного бита, проверка `Test` и `Any`
  - Граничные индексы (0, 63, 64, 4095, 4096)
  - Сброс бита `Reset` и проверка обновления родительских уровней
  - Корректность обхода `ForEachSetBit` на случайных выборках
  - Быстрая очистка `Clear` (проверка корня и листьев)
- [ ] Проверить сборку под MSVC x64 + Ninja (0 ошибок)
