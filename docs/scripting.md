/**
 * @file scripting.md
 * @brief Единый источник правды по системе скриптинга и системе .meta/GUID ассетов в ZzzEngine
 * и WPF-редакторе.
 *
 * # Система скриптинга и ассетов Zzz
 *
 * ---
 *
 * ## 1. Общие принципы
 *
 * - **Язык разработки:** C++ (`.hpp` + `.cpp`).
 * - **Отсутствие виртуальных функций для событий:** обновления/старт/уничтожение подписываются
 *   через `Event<>` в конструкторах классов, без vtable.
 * - **Владение:** Один `GameObject` может иметь несколько прикрепленных скриптов типа
 *   `script::Script` (владение через `std::shared_ptr`).
 * - **Множественность:** Глобальные скрипты (`script::Game`) и скрипты сцены (`script::Scene`) не
 *   являются синглтонами - одновременно может существовать несколько разных экземпляров каждого.
 * - **Интеграция в игру:** В финальном билде игры все скрипты компилируются статически напрямую в
 *   исполняемый файл без использования DLL.
 * - **Интеграция в редактор:** В редакторе скрипты собираются в отдельную `scripts.dll` для
 *   обеспечения функции горячей перезагрузки (Hot-Reload).
 *
 * ---
 *
 * ## 2. Иерархия классов скриптов
 *
 * Все скрипты находятся в пространстве имён `zzz::script` и наследуются от трёх базовых классов.
 * Физическая структура файлов движка (`src/engine/private/core/`):
 * - `scene/GameObject.h` - базовый игровой объект (`zzz::GameObject`).
 * - `scene/Scene.h` / `Scene.cpp` - базовый скрипт уровня/сцены (`zzz::script::Scene`).
 * - `scene/scripts/ScriptRegistry.h` / `ScriptRegistry.cpp` - единый реестр фабрик скриптов.
 * - `scene/scripts/base_script/Script.h` / `Script.cpp` - компонентный скрипт (`zzz::script::Script`).
 * - `scene/scripts/base_script/Game.h` / `Game.cpp` - глобальный скрипт игры (`zzz::script::Game`).
 *
 * @mermaid
 * graph TD
 *     GameObject["zzz::GameObject"]
 *     Script["zzz::script::Script (MonoBehaviour-like)"]
 *     Game["zzz::script::Game (Global Script)"]
 *     Scene["zzz::script::Scene (Scene Script)"]
 *     Registry["zzz::script::ScriptRegistry"]
 *
 *     GameObject -->|Содержит список| Script
 *     Script -->|Ссылается на| GameObject
 *     Registry -->|Регистрирует фабрики| Script
 *     Registry -->|Регистрирует фабрики| Game
 *     Registry -->|Регистрирует фабрики| Scene
 * @endmermaid
 *
 * ### 2.1. `zzz::script::Script` (аналог MonoBehaviour)
 *
 * Применяется для логики конкретных игровых объектов. Конструктор принимает указатель на
 * объект-владелец `GameObject`. Содержит структуру событий жизненного цикла `Events`
 * (`OnStart`, `OnUpdate(float dt)`, `OnDestroy`); подписка на события - в конструкторе
 * класса-наследника. В режиме редактора (`Z_EDITOR`) инстансы отслеживаются реестром для
 * корректного сброса при Hot-Reload.
 *
 * ```cpp
 * namespace zzz::script {
 *     class Script : public std::enable_shared_from_this<Script> {
 *     public:
 *         explicit Script(GameObject* owner);
 *         virtual ~Script();
 *
 *         GameObject* GetOwner() const { return m_Owner; }
 *         virtual std::string_view GetScriptTypeName() const = 0;
 *
 *         ScriptEvents Events;
 *     private:
 *         GameObject* m_Owner;
 *     };
 * }
 * ```
 *
 * #### Пример использования (PlayerController)
 * ```cpp
 * // PlayerController.hpp
 * #pragma once
 * #include <script/Script.hpp>
 *
 * class PlayerController : public zzz::script::Script {
 * public:
 *     explicit PlayerController(zzz::GameObject* owner);
 * private:
 *     void OnStart();
 *     void OnUpdate(float dt);
 * };
 *
 * // PlayerController.cpp
 * #include "PlayerController.hpp"
 *
 * PlayerController::PlayerController(zzz::GameObject* owner)
 *     : Script(owner)
 * {
 *     Events.OnStart.Subscribe(shared_from_this(), [this] { OnStart(); });
 *     Events.OnUpdate.Subscribe(shared_from_this(), [this](float dt) { OnUpdate(dt); });
 * }
 *
 * void PlayerController::OnStart() { }
 * void PlayerController::OnUpdate(float dt) { }
 * ```
 *
 * ### 2.2. `zzz::script::Game` - глобальный скрипт
 *
 * Существует на протяжении всей жизни приложения, не привязан к `GameObject`. Не синглтон: может
 * существовать и работать одновременно несколько разных экземпляров. Подходит для высокоуровневых
 * систем (аудио-менеджер, менеджер сохранений, инициализаторы).
 *
 * ```cpp
 * namespace zzz::script {
 *     class Game {
 *     public:
 *         Game() = default;
 *         virtual ~Game() = default;
 *         virtual std::string_view GetScriptTypeName() const = 0;
 *     };
 * }
 * ```
 *
 * ### 2.3. `zzz::script::Scene` - скрипт сцены
 *
 * Время жизни ограничено активностью конкретной сцены (карты). Не синглтон: на сцене может быть
 * запущено несколько скриптов сцены одновременно. Подходит для логики уровня, спавнеров, квестов.
 *
 * ```cpp
 * namespace zzz::script {
 *     class Scene {
 *     public:
 *         Scene() = default;
 *         virtual ~Scene() = default;
 *         virtual std::string_view GetScriptTypeName() const = 0;
 *     };
 * }
 * ```
 *
 * ---
 *
 * ## 3. Механизм регистрации скриптов
 *
 * Скрипты регистрируются явно через автоматически генерируемый файл `RegisterAllScripts.cpp`, что
 * позволяет избежать проблем с порядком инициализации статических объектов и отбрасыванием
 * неиспользуемого кода линкером (dead code stripping) в статической сборке игры.
 *
 * Шаблонный метод `ScriptRegistry::Register<T>(name)` на основе базового класса `T` через
 * `if constexpr` сам определяет, в какой из трёх отдельных реестров (`s_GameFactories`,
 * `s_SceneFactories`, `s_ScriptFactories`) положить фабрику - см.
 * [`ScriptRegistry.h`](../src/engine/private/core/scene/scripts/ScriptRegistry.h):
 *
 * ```cpp
 * template<typename T>
 * static void Register(std::string_view name)
 * {
 *     std::string nameStr(name);
 *     if constexpr (std::is_base_of_v<Game, T>)
 *         s_GameFactories[nameStr] = []() { return std::make_shared<T>(); };
 *     else if constexpr (std::is_base_of_v<Scene, T>)
 *         s_SceneFactories[nameStr] = []() { return std::make_shared<T>(); };
 *     else if constexpr (std::is_base_of_v<Script, T>)
 *         s_ScriptFactories[nameStr] = [](GameObject* owner) { return std::make_shared<T>(owner); };
 *     else
 *         static_assert(sizeof(T) == 0, "Unknown script base type");
 * }
 * ```
 *
 * Создание объектов по сохранённому имени класса: `ScriptRegistry::CreateScript/CreateGame/CreateScene(name, ...)`.
 * `ScriptRegistry::Clear()` сбрасывает все три реестра (используется при Hot-Reload).
 *
 * ### Генерация `RegisterAllScripts.cpp`
 *
 * Редактор ([`AssetsViewModel.GenerateRegisterAllScripts`](../src/tools/editor/ViewModels/AssetsViewModel.cs))
 * на каждое обновление дерева ассетов сканирует найденные скрипты и генерирует
 * `Assets/RegisterAllScripts.cpp`:
 *
 * ```cpp
 * #include <ScriptRegistry.h>
 * #include "Player.hpp"
 * #include "Enemy.hpp"
 *
 * extern "C" __declspec(dllexport) void RegisterAllScripts() {
 *     zzz::script::ScriptRegistry::Register<Player>("Player");
 *     zzz::script::ScriptRegistry::Register<Enemy>("Enemy");
 * }
 * ```
 *
 * Файл перезаписывается только если итоговый контент отличается от уже лежащего на диске (чтобы не
 * дёргать лишний раз пересборку, см. §5).
 *
 * **Дубли имени класса.** Так как `s_ScriptFactories`/`s_GameFactories`/`s_SceneFactories` - это
 * `unordered_map<string, ...>`, два разных файла с одинаковым `class_name` дали бы
 * `Register<>(name)` с одним и тем же ключом - вторая фабрика молча перетрёт первую. Генератор это
 * детектирует, логирует ошибку с путями обоих файлов и **не генерирует** `Register<>()` ни для
 * одного из конфликтующих классов, пока пользователь не переименует один из них - явная общая
 * неработоспособность нагляднее, чем непредсказуемая работа "через раз".
 *
 * В статической сборке игры (`game_win`) тот же кодоген выполняется CMake-скриптом на этапе
 * генерации проекта, и `RegisterAllScripts()` вызывается из `main()` перед запуском игрового цикла.
 *
 * ---
 *
 * ## 4. Создание скриптов из шаблонов
 *
 * Новые скрипты создаются через контекстное меню дерева ассетов редактора
 * (ПКМ -> Add -> Script / Game Script / Scene Script), реализация -
 * [`AssetsWidget.CreateScriptFromTemplates`](../src/tools/editor/Views/Widgets/AssetsWidget.xaml.cs).
 *
 * Шаблоны лежат в `src/editor_dll/templates/scripts/` (`{Type}.hpp.template` / `{Type}.cpp.template`):
 * - `{ClassName}` - имя класса, введённое пользователем.
 * - `{BaseClass}` - базовый класс (например, `zzz::script::Script`).
 * - `{IncludePath}` - путь подключения базового класса.
 * - `{Date}` - текущая дата генерации.
 *
 * Создаётся обязательная пара `.hpp` + `.cpp` (`.cpp` может быть почти пустым - просто инклюд) и
 * файл метаданных `.meta` (см. §5) с новым GUID.
 *
 * ---
 *
 * ## 5. Система `.meta`-файлов и GUID ассетов
 *
 * Дизайн рассчитан сразу как общая "AssetDatabase" (GUID <-> путь <-> тип ассета), а не специфичная
 * только под скрипты - на текущем этапе единственный подключённый тип ассета - скрипт.
 *
 * ### 5.1. Зачем GUID
 *
 * Имя класса/файла читаемо в Git, но ломается при переименовании. GUID не меняется никогда и
 * однозначно идентифицирует ассет независимо от пути и имени.
 *
 * - **Сцены, префабы и любые ссылки на ресурсы ссылаются на GUID, а не на имя класса/файла.**
 *   Переименование скрипта не требует правки ни одного файла сцены.
 * - **GUID - это слой редактора и сборщика, а не движка.** `ScriptRegistry` (движок) по-прежнему
 *   работает по имени класса (`Register<T>("ClassName")`, см. §3) - ничего в `src/engine` ради
 *   GUID не меняется. Перед компиляцией финальной игры редактор/сборщик разворачивает все
 *   GUID-ссылки из сцен в конкретные имена классов. Если GUID не резолвится (ассет удалён/потерян) -
 *   это ошибка сборки (build должен упасть, а не подставить заглушку молча); сам редактор при
 *   работе старается самовосстановиться (см. §5.4) и не даёт пользователю долго работать в
 *   сломанном состоянии.
 *
 * ### 5.2. Формат `.meta`
 *
 * Каждый скрипт - это обязательная пара `Player.hpp` + `Player.cpp` и файл метаданных `Player.meta`
 * рядом (без `.hpp` в названии), формат JSON:
 *
 * ```json
 * {
 *   "guid": "b5f36e84-18c7-4fd9-b22e-a567df489bc3",
 *   "class_name": "Player"
 * }
 * ```
 *
 * Метафайлы - на каждый ассет свой (а не одна общая таблица проекта), чтобы избежать
 * merge-конфликтов в Git при параллельной работе нескольких разработчиков и не терять привязку при
 * переносе файла. `.meta` всегда скрыта из дерева ассетов редактора - это служебные данные, не
 * ассет. Единая точка чтения/записи формата -
 * [`ScriptMetaFile`](../src/tools/editor/Services/Project/Infrastructure/ScriptMetaFile.cs).
 *
 * ### 5.3. Жизненный цикл `.meta` в дереве проекта
 *
 * | Операция | Поведение |
 * |---|---|
 * | Create | `.meta` создаётся вместе с `.hpp`/`.cpp` с новым GUID |
 * | Delete | `.meta` удаляется вместе с `.hpp`/`.cpp` одной командой |
 * | Rename | `.hpp`/`.cpp`/`.meta` переименовываются одной атомарной командой (`CompositeCommand`), `class_name` внутри `.meta` обновляется (`UpdateScriptMetaClassNameCommand`); GUID не меняется |
 * | Move/Drag&Drop | Через тот же `MoveOrRenameCommand`, что и rename |
 *
 * Rename - это **одна** команда в Undo/Redo (а не 2-3 последовательные), поэтому Ctrl+Z отменяет
 * переименование всей триады сразу, а не файл за файлом.
 *
 * ### 5.4. Полный скан и обработка извне
 *
 * - **Полный скан**
 *   ([`ProjectFileSystem.SyncScriptMetaFiles`](../src/tools/editor/Services/Project/Infrastructure/ProjectFileSystem.cs))
 *   выполняется один раз при открытии/смене проекта (`AssetsViewModel.OnProjectOpened`), а не на
 *   каждый `RefreshTree()`:
 *   - `.hpp` без `.meta` -> генерируется новая `.meta` с новым GUID.
 *   - `.meta` без `.hpp` (юзер вручную удалил `.hpp` в Проводнике, а `.meta` забыл) -> удаляется
 *     молча, без диалогов подтверждения (это безопасная операция - в отличие от коллизий GUID,
 *     см. §5.5).
 * - **Изменения во время работы редактора** отслеживаются точечно через `FileSystemWatcher`
 *   (`AssetsViewModel`), без полного пересканирования проекта. Обработка вынесена в интерфейс
 *   [`IAssetWatchHandler`](../src/tools/editor/Services/Project/Infrastructure/IAssetWatchHandler.cs),
 *   чтобы новые типы ассетов (текстуры, модели - пока не реализованы) подключались своим
 *   обработчиком без переделки самого watcher'а. Сейчас зарегистрирован только
 *   [`ScriptAssetWatchHandler`](../src/tools/editor/Services/Project/Infrastructure/ScriptAssetWatchHandler.cs)
 *   (слушает `.hpp`/`.cpp`, обрабатывает Created/Deleted/Renamed).
 *
 * ### 5.5. Коллизии GUID - сознательно отложено
 *
 * Если два разных файла на диске несут один и тот же GUID (например, копия скрипта через
 * Проводник), это **тихая** порча данных: ссылки на объектах могут массово указывать не туда, и
 * никто не заметит сразу. Запланированное (но не реализованное) решение:
 *
 * - Полный скан собирает карту `GUID -> [пути]` и батчем показывает модальный диалог со всеми
 *   найденными конфликтами сразу (а не по одному окну на конфликт).
 * - Пользователь явно выбирает, какой файл сохраняет GUID; остальным генерируется новый GUID
 *   автоматически.
 * - Диалог не закрывается без выбора (без отмены) - оставлять состояние неразрешённым нельзя.
 * - Когда появится система сцен, в диалог нужно добавить список затронутых ссылок ("этот GUID
 *   используется в N сценах/объектах") - сейчас этого нет, потому что самих сцен ещё не существует.
 *
 * Точка интеграции уже на месте:
 * [`GuidCollisionScanner.Scan`](../src/tools/editor/Services/Project/Infrastructure/GuidCollisionScanner.cs)
 * вызывается из `SyncScriptMetaFiles` на каждом полном скане, но пока всегда возвращает пустой
 * список - сознательная заглушка, не забытая недоделка.
 *
 * ### 5.6. Отложено на будущее (вне текущего объёма)
 *
 * - **Коллизии GUID** - см. §5.5.
 * - **Meta-файлы для ресурсов** (текстуры, модели и т.д.) с настройками импорта. Когда это
 *   появится, в схему `.meta` нужно добавить дискриминатор `"type"` (`"script"` / `"texture"` /
 *   ...), чтобы парсер знал, какую форму документа ожидать (`class_name` vs `import`-настройки).
 * - **ECS-компоненты как 4-я категория реестра.** `ScriptRegistry` уже разделяет фабрики на три
 *   списка (`Game` / `Scene` / `Script`-MonoBehaviour-like, см. §3). ECS-компоненты (потенциально
 *   многочисленные, в отличие от глобальных скриптов) предполагается выделить в отдельный список
 *   ради скорости поиска при загрузке сцены - реализация отложена.
 *
 * ---
 *
 * ## 6. Горячая перезагрузка (Hot-Reload) в редакторе
 *
 * ### Отслеживание и сборка
 * 1. WPF-редактор при возвращении фокуса в окно (`Window.Activated`) сравнивает время изменения
 *    всех `.hpp`/`.cpp` в `Assets/` с временем сборки `bin/scripts.dll`.
 * 2. Если есть более новые файлы (или `scripts.dll` не существует) - запускается фоновая сборка
 *    через CMake: конфигурация + `cmake --build <ProjectDir>/.editor/build --config Debug`.
 * 3. После успешного завершения вызывается перезагрузка в движке.
 *
 * ### Фаза перезагрузки (Reload)
 * Во избежание крашей из-за выгрузки машинного кода DLL перезагрузка выполняется строго по шагам:
 * 1. **Сериализация:** движок проходит по списку живых инстансов (`ScriptRegistry::GetActiveInstances()`)
 *    и сохраняет их переменные и связи.
 * 2. **Очистка объектов:** все скрипты на сцене уничтожаются, `weak_ptr`-ссылки инвалидируются.
 * 3. **Выгрузка DLL:** `FreeLibrary` на старую `scripts.dll` (через временную копию
 *    `scripts_temp.dll`, чтобы Windows не блокировала файл на запись при повторной компиляции).
 * 4. **Загрузка новой DLL:** `LoadLibrary` на новую `scripts_temp.dll`, заново регистрируются все
 *    типы (`RegisterAllScripts`).
 * 5. **Создание и восстановление:** скрипты создаются на объектах заново из нового реестра
 *    фабрик, восстанавливается их сохранённое состояние.
 *
 * #### Оптимизация для игры (Zero overhead)
 * Список живых инстансов `s_ActiveInstances` используется только в редакторе и вырезается в игре:
 * ```cpp
 * Script::Script(GameObject* owner) : m_Owner(owner) {
 * #if Z_EDITOR
 *     ScriptRegistry::RegisterInstance(this);
 * #endif
 * }
 * ```
 */
