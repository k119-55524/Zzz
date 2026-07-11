# TODO / Технический долг

Открытые архитектурные задачи, выявленные при обзоре кода 2026-07-05.

## ECS вместо `shared_ptr<Script>` на `GameObject`
Сейчас каждый `GameObject` хранит `std::vector<std::shared_ptr<script::Script>>`. Для сцены с тысячами объектов это cache-unfriendly: разбросанные аллокации, атомарные счётчики ссылок, косвенные вызовы. Стандартное решение — ECS с компонентами в SoA-массивах.

- В `docs/scripting.md` §5.6 ECS обозначен как отложенная 4-я категория `ScriptRegistry`, но не реализован.
- Развилка: свой ECS / EnTT / flecs.
- Влияет на: API компонентов, сериализацию сцены, инспектор в редакторе, hot-reload.
- Чем позже мигрировать — тем больше API-долга.

## Hot-Reload: сохранение состояния скриптов
После `EditorEngine::ReloadScripts` объекты сцены пересоздаются с дефолтами — правки в коде «сбрасывают» состояние. Работающий hot-reload должен восстанавливать поля.

- Заготовка есть: `ScriptRegistry::RegisterInstance/UnregisterInstance/GetActiveInstances` (только `Z_EDITOR`) — но никто не читает.
- Явно отмечено в `docs/scripting.md` §6 как «в реализации нет».
- Нужно: сериализация полей скрипта → clear → reload DLL → создание нового инстанса → десериализация полей. Требует reflection/introspection на поля скрипта (макро-based или C++26 reflection в будущем).

## Автостарт глобальных скриптов в билде игры игнорирует выбор пользователя
`Engine::Initialize` (`src/engine/engine.cpp`) под `#if !Z_EDITOR` стартует **все** зарегистрированные `Game`-скрипты через `ScriptRegistry::GetAllGameNames()`. Редакторский Play (`MainWindowViewModel.PlayCommand`) стартует только то, что выбрано в `game_config.toml` → `global_script_guids`. Это разные наборы, если в проекте есть `Game`-скрипты, которые существуют как ассеты, но сознательно не отмечены как глобальные — билд игры их всё равно запустит, а Play в редакторе — нет.

- Нужен кодоген, параллельный `RegisterAllScripts.cpp` (`AssetsViewModel.GenerateRegisterAllScripts`): резолвить `GlobalScriptGuids` → имена классов на этапе сборки игры и передавать их в `Engine::Initialize` вместо `GetAllGameNames()`.
- Инвалидация такого кодогена должна триггериться не только сканом Assets (как сейчас у `RegisterAllScripts.cpp`), но и изменением `GlobalScriptGuids` в `game_config.toml` — сейчас `RefreshTree()` о конфиге ничего не знает.
- Пока сознательно отложено: реальный билд игры стартует всё зарегистрированное.

## Тесты движка
Сейчас в `src/qa/` ~241 строка. При кроссплатформенности (5 ОС) и кодогене (`RegisterAllScripts.cpp`, `.meta` синхронизация) отсутствие тестов рискованно — регрессии на macOS/Android/iOS найдёт только пользователь.

Приоритетное покрытие:
- `ScriptRegistry` (регистрация, коллизии имён, `Clear`, hot-reload lifecycle).
- `.meta` / GUID: `ScriptMetaFile`, `GuidCollisionScanner`, `ProjectFileSystem.SyncScriptMetaFiles`.
- Undo/Redo команды в редакторе (C# — xUnit/NUnit).
- Логгер: фильтрация по маске, ordering в асинхронной очереди, работа бродкастеров.
- Path (особенно `Path_Apple.mm`).

## Единый стиль обработки ошибок

Одновременно живут три параллельных механизма, каждый со своей семантикой:

1. **`std::expected<T, std::string>`** — используется в 55 файлах. Помощник `UNEXPECTED(fmt, ...)` (`src/common/macroses.h:84`) логирует через `DOutError` и возвращает `std::unexpected(msg)`.
2. **Исключения** — `THROW_RUNTIME(fmt, ...)` → `throw_runtime_error` (`src/common/throw_wrappers.cpp:12`) логирует через `DOutException` и кидает `std::runtime_error`. `try/catch` встречается 103 раза в 28 C++ файлах.
3. **`ensure(cond, msg)`** — `src/common/ensure.h`. Кидает `runtime_error`, но **только в Debug/Development** (внутри `#if Z_DEBUG_BUILD || Z_DEVELOPMENT_BUILD`). В Release это NO-OP: `condition` даже не вычисляется.

### Конкретные места, где это торчит

- **Смесь в одной функции**: `ConfigManager::SaveConfig`/`LoadConfig` (`src/engine/private/platforms/config/ConfigManager.cpp:72,122`) возвращают `expected`, но внутри `try` с тремя `catch`, каждый конвертирует исключение обратно в `UNEXPECTED`.
- **Expected → throw → catch → expected → default**: `ConfigManager::Initialize` (стр. 22) получает `expected` от `GetSettingsDirectory`, вручную вызывает `THROW_RUNTIME` вместо возврата `expected.error()`, ловит в общем catch и молча ставит дефолтный конфиг. По пути логи задваиваются: `UNEXPECTED` пишет `DOutError`, потом `THROW_RUNTIME` пишет `DOutException`, потом `DOutException("Config loading error")` в catch.
- **Throw из конструктора без noexcept-эквивалента**: `Path::Path` (`src/engine/private/core/io/Path.cpp:7`) использует `ensure(...)` для валидации `appName` и `ResolveUserDataDirectory()`. В Debug/Development конструктор бросает. **В Release** `ensure` = NO-OP: конструктор молча продолжает с `m_UserDataDirectory` в дефолтном состоянии, дальше движок работает с невалидным `Path`, падение случится где-то в другом месте с непонятной трассой. Тихая порча — худший сценарий.
- **Двойной уровень защиты в `Engine::Run`** (`src/engine/engine.cpp:62`): возвращает `expected`, но должен ловить исключения — потому что `MainLoop::Run`, `CreateView` не возвращают `expected`, они кидают. На границе подсистем идёт конверсия.

### Проблемы, которые это создаёт

1. Нельзя понять контракт функции по сигнатуре: `SaveConfig` возвращает `expected` — но может ли она бросить? Никакого `noexcept`, никакой гарантии.
2. Логи одной ошибки задваиваются/затраиваются с разными уровнями (`DOutError` от `UNEXPECTED` + `DOutException` от `THROW_RUNTIME` + `DOutException` в catch).
3. Release-сборка теряет `ensure`-проверки. Инварианты («путь валиден», «указатель не null») в проде не проверяются — единственный «работающий» механизм в Release это `THROW_RUNTIME` и `UNEXPECTED`.
4. Читатель кода должен помнить три модели; выбор модели в новом коде непоследовательный.
5. `std::string` как тип ошибки стирает тип: «файл не найден» vs «нет прав» — только парсингом строки.

### Три возможных решения (от минимального к полному)

**A. Минимум — правила без рефакторинга.**  Зафиксировать в `CLAUDE.md`:
- Публичный API возвращает `expected`. Никаких `throw` из публичных функций.
- Внутри реализации `try/catch` только на границе с STL/OS, которые сами кидают. Конверсия в `expected` — сразу.
- `ensure` **никогда** не для инвариантов, влияющих на корректность в Release. Для них — `if (!cond) return UNEXPECTED(...)` или отдельный `Z_FATAL(cond, msg)`, работающий во всех сборках.
- `THROW_RUNTIME` и `UNEXPECTED` не должны дублировать логирование — либо логируем только на границе (где ловим), либо только в точке возникновения.

**B. Средний — новый макрос для критических инвариантов.**  Ввести `Z_FATAL(cond, msg)` (assert-with-message, работает и в Release, при провале → лог + `std::terminate` или corrupt-state exit). Явно отделить его от `ensure`, который остаётся debug-only «assert для отладки». Пройтись по всем `ensure` и решить: заменить на `Z_FATAL` (если инвариант критичен), на `return UNEXPECTED(...)` (если ошибка восстановима), или оставить как есть.

**C. Полный — `expected` везде + типизированный error.**
- Мигрировать всё на `expected` + собственный `enum ErrorCode` + `std::string context` вместо `std::string`.
- Убрать `throw` из движка полностью.
- Конструкторы, способные упасть, заменить на `static expected<Object> Create(...)`. Пример: `Path::Path(appName)` → `Path::Create(appName) -> expected<Path, PathError>`.
- Границы с STL/OS обёрнуты в `try_call([]{ ... }) -> expected<T, ...>`, единая точка конверсии.
- `noexcept` там, где применимо, для проверки инвариантов компилятором.

Файлы, затронутые пунктом: `src/engine/engine.cpp`, `src/common/throw_wrappers.{h,cpp}`, `src/common/ensure.h`, `src/common/macroses.h`, `src/engine/private/platforms/config/ConfigManager.cpp`, `src/engine/private/core/io/Path.cpp`, все конструкторы `Platform_*`, `Window_*`, `Config*` (потенциально).

## Выбор варианта сборки скриптов
Сейчас скрипты в редакторе жестко пересобираются в режиме `Debug` (чтобы корректно работала отладка в студии).
Необходимо:
- Добавить в настройки проекта/редактора (или в UI сборщика) возможность ручного выбора конфигурации сборки (`Debug`, `Release`, `RelWithDebInfo`) для пользовательских скриптов.
- При запуске PlayMode или фоновой перекомпиляции читать этот параметр и пробрасывать в аргумент `--config` для CMake-вызова в `MainWindow.xaml.cs`.
