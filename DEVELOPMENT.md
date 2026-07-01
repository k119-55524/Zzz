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

## RemoteLogViewer (WPF)

`src/tools/RemoteLogViewer` — отдельный C#/.NET (WPF) проект, предназначенный для перехвата и удобного просмотра сетевых логов игры в реальном времени.

**Сборка:**
Утилиту можно собрать двумя способами:
1. **Через Visual Studio:** Открыть общее решение **`src/tools/Tools.sln`** и собрать проект `RemoteLogViewer` (или пересобрать всё решение целиком). Сборка проекта и библиотек движка выполняется в централизованную папку `bin/$(Configuration)/` в корне репозитория.
2. **Через CMake:** При сборке проекта в CMake (сборка `ALL`) на Windows проект компилируется автоматически.

После сборки в корне репозитория автоматически создаётся ярлык **`RemoteLogViewer.lnk`**, указывающий на исполняемый файл в папке `bin/Debug/` или `bin/Release/` в зависимости от конфигурации сборки. NuGet-зависимости подтягиваются автоматически; .NET SDK при отсутствии на машине ставится автоматически при конфигурации CMake.

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

## Интеграция с Редактором (Editor SDK и Hot Reloading)

Движок поддерживает автоматическое формирование SDK для редактора, который используется игровыми проектами для компиляции и горячей перезагрузки (Hot Reloading).

**Как это работает:**
При сборке основных модулей движка (`common_lib`, `logger_lib`, `engine_lib`) CMake автоматически копирует необходимые бинарные файлы (`.dll`, `.lib`) и публичные заголовочные файлы (`.h`) в директорию `bin/<Config>/libs/zlibs/`.
При копировании заголовков применяется скрипт `copy_headers.cmake`, который отсеивает все `.cpp` и прочие служебные файлы, оставляя папку `include` чистой и готовой к использованию (сохраняя оригинальную структуру папок `src`).

**Подключение в проектах игры:**
Чтобы игровые проекты не зависели от жестких путей к исходникам движка, редактор при запуске сборки игры передает свой текущий путь через переменную CMake:
```bash
cmake -S "Путь/К/Игре" -B "Путь/К/Игре/build" -DZ_EDITOR_PATH="<Текущий_Путь_К_Редактору>"
```

В `CMakeLists.txt` игры этот путь используется для подключения SDK:
```cmake
# Подключаем хидеры SDK
target_include_directories(GameTarget PRIVATE "${Z_EDITOR_PATH}/libs/zlibs/include")

# Указываем путь к .lib файлам SDK
link_directories("${Z_EDITOR_PATH}/libs/zlibs/lib")

# Линкуем игру с движком
target_link_libraries(GameTarget PRIVATE engine_lib common_lib logger_lib)
```

Такой подход гарантирует, что проект игры 100% переносим, а горячая перезагрузка работает стабильно как на этапе разработки движка, так и при использовании установленного дистрибутива.
