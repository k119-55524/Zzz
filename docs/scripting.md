/**
 * @file scripting.md
 * @brief Единый источник правды по системе скриптинга и системе .meta/GUID ассетов в ZzzEngine
 * и WPF-редакторе.
 *
 * # Система скриптинга и ассетов Zzz
 *
 * ---
 *
 * ## 0. Архитектура системы скриптов
 *
 * ### 0.1. Четыре вида скриптов
 * Движок поддерживает 4 типа скриптов для реализации игровой логики:
 * 1. **`zzz::script::GameScript` (Глобальный скрипт):** Не привязан к сцене или объектам. Существует на протяжении всей работы приложения. Идеально подходит для написания менеджеров (аудио, сохранения, глобальный стейт).
 * 2. **`zzz::script::SceneScript` (Скрипт сцены):** Привязан к конкретной сцене (карте). Живет, пока активна сцена. Подходит для логики уровня, спавнеров и квестов.
 * 3. **`zzz::script::Script` (Компонентный скрипт / MonoBehaviour-like):** Привязывается к конкретному `GameObject` на сцене. Подходит для контроллеров игрока, поведения ИИ.
 * 4. **Обычные C++ классы и структуры:** Вы не обязаны наследовать весь свой код от базовых скриптов движка. Обычные C++ классы, структуры данных или математические хелперы свободно используются внутри движковых скриптов как обычный C++ код.
 *
 * ### 0.2. Создание скриптов
 * Все 4 типа скриптов и C++ классов могут создаваться в Редакторе. При создании через интерфейс редактора:
 * - Автоматически генерируются файлы `.hpp`, `.cpp`, `.meta` с уникальным GUID.
 * - Скрипт автоматически прописывается в реестре `RegisterAllScripts.cpp`.
 *
 * ### 0.3. Модель исполнения: Компиляция vs Рантайм
 * - **Компиляция (Собираем всё):** Поскольку в C++ сложно динамически вычленить только используемые свободные классы, в общую библиотеку (`scripts.dll`) **включаются все скрипты** проекта. При этом сама система сборки поддерживает инкрементальную компиляцию — пересобираются только измененные файлы, что делает процесс быстрым.
 * - **Рантайм (Работает только нужное):** Отрабатывают (инстанцируются и получают события) только те скрипты, которые явно затребованы игрой: прописанные в глобальном `GameConfig`, прикрепленные к активной сцене или к существующим объектам.
 *
 * ### 0.4. Варианты сборки и деплоя проекта
 * Архитектура системы сборки предполагает 3 основных варианта (workflow):
 * 1. **Вариант А. Деплой в платформенные проекты:** Редактор подготавливает весь контент (ассеты) и компилирует скрипты, после чего деплоит их в существующие заранее подготовленные проекты-оболочки (Android Studio, Xcode, Visual Studio и т.д.) для проверки сборки, тестирования платформозависимых фич и деплоя на девайсы.
 * 2. **Вариант Б. Сборка среды для C++ разработчика (В планах):** Генерация проекта (например, через CMake), где библиотека движка и библиотека скриптов подключаются в IDE для написания логики игры с автокомплитом и глубокой отладки. (Запланировано на будущее, пока не реализовано).
 * 3. **Вариант В. Standalone-сборка:** Финальная автоматическая сборка готового продукта. Выдает релизный билд со всеми запакованными ресурсами, готовый к дистрибуции.
 *
 * **Экспорт конфигурации (Универсальный сериализатор):**
 * Для передачи списка активных скриптов (например, глобальных из `GameConfig` или списков скриптов сцен) в платформенные проекты (Вариант А) используется **универсальный сериализатор**, расположенный на стороне движка. Сериализатор должен принимать пути (путь к файлу), так как он будет переиспользоваться во множестве подсистем для сохранения/чтения.
 * 
 * Формат выходного файла зависит от цели сборки:
 * - При финальной **сборке игры** файл глобальных скриптов экспортируется в **бинарном** формате.
 * - Во всех остальных случаях (разработка, деплой для отладки) используется **TOML**.
 * 
 * Редактор во время сборки использует контроллер, в который прокидывается этот сериализатор, для создания файла конфигурации. Во время пост-сборки этот файл копируется в платформенный проект вместе с ресурсами и читается движком при старте для инициализации нужных скриптов.
 *

 * ---
 *
 * ## 1. Общие принципы
 *
 * - **Идентичность поведения:** Механизмы работы со скриптами должны обеспечивать абсолютно идентичное поведение во всех средах: при запуске режима Play в редакторе (с использованием DLL), при сборке и отладке в платформенном проекте, а также в финальной собранной игре (со статической линковкой).
 * - **Язык разработки:** C++ (`.hpp` + `.cpp`).
 * - **Инкапсуляция подписок на события:** События сгруппированы по уровню в `EventBus`-классах
 *   (`ProjectEventBus`/`SceneEventBus`/`GameObjectEventBus`, см.
 *   [`EventBus.h`](../src/engine/public/core/events/EventBus.h)). Сами события (`Event<>`) скрыты внутри `EventBus` (`private`).
 *   Каждый базовый скрипт (`GameScript`, `SceneScript`, `Script`) имеет приватный метод `Init(...)`, который получает и сохраняет `EventBus`. 
 *   Пользовательские классы скриптов реализуют единственный чисто виртуальный метод `InitScript()`, 
 *   в котором они подписывают свои методы на события с помощью защищенных функций базового класса (например, `SubscribeToUpdate([this](float dt){ MyTick(dt); })`). 
 *   Шаблоны редактора (в `src/editor_dll/templates/scripts/`) настроены на генерацию именно `InitScript() override`.
 *   Пользовательский скрипт никогда напрямую не взаимодействует с `EventBus`. Под капотом `Event` использует `std::weak_ptr` для хранения подписчиков, что архитектурно гарантирует автоматическую безопасную отписку при уничтожении скрипта.
 * - **Владение:** Один `GameObject` может иметь несколько прикрепленных скриптов типа
 *   `script::Script` (владение через `std::shared_ptr`).
 * - **Множественность:** Глобальные скрипты (`script::GameScript`) и скрипты сцены
 *   (`script::SceneScript`) не являются синглтонами - одновременно может существовать несколько
 *   разных экземпляров каждого.
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
 * Физическая структура файлов движка (`src/engine/public/core/` и `private/core/`):
 * - `scene/GameObject.h` - базовый игровой объект (`zzz::GameObject`).
 * - `scene/scripts/ScriptRegistry.h` / `ScriptRegistry.cpp` - единый реестр фабрик скриптов.
 * - `scene/scripts/base_script/Script.h` / `Script.cpp` - компонентный скрипт (`zzz::script::Script`).
 * - `scene/scripts/base_script/GameScript.h` / `GameScript.cpp` - глобальный скрипт игры (`zzz::script::GameScript`).
 * - `scene/scripts/base_script/SceneScript.h` / `SceneScript.cpp` - скрипт сцены (`zzz::script::SceneScript`).
 * - `events/EventBus.h` - `ProjectEventBus`/`SceneEventBus`/`GameObjectEventBus`, через которые
 *   скрипты каждого уровня получают `OnStart`/`OnStop`/`OnUpdate`.
 *
 * `zzz::script::Scene` (`scene/Scene.h`) - отдельный, пока не используемый класс; несмотря на
 * похожее имя, это не базовый класс для скриптов сцены (им является `SceneScript`).
 *
 * @mermaid
 * graph TD
 *     GameObject["zzz::GameObject"]
 *     Script["zzz::script::Script (MonoBehaviour-like)"]
 *     GameScript["zzz::script::GameScript (Global Script)"]
 *     SceneScript["zzz::script::SceneScript (Scene Script)"]
 *     Registry["zzz::script::ScriptRegistry"]
 *     ProjectBus["zzz::engine::ProjectEventBus"]
 *     SceneBus["zzz::engine::SceneEventBus"]
 *     ObjectBus["zzz::engine::GameObjectEventBus"]
 *
 *     GameObject -->|Содержит список| Script
 *     Script -->|Ссылается на| GameObject
 *     Registry -->|Регистрирует фабрики| Script
 *     Registry -->|Регистрирует фабрики| GameScript
 *     Registry -->|Регистрирует фабрики| SceneScript
 *     GameScript -->|Init подписывается на| ProjectBus
 *     SceneScript -->|Init подписывается на| SceneBus
 *     Script -->|Init подписывается на| ObjectBus
 * @endmermaid
 *
 * ### 2.1. `zzz::script::Script` (аналог MonoBehaviour)
 *
 * Применяется для логики конкретных игровых объектов. Конструктор принимает указатель на
 * объект-владелец `GameObject`. Подписка на `OnStart`/`OnUpdate(float dt)`/`OnStop` происходит не
 * в конструкторе, а в приватном переопределении
 * `Init(std::shared_ptr<zzz::engine::GameObjectEventBus> bus)`, которое должен вызвать владелец
 * скрипта после его создания. В режиме редактора (`Z_EDITOR`) инстансы отслеживаются реестром для
 * корректного сброса при Hot-Reload.
 *
 * > **Текущее состояние:** привязка `Script` к `GameObject` (`GameObject::AddScript`) пока не
 * > создаёт `GameObjectEventBus` и не вызывает `Init` - это часть ещё не реализованной интеграции
 * > `GameObject`/`Scene` в рантайме (см. TODO.md, тема ECS). Сгенерированный по шаблону `Script`
 * > компилируется, но его `OnStart`/`OnUpdate`/`OnStop` пока не вызываются ни для одного экземпляра.
 *
 * ```cpp
 * namespace zzz::script {
 *     class Script : public std::enable_shared_from_this<Script> {
 *     public:
 *         Script() = delete;
 *         explicit Script(GameObject* owner);
 *         virtual ~Script();
 *
 *         GameObject* GetOwner() const { return m_Owner; }
 *
 *     private:
 *         virtual void Init(std::shared_ptr<zzz::engine::GameObjectEventBus> bus) = 0;
 *
 *         GameObject* m_Owner;
 *     };
 * }
 * ```
 *
 * #### Пример использования (PlayerController)
 * ```cpp
 * // PlayerController.hpp
 * #pragma once
 * #include <Script.h>
 *
 * class PlayerController : public zzz::script::Script {
 * public:
 *     explicit PlayerController(zzz::GameObject* owner);
 * private:
 *     void Init(std::shared_ptr<zzz::engine::GameObjectEventBus> bus) override;
 *
 *     void OnStart();
 *     void OnUpdate(float dt);
 *     void OnStop();
 * };
 *
 * // PlayerController.cpp
 * #include "PlayerController.hpp"
 *
 * PlayerController::PlayerController(zzz::GameObject* owner)
 *     : Script(owner)
 * {
 * }
 *
 * void PlayerController::Init(std::shared_ptr<zzz::engine::GameObjectEventBus> bus)
 * {
 *     bus->OnStart.Subscribe(shared_from_this(), [this] { OnStart(); });
 *     bus->OnUpdate.Subscribe(shared_from_this(), [this](float dt) { OnUpdate(dt); });
 *     bus->OnStop.Subscribe(shared_from_this(), [this] { OnStop(); });
 * }
 *
 * void PlayerController::OnStart() { }
 * void PlayerController::OnUpdate(float dt) { }
 * void PlayerController::OnStop() { }
 * ```
 *
 * ### 2.2. `zzz::script::GameScript` - глобальный скрипт
 *
 * Существует на протяжении всей жизни приложения, не привязан к `GameObject`. Не синглтон: может
 * существовать и работать одновременно несколько разных экземпляров. Подходит для высокоуровневых
 * систем (аудио-менеджер, менеджер сохранений, инициализаторы).
 * `Init(std::shared_ptr<zzz::engine::ProjectEventBus> bus)` вызывается движком
 * (`Engine::StartGame`, который хранит `shared_ptr<GameScript>` в `m_GlobalGameScripts` и является
 * `friend`-ом класса) - подписываться на `OnStart`/`OnUpdate(const zzz::engine::Time&)` нужно там.
 *
 * ```cpp
 * namespace zzz::script {
 *     class GameScript : public std::enable_shared_from_this<GameScript> {
 *     public:
 *         GameScript() = default;
 *         virtual ~GameScript() = default;
 *
 *     private:
 *         friend class zzz::engine::Engine;
 *         virtual void Init(std::shared_ptr<zzz::engine::ProjectEventBus> bus) = 0;
 *     };
 * }
 * ```
 *
 * ### 2.3. `zzz::script::SceneScript` - скрипт сцены
 *
 * Время жизни ограничено активностью конкретной сцены (карты). Не синглтон: на сцене может быть
 * запущено несколько скриптов сцены одновременно. Подходит для логики уровня, спавнеров, квестов.
 * Как и у `Script`/`GameScript`, подписка на события идёт через приватный
 * `Init(std::shared_ptr<zzz::engine::SceneEventBus> bus)`.
 *
 * > **Текущее состояние:** система сцен ещё не реализована (см. §5.1) - `SceneScript::Init` пока
 * > никем не вызывается; вызывающий код появится вместе со Scene-рантаймом.
 *
 * ```cpp
 * namespace zzz::script {
 *     class SceneScript : public std::enable_shared_from_this<SceneScript> {
 *     public:
 *         SceneScript() = default;
 *         virtual ~SceneScript() = default;
 *
 *     private:
 *         virtual void Init(std::shared_ptr<zzz::engine::SceneEventBus> bus) = 0;
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
 * `if constexpr` сам определяет, в какой из трёх отдельных реестров (`s_GameScriptFactories`,
 * `s_SceneScriptFactories`, `s_ScriptFactories`) положить фабрику - см.
 * [`ScriptRegistry.h`](../src/engine/public/core/scene/scripts/ScriptRegistry.h):
 *
 * ```cpp
 * template<typename T>
 * static void Register(std::string_view name)
 * {
 *     std::string nameStr(name);
 *     if constexpr (std::is_base_of_v<GameScript, T>)
 *         s_GameScriptFactories[nameStr] = []() { return safe_make_shared<T>(); };
 *     else if constexpr (std::is_base_of_v<SceneScript, T>)
 *         s_SceneScriptFactories[nameStr] = []() { return safe_make_shared<T>(); };
 *     else if constexpr (std::is_base_of_v<Script, T>)
 *         s_ScriptFactories[nameStr] = [](GameObject* owner) { return safe_make_shared<T>(owner); };
 *     else
 *         static_assert(sizeof(T) == 0, "Unknown script base type");
 * }
 * ```
 *
 * Создание объектов по сохранённому имени класса:
 * `ScriptRegistry::CreateScript/CreateGameScript/CreateSceneScript(name, ...)`.
 * `ScriptRegistry::Clear()` сбрасывает все три реестра (используется при Hot-Reload).
 *
 * ### Генерация `RegisterAllScripts.cpp`
 *
 * Редактор ([`AssetsViewModel.GenerateRegisterAllScripts`](../src/tools/editor/ViewModels/AssetsViewModel.cs))
 * на каждое обновление дерева ассетов сканирует найденные скрипты и генерирует
 * `.editor/RegisterAllScripts.cpp`:
 *
 * ```cpp
 * #include <ScriptRegistry.h>
 * #include "Player.hpp"
 * #include "Enemy.hpp"
 *
 * extern "C" __declspec(dllexport) void RegisterAllScripts() {
 *     zzz::script::ScriptRegistry::Register<Gameplay::Player>("Gameplay::Player");
 *     zzz::script::ScriptRegistry::Register<Enemy>("Enemy");
 * }
 * ```
 *
 * Файл перезаписывается только если итоговый контент отличается от уже лежащего на диске (чтобы не
 * дёргать лишний раз пересборку, см. §5).
 *
 * **Дубли полного имени класса.** Так как `s_ScriptFactories`/`s_GameFactories`/`s_SceneFactories` - это
 * `unordered_map<string, ...>`, два разных файла с одинаковым `qualified_name`
 * (`namespace::class_name`, либо просто `class_name` без namespace) дали бы
 * `Register<>(name)` с одним и тем же ключом - вторая фабрика молча перетрёт первую. Генератор это
 * детектирует, логирует ошибку с путями обоих файлов и **не генерирует** `Register<>()` ни для
 * одного из конфликтующих классов, пока пользователь не изменит namespace или имя одного из них - явная общая
 * неработоспособность нагляднее, чем непредсказуемая работа "через раз". Это архитектурно правильное решение, защищающее проект от неочевидных ошибок линкера C++ (нарушение One Definition Rule) еще до этапа компиляции.
 *
 * В статической сборке игры (`game_win`) тот же кодоген выполняется CMake-скриптом на этапе
 * генерации проекта, и `RegisterAllScripts()` вызывается из `main()` перед запуском игрового цикла.
 *
 * ---
 *
 * ## 4. Создание скриптов из шаблонов
 *
 * Новые скрипты создаются через контекстное меню дерева ассетов редактора - либо ПКМ по узлу
 * (Add -> Add Script), либо ПКМ по пустому месту дерева. Пункт меню один - базовый тип (Script/
 * Game/Scene) выбирается не в меню, а внутри диалога создания, который открывается сразу:
 *
 * - базовый скрипт (`Script`, `Game`, `Scene`), по умолчанию `Script`/MonoBehaviour-like;
 * - namespace в C++-формате через `::` (можно оставить пустым);
 * - имя скрипта/класса.
 *
 * Диалог валидирует ввод сразу при наборе. `Add` активна только если имя является корректным C++
 * идентификатором, namespace состоит из корректных C++ идентификаторов, в целевой папке нет файлов
 * `{Name}.hpp`/`{Name}.cpp`/`{Name}.meta`, а во всём проекте нет скрипта с тем же
 * `qualified_name`. История namespace хранится в глобальной сессии редактора между запусками;
 * элементы истории можно удалить прямо из выпадающего списка.
 *
 * Исходники шаблонов лежат в `src/editor_dll/templates/scripts/` (`{Type}.hpp.template` /
 * `{Type}.cpp.template`). `editor.csproj` копирует их в `templates/scripts/` рядом с `editor.exe`
 * при каждой сборке - `AssetsWidget.GetTemplatesDirectory` ищет их сначала там (чтобы создание
 * скриптов работало и в развёрнутом билде без исходников рядом), и только если этой папки нет -
 * по пути внутрь репозитория разработчика как fallback.
 * - `{ClassName}` - имя класса, введённое пользователем.
 * - `{NamespaceOpenHpp}` / `{NamespaceCloseHpp}` и `{NamespaceOpenCpp}` / `{NamespaceCloseCpp}` -
 *   обёртка C++ namespace с форматированием под `.hpp` и `.cpp`; для пустого namespace это
 *   пустые строки. `{NamespaceIndent}` добавляет отступ внутри namespace в `.hpp`.
 * - `{BaseClass}` - базовый класс (например, `zzz::script::Script`).
 * - `{IncludePath}` - путь подключения базового класса.
 * - `{Date}` - текущая дата генерации.
 *
 * Создаётся обязательная пара `.hpp` + `.cpp` (`.cpp` может быть почти пустым - просто инклюд) и
 * файл метаданных `.meta` (см. §5) с новым GUID.
 *
 * После создания скрипт в дереве можно переместить или удалить. Переименование C++ класса через
 * дерево отключено: имя класса задаётся только в диалоге создания, чтобы не расходились имя файла,
 * содержимое `.meta`, namespace и сгенерированный код.
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
 * Имя класса/файла читаемо в Git, но может меняться при перемещении или будущих миграциях. GUID не меняется никогда и
 * однозначно идентифицирует ассет независимо от пути и имени.
 *
 * - **Текущее состояние (сцены не реализованы):** GUID хранится только в `.meta`-файле и
 *   используется редактором для устойчивой идентификации скриптов. В файлах конфигурации,
 *   которые создаются в будущем (сцены, префабы), ссылки на скрипты будут храниться как GUID.
 *   Это означает, что изменение имени/пути скрипта в будущей миграции не потребует правки ни одного файла сцены.
 * - **Будущее:** Когда система сцен появится, файлы сцен (`.zzz`) будут хранить GUID скриптов
 *   вместо имён классов. Перед компиляцией финальной игры редактор/сборщик разворачивает все
 *   GUID-ссылки из сцен в конкретные имена классов через таблицу `GUID -> qualified_name` из `.meta`.
 *   Если GUID не резолвится (ассет удалён/потерян) — это ошибка сборки.
 * - **GUID - это слой редактора и сборщика, а не движка.** `ScriptRegistry` (движок) по-прежнему
 *   работает по полному имени класса (`Register<T>("Namespace::ClassName")`, см. §3) — ничего в `src/engine` ради
 *   GUID не меняется.
 *
 * ### 5.2. Формат `.meta`
 *
 * Каждый скрипт - это обязательная пара `Player.hpp` + `Player.cpp` и файл метаданных `Player.meta`
 * рядом (без `.hpp` в названии), формат TOML (как и все остальные текстовые файлы проекта - см.
 * §"Единый формат хранения: TOML" в README `Services/Project`; расширение остаётся `.meta`, а не
 * `.toml`, чтобы скан дерева ассетов продолжал скрывать эти файлы по расширению):
 *
 * ```toml
 * guid = "b5f36e84-18c7-4fd9-b22e-a567df489bc3"
 * class_name = "Player"
 * namespace = "Gameplay"
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
 * | Rename | Для скриптов отключён; имя класса задаётся только при создании |
 * | Move/Drag&Drop | Перемещает `.hpp`/`.cpp`/`.meta` вместе, GUID и `class_name`/`namespace` не меняются |
 *
 * Move - это **одна** команда в Undo/Redo (а не 2-3 последовательные), поэтому Ctrl+Z отменяет
 * перемещение всей триады сразу, а не файл за файлом.
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
 * Если два разных файла на диске несут один и тот же GUID (например, если пользователь вручную скопировал папку со скриптом через
 * Проводник Windows вместе с `.meta`-файлом и вставил её в другую часть проекта), это **тихая** порча данных: ссылки на объектах могут массово указывать не туда, и
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
 * ### Отслеживание изменений
 * Триггер - точечный `FileSystemWatcher` на `Assets/` ([`AssetsViewModel`](../src/tools/editor/ViewModels/AssetsViewModel.cs)),
 * а не проверка времени изменения по возврату фокуса:
 * - **Create/Delete/Rename** `.hpp`/`.cpp` - `RefreshTree()` (перегенерирует `RegisterAllScripts.cpp`,
 *   см. §3), затем сразу компиляция, если окно редактора активно.
 * - **Правка содержимого** (`Changed`) - debounce 500мс через `DispatcherTimer`
 *   (`ScheduleCompileDebounced`): IDE может вызвать `Changed` несколько раз подряд при одном
 *   сохранении, ждём тишины перед запуском CMake.
 * - **Окно не в фокусе** в момент любого из событий выше - компиляция не запускается сразу.
 *   [`ScriptRebuildCoordinator`](../src/tools/editor/Services/ScriptRebuildCoordinator.cs) запоминает
 *   один отложенный флаг (`projectRoot`) и воспроизводит его **один раз** при следующем
 *   `Window.Activated` - без сравнения времени файлов на этом этапе, просто "была правка, пока
 *   отвлеклись - дособери сейчас".
 * - **Открытие проекта** всегда форсирует одну сборку независимо от времени файлов
 *   (`AssetsViewModel.OnProjectOpened`).
 * - Если запрос на компиляцию приходит, пока предыдущая сборка ещё выполняется (`_isCompiling`),
 *   он не запускает параллельную сборку, а встаёт в очередь на один слот и переигрывается сразу
 *   после завершения текущей (`MainWindow.CompileScriptsAsync`).
 *
 * `CheckAndCompileScriptsAsync` содержит также эвристику сравнения времени записи
 * `Assets/*.hpp/.cpp` с `.editor/bin/scripts.dll` (включая отдельный случай для
 * `RegisterAllScripts.cpp`) для режима `forceRebuild: false` - на момент написания все реальные
 * вызовы в кодовой базе передают `forceRebuild: true`, так что эта ветка кода существует, но сейчас
 * не используется ни одним вызывающим местом.
 *
 * ### Сборка
 * `cmake -B .editor/build -S .editor` (только если `CMakeLists.txt` изменился или ещё нет
 * `CMakeCache.txt`) + `cmake --build .editor/build --config Debug` (или `Release`). Результат
 * складывается в `.editor/bin_build/scripts.dll` + `.pdb` (staging, ещё не то, что подхватит движок).
 *
 * ### Приём собранной DLL
 * Существующая `.editor/bin/scripts.dll` (если есть) переименовывается в `scripts_old_<tick>.dll`
 * (бэкап), затем свежесобранная DLL/PDB из `bin_build` перемещается в `.editor/bin/`. При ошибке на
 * этом шаге старая DLL возвращается из бэкапа и движок перезагружается ей, после чего исключение
 * пробрасывается дальше. Старые `scripts_old_*.dll/.pdb` подчищаются в фоне перед следующей сборкой.
 *
 * ### Фаза перезагрузки в движке (`EditorEngine::ReloadScripts`, C++)
 * Реализация проще, чем можно было бы ожидать - никакой сериализации состояния объектов на сцене
 * нет:
 * 1. **`UnloadScripts()`:** `ScriptRegistry::Clear()` очищает три реестра фабрик и список
 *    `s_ActiveInstances` - но **не** уничтожает и не сохраняет состояние самих объектов скриптов,
 *    уже созданных на сцене; `FreeLibrary` на старую DLL, удаление предыдущего временного файла.
 * 2. **Копия во временный файл:** `.editor/bin/scripts.dll` -> `scripts_temp_<GetTickCount()>.dll` с
 *    retry-циклом (до 50 попыток по 100мс) - обходит кратковременную блокировку файла антивирусом
 *    сразу после сборки MSBuild (Sharing Violation).
 * 3. **Загрузка:** `LoadLibraryA` на временную копию, `GetProcAddress("RegisterAllScripts")`, вызов -
 *    заново регистрирует все фабрики.
 *
 * **Чего в реализации нет, хотя выглядит как заготовка под это:**
 * - Нет сериализации/восстановления состояния скриптов - объекты на сцене после Reload не
 *   пересоздаются автоматически и не получают назад старые значения полей.
 * - `ScriptRegistry::RegisterInstance/UnregisterInstance/GetActiveInstances` (только в `Z_EDITOR`)
 *   существуют и заполняются (см. ниже), но ни один потребитель их не считывает для
 *   сохранения/восстановления - задел на будущее, а не работающая фича.
 * - Нет явной инвалидации `weak_ptr`-ссылок сверх того, что происходит естественно при разрушении
 *   `shared_ptr`.
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
 *
 * ---
 *
 * ## 7. Resource model note
 *
 * Scripts are asset resources under `Assets/`, but the general asset identity model is
 * documented separately in [`resources.md`](resources.md).
 *
 * Key rules:
 *
 * - Script references in project/game configuration should use `.meta` GUIDs, not paths.
 * - Folders are containers only: they do not have GUIDs and do not have an asset type.
 * - `AssetResourceType.Script` is the asset classification for script files, not for
 *   folders named `Scripts`.
 * - When a script is selected from settings, the editor should resolve its GUID and
 *   highlight the corresponding file node in the Assets tree.
 * - The editor-side script GUID/class/path cache is owned by `ScriptAssetIndexService`,
 *   not by `AssetsViewModel`; filesystem updates come from `ProjectFileWatcherService`.
 *
 * ---
 *
 * ## 8. Архитектурные планы (Roadmap)
 *
 * Этот раздел фиксирует запланированные, но еще не реализованные механизмы системы скриптов. По мере реализации и разрастания кода этот документ будет разбит на части.
 *
 * - **Стабильная отписка от EventBus и инкапсуляция:**
 *   Отписка (включая автоматическую отписку по контексту и явную `Unsubscribe(shared_ptr)`) не должна ломать очередь вызовов или инвалидировать итераторы (отложенное удаление `isDead` с последующей зачисткой `remove_if`). События в `EventBus` должны быть инкапсулированы (private), а базовые классы скриптов должны использовать подход с единственным чисто виртуальным методом `InitScript()`, предоставляя пользователю защищенные методы подписки (`SubscribeToUpdate` и т.д.).
 *
 * - **Строгая модель жизненного цикла (Lifecycle):**
 *   Необходимо расширить события и закрепить четкий пайплайн: `Construct` (C++) → `Init(Context/EventBus)` → `OnStart` → `OnUpdate` → `OnLateUpdate` (критично для логики камер) → `OnStop` → `Destroy`.
 *
 * - **Механизм сериализации (State Restore):**
 *   В планах (пока не критично) реализация механизма сериализации/десериализации публичных/редакторских полей скриптов. Это необходимо для сохранения состояния объектов перед Hot-Reload и восстановления после загрузки новой DLL.
 *
 * - **Безопасный процесс выгрузки скриптов (Hot-Reload) и ABI-совместимость:**
 *   В финальной сборке игры DLL не используются (статическая линковка), поэтому проблемы с C++ ABI отпадают. Однако для редактора (`scripts.dll`) требуется строгая гарантия одинаковых настроек компилятора и CRT (например, `/MD`) между движком и DLL для безопасной передачи `std::shared_ptr`.
 *   Сам процесс выгрузки должен стать безопасным (нельзя делать `FreeLibrary` при живых объектах). Пайплайн:
 *   1. Опрос списка `ScriptRegistry::GetActiveInstances()`.
 *   2. Сериализация стейта (если уже поддержано).
 *   3. Уничтожение всех инстансов (сброс ссылок в `GameObject` и вызов деструкторов скриптов).
 *   4. Безопасный вызов `FreeLibrary`.
 *   5. Загрузка новой DLL, создание инстансов и восстановление их стейта.
 *
 * - **Проброс систем платформы (Platform APIs):**
 *   Требуется разработать механизм передачи интерфейсов платформы внутрь скриптов через инъекцию зависимостей (передача Контекста/Сервисов), а не через глобальные синглтоны.
 *
 * - **Поиск скриптов (Межскриптовое взаимодействие):**
 *   Необходим механизм разрешения зависимостей (например, `RequireScript<T>()` или поиск по сцене/объекту).
 *
 * - **Логирование из пользовательских скриптов:**
 *   Добавить поддержку логов из скриптов с явным разделением на каналы/категории: `Engine`, `Editor`, `Script`, `Build`.
 */
