# Этап 9.1: Менеджер проектов Сборщика Ассетов: каркас проекта, валидация структуры, пресеты сборки и платформенные конфиги

**Статус:** ⏳ В процессе  
**Цель этапа:** Превратить Сборщик Ассетов в надежный Project Hub: генерация каркаса нового проекта ассетов (Scaffolding) в пустой папке, строгая валидация структуры существующих проектов с блокировкой поврежденных (Fail-Fast без затирания исходников), вынос платформенных настроек в каталог `build_settings/platforms/`, введение пресетов сборки (`build_settings/presets.json`) и выпадающих списков конфигураций в GUI/CLI сборщика.

---

## 1. Архитектура и структура файлов проекта ассетов

### 1.1 Структура каталога проекта ассетов:
```text
<AssetProjectRoot>/
├── project.json                      # Манифест проекта (company_name, app_name, active_preset)
├── build_settings/                   # Настройки сборки и матрица платформ
│   ├── presets.json                  # Наборы сборки (Build Presets)
│   └── platforms/                    # Конфигурации для платформ
│       ├── windows/
│       │   └── default.json          # Полный конфиг Windows (startView, platform, build)
│       ├── android/
│       │   └── default.json          # Полный конфиг Android
│       ├── ios/
│       │   └── default.json          # Полный конфиг iOS
│       ├── linux/
│       │   └── default.json          # Полный конфиг Linux
│       └── macos/
│           └── default.json          # Полный конфиг macOS
├── Assets/                           # Игровые ресурсы
│   ├── Scenes/
│   ├── Views/
│   └── Data/
└── Scripts/                          # Пользовательские C++ скрипты
```

### 1.2 Формат `project.json`:
Обязательно сохраняет все поля идентификации игры, необходимые для рантайма движка (`company_name` используется для путей сохранения данных в `Path::GetUserDataDirectory()`):
```json
{
  "company_name": "Zzz",
  "app_name": "ZzzGame",
  "app_version": "1.0.0",
  "name": "zzz_assets_test_000",
  "version": "1.0.0",
  "start_scene": "Assets/Scenes/MainScene.zs",
  "build_settings": {
    "presets_file": "build_settings/presets.json",
    "active_preset": "Default"
  }
}
```
*(Устаревший массив `platform_configs` полностью удаляется из `project.json`).*

### 1.3 Формат `build_settings/presets.json` (Наборы сборки):
Определяет матрицу сборки для проекта. Хранится в Git вместе с проектом:
```json
{
  "presets": [
    {
      "name": "Default",
      "description": "Базовый набор для разработки",
      "targets": [
        {
          "name": "game_win",
          "platform": "Windows",
          "config_file": "build_settings/platforms/windows/default.json",
          "is_enabled": true
        },
        {
          "name": "game_android",
          "platform": "Android",
          "config_file": "build_settings/platforms/android/default.json",
          "is_enabled": false
        }
      ]
    }
  ]
}
```

### 1.4 Формат платформенного конфига (`build_settings/platforms/<platform>/default.json`):
Сохраняет 1-в-1 всю структуру настроек платформы и окон (`build`, `platform`, `startView`), добавляя поле `"name"` для отображения в ComboBox GUI:
```json
{
  "name": "Windows Default",
  "build": {
    "icon": "Assets/Textures/app_icon.ico",
    "executableName": "ZzzGame",
    "visualStudioToolset": "v143"
  },
  "platform": {
    "windowClassName": "ZzzGameWindowClass",
    "child_views": [],
    "independent_views": []
  },
  "startView": {
    "title": "Zzz Game - Windows",
    "defaultSize": {
      "width": 1280,
      "height": 720
    },
    "windowMode": "Windowed",
    "resizable": true
  }
}
```

---

## 2. Разделение ответственности и подсистемы

### 2.1 Разграничение `BuildProfile` (Machine) и `BuildPreset` (Project):
- **`BuildProfile` (в `%LOCALAPPDATA%/.../session_config.json`):**
  - Содержит только machine-specific данные: пути на конкретном диске (`SourcePath`, `DestinationPath`) и имя активного пресета (`ActivePresetName`).
  - Коллекция `TargetProjects` удаляется из локального профиля разработчика, исключая рассинхронизацию.
- **`BuildPreset` (в `build_settings/presets.json` проекта):**
  - Содержит матрицу целевых проектов: имя таргета, платформа, путь к платформенному конфигу, флаг активности.

### 2.2 Создание каркаса нового проекта (Scaffolding):
- Вызывается через «+ Создать проект» в GUI или `--init` в CLI.
- Создается **только** в пустой папке.
- Генерирует валидный скелет: `project.json` (с `company_name`, `app_name`), `Assets/`, `Scripts/`, `build_settings/presets.json` и `build_settings/platforms/{windows,android,...}/default.json`.

### 2.3 Строгая валидация структуры существующего проекта (Fail-Fast):
- При выборе папки проверяются:
  1. `project.json` (наличие `company_name`, `app_name`, `start_scene`, `build_settings`);
  2. Наличие папки `build_settings/` и файла `presets.json`;
  3. Существование файлов, указанных в таргетах пресетов.
- **Поведение:** При обнаружении ошибок сборщик **не трогает и не перезаписывает файлы**, блокирует сборку и выводит понятный список ошибок.

### 2.4 Выпадающие списки конфигураций платформ (GUI):
- В таблице таргетов отображаются:
  - `None` (по умолчанию, либо если файл удален с диска).
  - Доступные JSON-файлы из `build_settings/platforms/<platform>/` с именами из поля `"name"`.
- Таргеты со значением `None` или отключенным чекбоксом не собираются.

### 2.5 Передача платформенного конфига в нативную DLL (`assets_builder_dll`):
- Сигнатура P/Invoke:
  `PackProjectNative(sourceDir, destinationDir, targetPlatform, platformConfigRelativePath)`.
- Логика поиска конфига через устаревший `platform_configs` в `PackagePacker.cpp` полностью удаляется.

---

## 3. План реализации по шагам

1. **Миграция тестового проекта `zzz_assets_test_000`:**
   - Вынос настроек платформ в `build_settings/platforms/{windows,android,...}/default.json`.
   - Создание `build_settings/presets.json` с набором "Default".
   - Обновление `project.json` (удаление `platform_configs`, сохранение `company_name: "Zzz"`, `app_name: "ZzzGame"`).
2. **Ядро сборщика (`assets_builder_lib`):**
   - Модели `BuildPreset`, `BuildPresetTarget`, `PlatformConfigSummary`.
   - `ProjectValidator` (строгая валидация `project.json` и файловой структуры).
   - `ProjectScaffolder` (создание чистого скелета проекта в пустой папке).
   - `BuildPresetManager` (работа с `build_settings/presets.json`).
3. **Нативная DLL (`assets_builder_dll`):**
   - Обновление `PackProjectNative` для приема относительного пути к платформенному конфигу.
   - Удаление старого `ResolvePlatformJson()` из `PackagePacker.cpp`.
4. **Интерфейс (`assets_builder_gui`):**
   - Верхняя панель: селектор проектов (Recent Projects) + кнопки создания/открытия.
   - Панель пресетов: селектор пресетов открытого проекта.
   - Таблица таргетов: ComboBox с платформенными конфигами и опцией `None`.
   - Очистка `BuildProfile` от дублирующего списка таргетов.
   - Окно ошибок валидации проекта.
5. **Верификация:**
   - Проверка создания каркаса в новой папке.
   - Проверка блокировки открытия повреждённого проекта.
   - Сквозная упаковка и запуск `game_win.exe`.
