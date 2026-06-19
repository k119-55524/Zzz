# Инструкции по настройке окружения и сборке

## Сборка macOS и iOS проектов для отладки движка

### Требования

- macOS 12+
- Xcode 14+ (с установленными Command Line Tools)
- CMake 3.28+

### Шаги

1. (Опционально) Применить конфигурацию сборки через Config Switcher:

   ```sh
   ./config_switcher_switch
   ```

2. Запустить скрипт генерации Xcode-проектов. При первом запуске дать ему исполняемый статус:

   ```sh
   chmod +x GenerateXcodeProject.command
   ./GenerateXcodeProject.command
   ```

   Или двойным кликом из Finder (предварительно дав статус через `chmod +x`).

   Скрипт генерирует оба проекта — macOS и iOS.

3. Открыть нужный проект:

   ```sh
   open projects/game_macos/Zzz.xcodeproj  # macOS
   open projects/game_ios/Zzz.xcodeproj    # iOS
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

## RemoteLogViewer (WPF)

`tools/RemoteLogViewer` — отдельный C#/.NET (WPF) проект, не часть C++ сборки движка. Утилита предназначена для перехвата и удобного просмотра сетевых логов игры в реальном времени.

**Сборка:**
Собирается автоматически при сборке проекта в CMake (сборка `ALL`) на Windows. После успешной компиляции в корне репозитория появится ярлык `RemoteLogViewer.lnk`. NuGet-зависимости (если появятся) подтягиваются автоматически через `PackageReference`; сам .NET SDK при отсутствии на машине ставится автоматически при конфигурации CMake.

Дизайнер окон (`MainWindow.xaml` и т.п.) встроен в Visual Studio для WPF «из коробки» — дополнительных расширений ставить не нужно.

**Особенности:**
- Прослушивание логов с разных адресов/портов в отдельных вкладках.
- Цветовая индикация уровня важности (Info, Warning, Error, Critical).
- Раскрытие строк логов по клику для просмотра детальной информации (сообщение, файл, строка кода, функция).
- Копирование отформатированных записей со всеми метаданными в буфер обмена.
- Сохранение настроек сессий и автоматическое восстановление при следующем запуске.

### Пример использования (C++)

В коде движка или игры необходимо добавить сетевой бродкастер в глобальный логгер. Пример из `projects/game_win/main.cpp`:

```cpp
#include <common/common.h>

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
