# ZzzTest

## Начало работы

1. Скачать репозиторий из Git

## Сборка macOS проекта для отладки движка

### Требования

- macOS 12+
- Xcode 14+ (с установленными Command Line Tools)
- CMake 3.28+

### Шаги

1. (Опционально) Применить конфигурацию сборки через Config Switcher:

   ```sh
   ./config_switcher_switch
   ```

2. Запустить скрипт генерации Xcode-проекта. При первом запуске дать ему исполняемый статус:

   ```sh
   chmod +x GenerateXcodeProject.command
   ./GenerateXcodeProject.command
   ```

   Или двойным кликом из Finder (предварительно дав статус через `chmod +x`).

3. Открыть сгенерированный проект:

   ```sh
   open projects/game_macos/Zzz.xcodeproj
   ```

4. В Xcode выбрать схему `game_macos` и собрать/запустить.

### Используемые фреймворки

Проект использует Cocoa, Metal, MetalKit и QuartzCore — все они входят в стандартный Xcode SDK и дополнительной установки не требуют.

## Инструменты

### Build Configurator

> Только для Windows

> Инструмент сгенерирован ИИ ввиду малозначительности и редкости использования — на доскональное качество реализации не претендует.

1. Открыть проект `tools/build_configurator/BuildConfigurator.csproj` в Visual Studio
2. Выбрать конфигурацию **Release**
3. Собрать проект: Build → Build Solution

После сборки исполняемый файл автоматически появится в папке `build_configs/` в корне репозитория.

### Config Switcher

> Инструмент сгенерирован ИИ ввиду малозначительности и редкости использования — на доскональное качество реализации не претендует.

Консольный инструмент для применения конфигурации сборки (Windows/Linux/Android).

Сборка (однократно):

- **Windows:** `automation\build_config_switcher.bat`
- **Linux/Android:** `automation/build_config_switcher.sh`

После сборки исполняемый файл автоматически появится в корне репозитория.

Запуск из корня репозитория:

```
./build_configurator_switch
```

Инструмент применяет выбранную конфигурацию в `build_configs/current.cmake` и опционально удаляет папку `build/` и перезапускает CMake.
