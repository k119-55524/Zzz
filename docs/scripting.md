# Система скриптования (Scripting System)

Документ описывает архитектуру C++-скриптования в ZzzEngine.

---

## 1. Общие принципы

- **Язык разработки:** C++ (`.hpp` + `.cpp`).
- **Отсутствие виртуальных функций:** Все события (апдейты, старт, уничтожение) подписываются через `Event<>` в конструкторах классов, без vtable.
- **Владение:** Один `GameObject` может иметь несколько прикрепленных скриптов типа `script::Script` (владение через `std::shared_ptr`).
- **Синглтоны:** Глобальные скрипты (`script::Game`) и скрипты сцены (`script::Scene`) существуют в единственном экземпляре.
- **Интеграция в игру:** В финальном билде игры все скрипты компилируются статически напрямую в исполняемый файл без использования DLL.
- **Интеграция в редактор:** В редакторе скрипты собираются в отдельную `scripts.dll` для обеспечения функции горячей перезагрузки (Hot-Reload).

---

## 2. Иерархия классов скриптов

Все скрипты находятся в пространстве имён `zzz::script` и наследуются от трех основных базовых классов:

```
zzz::script
├── Script      - MonoBehaviour-аналог (навешивается на GameObject)
├── Game        - Глобальный синглтон (на всё время жизни игры)
└── Scene       - Синглтон сцены (на время загрузки сцены)
```

### 2.1. `zzz::script::Script`

Применяется для логики конкретных игровых объектов. Конструктор принимает указатель на объект-владелец `GameObject`:

```cpp
namespace zzz::script {
    class Script : public std::enable_shared_from_this<Script> {
    public:
        explicit Script(GameObject* owner);
        virtual ~Script();

        GameObject* GetOwner() const { return m_Owner; }
        virtual std::string_view GetScriptTypeName() const = 0;

    private:
        GameObject* m_Owner;
    };
}
```

#### Пример использования (PlayerController):
```cpp
// PlayerController.hpp
#pragma once
#include <script/Script.hpp>

class PlayerController : public zzz::script::Script {
public:
    explicit PlayerController(zzz::GameObject* owner);
private:
    void OnStart();
    void OnUpdate(float dt);
};

// PlayerController.cpp
#include "PlayerController.hpp"

PlayerController::PlayerController(zzz::GameObject* owner)
    : Script(owner)
{
    // Подписка на события жизненного цикла
    Events.OnStart.Subscribe(shared_from_this(), [this] { OnStart(); });
    Events.OnUpdate.Subscribe(shared_from_this(), [this](float dt) { OnUpdate(dt); });
}

void PlayerController::OnStart() {
    // логика старта
}

void PlayerController::OnUpdate(float dt) {
    // логика обновления
}
```

### 2.2. `zzz::script::Game`

Глобальный менеджер, существующий на протяжении всей жизни движка. Не привязан к `GameObject`:
```cpp
namespace zzz::script {
    class Game {
    public:
        Game();
        virtual ~Game();
        virtual std::string_view GetScriptTypeName() const = 0;
    };
}
```

### 2.3. `zzz::script::Scene`

Менеджер, существующий только пока сцена загружена в память:
```cpp
namespace zzz::script {
    class Scene {
    public:
        Scene();
        virtual ~Scene();
        virtual std::string_view GetScriptTypeName() const = 0;
    };
}
```

---

## 3. Автоматическая регистрация скриптов

Регистрация скриптов происходит явно через автоматически генерируемый файл `RegisterAllScripts.cpp`, что позволяет избежать проблем с порядком инициализации статических объектов и отбрасыванием неиспользуемого кода линкером.

Шаблонная функция регистрации `zzz::script::Register<T>(name)` определяет тип скрипта и помещает его фабрику в нужный реестр:

```cpp
template<typename T>
void Register(std::string_view name)
{
    if constexpr (std::is_base_of_v<script::Game, T>)
        GameRegistry::Add(name, []{ return new T(); });
    else if constexpr (std::is_base_of_v<script::Scene, T>)
        SceneRegistry::Add(name, []{ return new T(); });
    else if constexpr (std::is_base_of_v<script::Script, T>)
        ScriptRegistry::Add(name, []{ return new T(*owner_placeholder*); }); // Фабрика для GameObject
}
```

### Сгенерированный файл `RegisterAllScripts.cpp`:
```cpp
#include "PlayerController.hpp"
#include "GameManager.hpp"

void RegisterAllScripts()
{
    zzz::script::Register<PlayerController>("PlayerController");
    zzz::script::Register<GameManager>("GameManager");
}
```

---

## 4. Горячая перезагрузка (Hot-Reload) в Редакторе

### Отслеживание и Сборка:
1. Редактор на базе WPF следит за файлами в `Assets/` через `FileSystemWatcher`.
2. При изменении кода взводится флаг `IsDirty`.
3. При возвращении фокуса в редактор (`Window.Activated`) запускается фоновая сборка через CMake:
   `cmake --build <ProjectDir>/Build --config Development --target scripts`
4. После успешного завершения вызывается перезагрузка в движке.

### Фаза Перезагрузки (Reload):
Во избежание крашей из-за выгрузки машинного кода DLL, перезагрузка выполняется строго по шагам:

1. **Сериализация:** Движок проходит по списку всех живых инстансов скриптов (хранимых в `ScriptRegistry::GetActiveInstances()`) и сохраняет их переменные и связи.
2. **Очистка объектов:** Все скрипты на сцене уничтожаются. `weak_ptr` ссылки на них инвалидируются.
3. **Выгрузка DLL:** Вызывается `FreeLibrary` на старую `scripts.dll`.
4. **Загрузка новой DLL:** Вызывается `LoadLibrary` на новую `scripts.dll`, заново регистрируются все типы.
5. **Создание и восстановление:** Скрипты создаются на объектах заново из нового реестра фабрик, восстанавливается их сохраненное состояние.

#### Оптимизация для игры (Zero Overhead):
Глобальный список живых инстансов `s_ActiveInstances` используется **только в редакторе** и вырезается в игре:
```cpp
Script::Script(GameObject* owner) : m_Owner(owner) {
#if Z_EDITOR
    ScriptRegistry::RegisterInstance(this);
#endif
}
```

---

## 5. Создание файлов из шаблонов

Новые скрипты создаются через контекстное меню дерева ресурсов редактора (ПКМ → Add → Script / Game Script / Scene Script).

Редактор генерирует файлы `.hpp`/`.cpp` на основе шаблонов, расположенных в папке `editorDLL/templates/scripts/`:
- `{ClassName}` заменяется на имя класса, введенное пользователем.
- `{BaseClass}` заменяется на базовый класс (например, `zzz::script::Script`).
- `{IncludePath}` заменяется на путь подключения базового класса (`script/Script.hpp`).
- `{Date}` заменяется на текущую дату сборки.
