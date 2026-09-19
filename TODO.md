# Технический долг и задачи (TODO)

## 1. Разрыв циклической зависимости `core ↔ logger`

### Описание проблемы:
В проекте существует циклическая зависимость между модулями `core` и `logger`:
1. **`core → logger`:** макрос `DOut` в [`src/core/utils/macros/LogMacros.h`](src/core/utils/macros/LogMacros.h) напрямую обращается к синглтону `::zzz::logger::g_Logger.LogMessage(...)`, из-за чего модуль `core` требует объявления и типов из `logger`.
2. **`logger → core`:** модуль `logger` (в частности, сетевой броадкастер `NetworkBroadcaster` и пакеты `LogEntry`) использует `core::Serializer`, `core::Time` и базовые типы ядра.
3. **CMake:** взаимная линковка `target_link_libraries(core_lib logger_lib)` и `target_link_libraries(logger_lib core_lib)`.

Хотя компоновщик MSVC/Ninja для статических библиотек на Windows разрешает этот цикл, на других платформах (Linux `ld`, Android NDK) или при сборке разделяемых библиотек (`.so`/`.dll`) циклическая зависимость может приводить к ошибкам компоновки.

---

### Варианты решения:

#### Вариант А: Log Sink / Function Pointer в Core (Рекомендуемый)
- В `core` создаётся легковесный заголовок диспетчеризации (например, `src/core/logger/LogDispatcher.h`) с функциональными указателями:
  ```cpp
  namespace zzz::core
  {
      using LogCallback = void(*)(eLogMessageType type, const std::source_location& loc, const LogCategory& cat, std::string_view msg);
      using FilterCallback = bool(*)(const LogCategory& cat);

      inline LogCallback g_LogCallback = nullptr;
      inline FilterCallback g_CategoryFilter = nullptr;

      void SetLogHandler(LogCallback logCb, FilterCallback filterCb) noexcept;
  }
  ```
- Макрос `DOut` в `core` переключается на вызов `g_LogCallback` и больше не включает `logger.h`.
- Модуль `logger` зависит от `core` и при старте (`Logger::Initialize()`) регистрирует свой обработчик.
- Зависимость становится строго однонаправленной: `logger_lib → core_lib`.

#### Вариант Б: Слияние `logger` внутрь `core`
- Перенести `src/logger/` в `src/core/logger/`, сделав логгер штатной подсистемой `core_lib`.
- Устраняет разделение на две библиотеки, упрощает сборку и конфигурацию CMake.

---

### Статус:
- Отложено пользователем до этапа мультиплатформенной стабилизации / рефакторинга ядра.
