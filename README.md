# ZzzTest

## Начало работы

1. Скачать репозиторий из Git

2. Подготовить рабочее окружение. Подробные инструкции по установке необходимых зависимостей для разных платформ (macOS, iOS, Linux/WSL) вынесены в отдельный файл: **[docs/DEVELOPMENT.md](docs/DEVELOPMENT.md)**.
3. Ознакомиться с архитектурой движка, ресурсами и подсистемой скриптов можно в файле: **[docs/ARCHITECTURE.md](docs/ARCHITECTURE.md)**.


## Тесты и бенчмарки

В проекте используются Google Test и Google Benchmark, расположенные в папке `src/qa/` (`src/qa/tests` и `src/qa/benchmark`).

Управление тем, какие модули тестов собираются, осуществляется через **CMake-дефайны** в **Build Configurator**:
1. Основные рубильники (`Z_ADD_PROJECT_TESTS_IN_BUILD` и `Z_ADD_PROJECT_BENCHMARKS_IN_BUILD`) управляют глобальным включением подпроектов.
2. Дефайны отдельных подсистем (например, `Z_TESTS_ENABLE_TEMPLATES`) используются в CMake-скриптах для точечного добавления исходников `.cpp` нужных модулей.

Чтобы включить тестов, добавьте нужные CMake-дефайны в активную конфигурацию через графический интерфейс конфигуратора и примените её с помощью консольного свитчера.

## Инструменты

Все инструменты разработки, включая конфигурационные утилиты и C++ проекты движка, объединены в общее решение **`src/tools/Tools.sln`** для удобной сборки и отладки (включая отладку C++ DLL из C# редактора).

Для быстрого доступа к собранным инструментам в корне репозитория расположены следующие ярлыки:

| Ярлык в корне | Описание | Путь к бинарнику | Как собирается |
| :--- | :--- | :--- | :--- |
| **`BuildConfigurator.lnk`** | Графический редактор профилей сборки (WPF) | `build_configs/BuildConfigurator.exe` | VS/MSBuild (`Tools.sln`), конфигурация **Release** (публикуется автоматически) |
| **`BuildConfiguratorSwitch.lnk`** | Консольный свитчер конфигураций (C++) | `build_configs/build_configurator_switch.exe` | Через скрипты сборки в `automation/` или CMake |
| **`RemoteLogViewer.lnk`** | Сетевой просмотрщик логов движка (WPF) | `bin/<Config>/RemoteLogViewer.exe` | Автоматически в CMake или через VS/MSBuild (`Tools.sln`) |

Сборка C# редактора (`editor.exe`), библиотек движка и сопутствующих файлов производится в общую централизованную директорию `bin/<Config>/` в корне репозитория. Сборочные утилиты (`BuildConfigurator.exe` и `build_configurator_switch.exe`) собираются в директорию `build_configs/` в корне.


### Build Configurator

> Только для Windows

Графическая утилита (WPF) для настройки профилей сборки.

1. Открыть решение `src/tools/Tools.sln` в Visual Studio
2. Собрать проект `BuildConfigurator` (или пересобрать всё решение) в конфигурации **Release**

После сборки исполняемый файл автоматически появится в папке `build_configs/` в корне репозитория (на него ссылается ярлык `BuildConfigurator.lnk` в корне).

### Config Switcher

Консольный инструмент для переключения активной конфигурации сборки.

Сборка (однократно):
- **Windows:** запустить `automation\build_config_switcher.bat`
- **Linux/macOS:** запустить `automation/build_config_switcher.sh`

После сборки исполняемый файл появится в папке `build_configs/`, а в корне репозитория появится ярлык **`BuildConfiguratorSwitch.lnk`**.

Запуск из корня репозитория:
- Через ярлык `BuildConfiguratorSwitch.lnk` или напрямую:
  ```sh
  ./build_configs/build_configurator_switch
  ```

Инструмент позволяет выбрать конфигурацию, сохраняя её в `build_configs/current.cmake`.

### Remote Log Viewer

Утилита для перехвата сетевых логов движка в реальном времени.

1. Открыть решение `src/tools/Tools.sln` в Visual Studio и собрать проект `RemoteLogViewer` (сборка идёт в централизованную папку `bin/<Config>/` в корне проекта).
2. Запустить утилиту можно через ярлык **`RemoteLogViewer.lnk`** в корне репозитория.

Подробности сборки и пример использования в коде см. в **[docs/DEVELOPMENT.md](docs/DEVELOPMENT.md#RemoteLogViewer-wpf)**.

