# Инструкции по настройке окружения и сборке

## Сборка macOS и iOS проектов для отладки движка

### Требования

- macOS 12+
- Xcode 14+ (с установленными Command Line Tools)
- CMake 3.28+

### Шаги

1. (Опционально) Применить конфигурацию сборки через Config Switcher:

   ```sh
   ./build_configs/build_configurator_switch
   ```

2. Запустить скрипт генерации Xcode-проектов. При первом запуске дать ему исполняемый статус:

   ```sh
   chmod +x automation/GenerateXcodeProject.command
   ./automation/GenerateXcodeProject.command
   ```

   Или двойным кликом из Finder (предварительно дав статус через `chmod +x`).

   Скрипт генерирует оба проекта — macOS и iOS.

3. Открыть нужный проект:

   ```sh
   open src/projects/game_macos/Zzz.xcodeproj  # macOS
   open src/projects/game_ios/Zzz.xcodeproj    # iOS
   ```

4. В Xcode выбрать соответствующую схему (`game_macos` или `game_ios`) и собрать/запустить.

### Используемые фреймворки

| Платформа | Фреймворки |
|-----------|-----------|
| macOS | Cocoa, Metal, MetalKit, QuartzCore |
| iOS | UIKit, Metal, MetalKit, Foundation |

Все фреймворки входят в стандартный Xcode SDK и дополнительной установки не требуют.

## Сборка Linux проекта на Windows (Visual Studio)

Для сборки и отладки Linux-таргета на Windows используется WSL (Windows Subsystem for Linux) — стандартное решение, интегрированное в Visual Studio.

### Установка WSL

> Далее используется Ubuntu-24.04 как пример. Дистрибутив может быть другим — подставить нужный.

```powershell
wsl --install -d Ubuntu-24.04
```

После установки перезагрузить компьютер.

Проверка:

```powershell
wsl --version
wsl -l -v
```

### Перенос на другой диск (опционально)

```powershell
wsl --shutdown
wsl --manage Ubuntu-24.04 --move D:\Lunux_WSL\Ubuntu-24.04
wsl --set-default Ubuntu-24.04
```

Если `--move` завершился с ошибкой:

```powershell
wsl --unregister Ubuntu-24.04
wsl --import-in-place Ubuntu-24.04 D:\Lunux_WSL\Ubuntu-24.04\ext4.vhdx
wsl --set-default Ubuntu-24.04
```

### Запуск

```powershell
wsl -d Ubuntu-24.04
```

### Развёртывание окружения внутри WSL

После первого запуска Ubuntu выполнить следующее.

#### Обновление системы

```bash
sudo apt update
sudo apt upgrade -y
```

#### C++ тулчейн

```bash
sudo apt install -y build-essential clang cmake ninja-build gdb lldb git curl wget zip unzip tar pkg-config
```

Проверка:

```bash
gcc --version
g++ --version
clang++ --version
cmake --version
ninja --version
git --version
```

#### LunarG Vulkan SDK

Предварительно убедиться, что установлены зависимости для работы с apt-репозиториями:

```bash
sudo apt install -y lsb-release gnupg
```

```bash
wget -qO- https://packages.lunarg.com/lunarg-signing-key-pub.asc | sudo tee /etc/apt/trusted.gpg.d/lunarg.asc > /dev/null
sudo wget -qO /etc/apt/sources.list.d/lunarg-vulkan-noble.list http://packages.lunarg.com/vulkan/lunarg-vulkan-noble.list
sudo apt update
sudo apt install -y vulkan-sdk
```

Дополнительные пакеты:

```bash
sudo apt install -y mesa-utils vulkan-tools glslang-tools spirv-tools
```

#### Проверка

Vulkan:

```bash
vulkaninfo --summary
glslc --version
spirv-val --version
vulkaninfo | grep VK_LAYER_KHRONOS_validation
ls /usr/include/vulkan
ls /usr/share/vulkan/icd.d
glxinfo -B
vkcube
```

### Подключение в Visual Studio

В Visual Studio должен быть установлен компонент **Linux development with C++** (либо минимально — **C++ CMake tools for Linux**). Установить через **Visual Studio Installer → Modify → Individual components**.

1. Открыть **Tools → Options → Cross Platform → Connection Manager**
2. Нажать **Add** и выбрать тип подключения **Windows Subsystem for Linux**
3. Visual Studio автоматически обнаружит установленный WSL-дистрибутив

После этого Linux-таргет (`game_linux`) будет доступен для сборки и удалённой отладки прямо из Visual Studio.

## Assets Builder (WPF + C++ DLL)

`src/tools/assets_builder` — ключевой инструмент ресурсного пайплайна движка. Состоит из WPF GUI (`assets_builder_gui`), логической библиотеки (`assets_builder_lib`) и нативной C++ библиотеки упаковки (`assets_builder_dll`).

**Назначение:**
- Валидация и трансформация исходных форматов (меши, текстуры, описания сцен).
- Генерация метаданных (`.meta`) с фиксацией версий, хэшей и GUID.
- Упаковка ресурсов в компактные бинарные контейнеры движка: `package.dat` (таблица смещений и метаданные) и `data.dat` (сырые бинарные данные ресурсов).

**Сборка:**
- **Автоматически в CMake:** при сборке на Windows таргет `assets_builder_gui_build` автоматически собирает C# GUI после компиляции C++ библиотеки `assets_builder_dll`.
- **Через Visual Studio:** открыть решение **`src/tools/Tools.sln`** и собрать проект `assets_builder_gui`.

Скомпилированные исполняемые файлы и библиотеки помещаются в каталог:
`dist/<Config>/assets_builder_gui.exe` и `dist/<Config>/assets_builder_dll.dll`.

## RemoteLogViewer (WPF)

`src/tools/RemoteLogViewer` — отдельный C#/.NET (WPF) проект, предназначенный для перехвата и удобного просмотра сетевых логов игры в реальном времени.

**Сборка:**
Утилиту можно собрать двумя способами:
1. **Через Visual Studio:** Открыть общее решение **`src/tools/Tools.sln`** и собрать проект `RemoteLogViewer` (или пересобрать всё решение целиком). Сборка проекта и сопутствующих библиотек движка выполняется в централизованную папку `dist/<Config>/` в корне репозитория.
2. **Через CMake:** При сборке проекта в CMake (сборка `ALL`) на Windows проект компилируется автоматически.

После сборки в корне репозитория автоматически создаётся ярлык **`RemoteLogViewer.lnk`**, указывающий на исполняемый файл в папке `dist/Debug/` или `dist/Release/` в зависимости от активной конфигурации. NuGet-зависимости подтягиваются автоматически; .NET SDK при отсутствии на машине ставится автоматически скриптом CMake.

Дизайнер окон (`MainWindow.xaml` и т.п.) встроен в Visual Studio для WPF «из коробки» — дополнительных расширений ставить не нужно.

**Особенности:**
- Прослушивание логов с разных адресов/портов в отдельных вкладках.
- Цветовая индикация уровня важности (Info, Warning, Error, Critical).
- Раскрытие строк логов по клику для просмотра детальной информации (сообщение, файл, строка кода, функция).
- Копирование отформатированных записей со всеми метаданными в буфер обмена.
- Сохранение настроек сессий и автоматическое восстановление при следующем запуске.

### Пример использования (C++)

В коде движка или игры необходимо добавить сетевой бродкастер в глобальный логгер. Пример из `src/projects/game_win/main.cpp`:

```cpp
#include <core/core.h>

using namespace zzz::logger;

int APIENTRY wWinMain(...)
{
    // Подключаем отправку логов в RemoteLogViewer по сети на локалхост
    g_Logger.AddNetworkBroadcaster(c_LocalhostIPv4, c_DefaultLoggerPort);

    // Обычные сообщения (Info)
    DOut("[Windows OS]. Game started.");

    try
    {
        // ... инициализация и запуск ...
        DOutError("[Windows OS]. Game runtime error: {}.", res.error()); // Ошибка
    }
    catch (const std::exception& e)
    {
        DOutException("[Windows OS]. WinMain {}.", e.what()); // Исключение
    }

    return 0;
}
```

## Редактор и Hot Reloading (Статус разработки)

> **Статус:** Разработка C# WPF редактора (`src/tools/editor`) и связанного механизма изолированного SDK/горячей перезагрузки начата, но **временно отложена** ввиду большого объёма работ и низкого приоритета на текущем этапе. Основной фокус направлен на завершение базового пайплайна движка, GAPI и ресурсной системы.

**Планируемая архитектура SDK для редактора:**
Для компиляции скриптов игры и горячей перезагрузки планируется автоматическое формирование чистого SDK движка в директории `dist/<Config>/libs/zlibs/`:
- Скрипт фильтрации публичных заголовков формирует изолированный каталог `include/` без внутренних файлов реализации.
- Редактор при компиляции пользовательских скриптов передаёт путь к SDK через переменную `Z_EDITOR_PATH`:
  ```cmake
  # Подключение заголовков SDK
  target_include_directories(GameTarget PRIVATE "${Z_EDITOR_PATH}/libs/zlibs/include")
  # Указание пути к библиотекам SDK
  link_directories("${Z_EDITOR_PATH}/libs/zlibs/lib")
  # Линковка со скомпилированными модулями
  target_link_libraries(GameTarget PRIVATE engine_lib core_lib logger_lib)
  ```
Такой подход обеспечит переносимость игровых скриптов и сценариев без привязки к абсолютным путям репозитория.
