# ZzzTest

## Начало работы

1. Скачать репозиторий из Git

2. Подготовить рабочее окружение. Подробные инструкции по установке необходимых зависимостей для разных платформ (macOS, iOS, Linux/WSL) вынесены в отдельный файл: **[DEVELOPMENT.md](DEVELOPMENT.md)**.

## Тесты и бенчмарки

В проекте используются Google Test и Google Benchmark, расположенные в папке `qa/` (`qa/tests` и `qa/benchmarks`).

Управление тем, какие модули тестов собираются, осуществляется через **CMake-дефайны** в **Build Configurator**:
1. Основные рубильники (например, `Z_BUILD_TESTS` / `Z_ADD_PROJECT_TESTS_IN_BUILD` и `Z_BUILD_BENCHMARKS` / `Z_ADD_PROJECT_BENCHMARKS_IN_BUILD`) управляют глобальным включением подпроектов.
2. Дефайны отдельных подсистем (например, `Z_TESTS_ENABLE_TEMPLATES`) используются в CMake-скриптах для точечного добавления исходников `.cpp` нужных модулей.

Чтобы включить тесты, добавьте нужные CMake-дефайны в активную конфигурацию через графический интерфейс конфигуратора и примените её с помощью консольного свитчера..

## Инструменты

### Build Configurator

> Только для Windows

1. Открыть проект `tools/build_configurator/BuildConfigurator.csproj` в Visual Studio
2. Выбрать конфигурацию **Release**
3. Собрать проект: Build → Build Solution

После сборки исполняемый файл автоматически появится в папке `корень репозитория/build_configs/`.

### Config Switcher

Консольный инструмент для применения конфигурации сборки (Windows/Linux/Android).

Сборка (однократно):

- **Windows:** `automation\build_config_switcher.bat`
- **Linux/Android:** `automation/build_config_switcher.sh`

После сборки исполняемый файл автоматически появится в корне репозитория.

Запуск из корня репозитория:

```
./build_configurator_switch
```

Инструмент позволяет выбрать конфигурацию(сохраняя её `build_configs/current.cmake`).
