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
│       │   └── default_config.json   # Полный конфиг Windows (startView, platform, build)
│       ├── android/
│       │   └── default_config.json   # Полный конфиг Android
│       ├── ios/
│       │   └── default_config.json   # Полный конфиг iOS
│       ├── linux/
│       │   └── default_config.json   # Полный конфиг Linux
│       └── macos/
│           └── default_config.json   # Полный конфиг macOS
├── Assets/                           # Игровые ресурсы
│   ├── Scenes/
│   ├── Views/
│   └── Data/
└── Scripts/                          # Пользовательские C++ скрипты
```

### 1.2 Формат `project.json` (Базовый кроссплатформенный манифест):
Обязательно сохраняет поля идентификации игры, необходимые для рантайма движка (`company_name` и `app_name` используются для путей сохранения данных в `Path::GetUserDataDirectory()`). Устаревшие легаси-дубликаты (`name`, `version`, `app_version`) удалены:
```json
{
  "company_name": "Zzz",
  "app_name": "ZzzGame",
  "description": "Тестовый проект игровых ресурсов",
  "start_scene": "3cbf41ff-f608-47ca-b383-ea5698648aca",
  "game_scripts": [
    "a8f94d12-e5b1-4c92-bf38-71e4029410ad",
    "b73c891e-42f0-410a-9d66-88c9a3b01e74",
    "f4710b65-c9e8-4632-8419-3d027f918e2c"
  ],
  "scenes": [
    "3cbf41ff-f608-47ca-b383-ea5698648aca"
  ],
  "views": [
    "e4d3a2b1-1234-4567-89ab-cdef01234567"
  ],
  "build_settings": {
    "presets_file": "build_settings/presets.json",
    "active_preset": "Default"
  }
}
```
*(Устаревший массив `platform_configs` и поля `name`/`version` полностью удалены из `project.json`).*

### 1.3 Формат `build_settings/presets.json` (Наборы сборки):
Определяет матрицу сборки для проекта. Хранится в Git вместе с проектом. Включение таргета в сборку определяется назначением файла конфигурации (`config_file != null` / не None):
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
          "config_file": "build_settings/platforms/windows/default_config.json"
        },
        {
          "name": "game_android",
          "platform": "Android",
          "config_file": null
        }
      ]
    }
  ]
}
```

### 1.4 Формат платформенного конфига (`build_settings/platforms/<platform>/default_config.json`):
Хранит настройки конкретной платформы:
- **Версии сторов:** вынесены в блок `build` (`version`/`buildNumber` для Win/Linux, `versionName`/`versionCode` для Android, `bundleVersion`/`buildNumber` для Apple).
- **Секции дельты контента:** `add_scripts`, `remove_scripts`, `add_scenes`, `remove_scenes`, опциональный `start_scene`.
```json
{
  "name": "Windows Default",
  "build": {
    "executableName": "GameZzz.exe",
    "version": "1.0.0",
    "buildNumber": 1,
    "icon": "Assets/Icons/win_icon.ico",
    "visualStudioToolset": "v143"
  },
  "platform": {
    "windowClassName": "ZzzEngineWindowClass",
    "child_views": [],
    "independent_views": []
  },
  "add_scripts": [],
  "remove_scripts": [],
  "add_scenes": [],
  "remove_scenes": [],
  "startView": {
    "title": "GameZzz",
    "defaultSize": {
      "width": 1280,
      "height": 720
    },
    "windowMode": "Windowed",
    "resizable": true
  }
}
```

### 1.5 Принцип вычисления контента таргета (Base + Delta Overrides):
При упаковке пакета под конкретную платформу состав ресурсов формируется по формулам:
- $\text{scripts} = (\text{project.json["game\_scripts"]} \setminus \text{platform["remove\_scripts"]}) \cup \text{platform["add\_scripts"]}$
- $\text{scenes} = (\text{project.json["scenes"]} \setminus \text{platform["remove\_scenes"]}) \cup \text{platform["add\_scenes"]}$
- Стартовая сцена: `platform["start_scene"]` (если задана), иначе `project.json["start_scene"]`.

---

## 2. Разделение ответственности и подсистемы

### 2.1 Разграничение `BuildProfile` (Machine) и `BuildPreset` (Project):
- **`BuildProfile` (в `%LOCALAPPDATA%/.../session_config.json`):**
  - Содержит только machine-specific данные: пути на конкретном диске (`SourcePath`, `DestinationPath`) и имя активного пресета (`ActivePresetName`).
  - Коллекция `TargetProjects` удаляется из локального профиля разработчика, исключая рассинхронизацию.
- **`BuildPreset` (в `build_settings/presets.json` проекта):**
  - Содержит матрицу целевых проектов: имя таргета, платформа, путь к платформенному конфигу, флаг активности.

### 2.2 Создание каркаса нового проекта (Scaffolding) — [❌ Удалено при ревизии архитектуры, см. ниже]
- Класс `ProjectCreator` (`CreateProject`/`CanCreateInDirectory`) был реализован, но ни разу не был подключён:
  ни кнопки «+ Создать проект» в GUI, ни флага `--init` в CLI не существует (в `App.xaml.cs` обрабатывается
  только `--build`). Это расхождение плана с реальным кодом обнаружено и устранено 2026-09-06: `ProjectCreator.cs`
  удалён как мёртвый код (перенесён в `_to_delete/` в корне репозитория для ручного удаления). Функциональность
  сборщика при этом не изменилась — фича не была нигде достижима.
- Если Scaffolding понадобится в будущем — реализуется заново отдельным пунктом плана, с явной кнопкой/флагом.

### 2.3 Строгая валидация структуры существующего проекта (Fail-Fast):
- При выборе папки проверяются:
  1. `project.json` (наличие `company_name`, `app_name`, `start_scene`, `build_settings`);
  2. Наличие папки `build_settings/` и файла `presets.json`;
  3. Существование файлов, указанных в таргетах пресетов.
- **Поведение:** При обнаружении ошибок сборщик **не трогает и не перезаписывает файлы**, блокирует сборку и выводит понятный список ошибок.

### 2.4 Выпадающие списки конфигураций платформ (GUI):
- В таблице таргетов отображаются:
  - `None` (таргет не участвует в сборке, если конфиг не назначен или файл удален с диска).
  - Доступные JSON-файлы из `build_settings/platforms/<platform>/` с именами из поля `"name"`.
- Таргеты со значением `None` не собираются (чекбоксы включения полностью удалены, активация определяется выбором валидного конфига).
- Изменение конфигурации, платформы или пути назначения немедленно переводит профиль в состояние `IsDirty` с отображением `*` в заголовке окна и активацией кнопок «Сохранить» / «Отмена».
- При нажатии «Собрать» с несохранёнными изменениями запрашивается подтверждение сохранения настроек на диск. При отмене сборка прекращается (Fail-Safe), при согласии настройки атомарно сохраняются и сборка продолжается.

### 2.5 Передача платформенного конфига в нативную DLL (`assets_builder_dll`):
- Сигнатура P/Invoke:
  `PackProjectNative(sourceDir, destinationDir, targetPlatform, platformConfigRelativePath)`.
- Логика поиска конфига через устаревший `platform_configs` в `PackagePacker.cpp` полностью удалена. Нативная DLL парсит переданный относительный путь конфига, накладывает секции дельты (`add_scripts`, `remove_scripts`, `add_scenes`, `remove_scenes`, `start_scene`) и версионные метаданные сторов.

---

## 3. План реализации по шагам

1. **Миграция тестового проекта `zzz_assets_test_000`:** [✅ Выполнено]
   - Вынос настроек платформ в `build_settings/platforms/{windows,android,...}/default_config.json`.
   - Создание `build_settings/presets.json` с набором "Default".
   - Обновление `project.json` (удаление `platform_configs`, сохранение `company_name: "Zzz"`, `app_name: "ZzzGame"`).
2. **Ядро сборщика (`assets_builder_lib`):** [✅ Выполнено]
   - Модели `BuildPreset`, `BuildPresetTarget`, `PlatformConfigSummary`.
   - `ProjectValidator` (строгая валидация `project.json` и файловой структуры).
   - ~~`ProjectCreator` (создание структуры нового проекта в пустой папке)~~ — удалён 2026-09-06 как неиспользуемый мёртвый код (см. п.2.2).
   - `BuildPresetManager` (работа с `build_settings/presets.json`).
3. **Нативная DLL (`assets_builder_dll`):** [✅ Выполнено]
   - Обновление `PackProjectNative` для приема относительного пути к платформенному конфигу.
   - Слияние и дельта-переопределение ресурсов (`add_scripts`/`remove_scripts`, `add_scenes`/`remove_scenes`, `start_scene`, версии сторов).
4. **Интерфейс (`assets_builder_gui`):** [✅ Выполнено]
   - Верхняя панель: выбор проекта из истории и файлового диалога.
   - Панель пресетов: селектор пресетов открытого проекта (`presets.json`).
   - Таблица таргетов: 3 колонки (Конфигурация с опцией `None`, Папка назначения с кнопкой обзора, Платформа).
   - Очистка `BuildProfile` от дублирующего списка таргетов (сохранение только machine-specific путей).
   - Подтверждение сохранения при сборке с `IsDirty`.
   - Путь сборки по умолчанию (`.build`) — общий для всех профилей, хранится рядом с настройками сборщика: `%LOCALAPPDATA%/Zzz/AssetsBuilder/.build` (`SessionManager.GetDefaultBuildPath()`), а не внутри исходной папки проекта. При загрузке `session_config.json` профили со старым дефолтным путём (`<SourcePath>/.build`) автоматически мигрируют на новый (наравне с уже существовавшей миграцией `_build`/`builds`).
   - `PrepareBuildRoot` перед каждой сборкой полностью удаляет папку назначения и пересоздаёт её; в её корень кладётся маркер-файл `buildtime-data.txt`, содержащий единую метку времени сборки (ISO-8601 UTC).
   - `assets_config.json` целевых проектов (`UpdateTargetProjectsConfig`) дополнительно содержит поле `last_build_time` — та же метка времени, что записана в `buildtime-data.txt` (одна на весь запуск сборки, общая для всех таргетов), рядом с существующим `active_build_directories`.
4.1. **Ревизия архитектуры (2026-09-06):** [✅ Выполнено] Функциональность не менялась, устранены находки аудита:
   - Удалён мёртвый код: `ProjectCreator.cs` (см. п.2.2) и весь каталог `assets_builder_lib/Serializers/`
     (`IAssetSerializer`, `DefaultRawSerializer`, `SceneSerializer`, `ViewSerializer`, `ProjectManifestSerializer`,
     `BinarySerializationExtensions`) — старая C#-реализация бинарной сериализации, полностью вытесненная
     нативным `PackProjectNative`/`PackagePacker.cpp` и нигде более не вызывавшаяся. Оба перенесены в `_to_delete/`
     в корне репозитория (нет прав на удаление с этой стороны — удали вручную).
   - `AssetExtensions.cs` унифицирован под правило «C++ структуры и константы — только через DLL»: убраны
     C#-хардкод-копии расширений ресурсов (`.zav`/`.zcv`/`.ziv`/`.zp`/`.obj`/`.png`/`.zmat`/`.hlsl`) и фолбэк
     на локальный `HashSet` при сбое P/Invoke. Добавлены нативные экспорты `IsSupportedDataAssetExtension` и
     `IsSupportedViewExtension` в `BuilderApi.h/.cpp` (по аналогии с уже существовавшим `IsSupportedAssetExtension`),
     оба источник истины — `core/io/AssetFileExtensions.h` (`zzz::core::ext::*`). Расширения C++-скриптов
     (`.h`/`.hpp`/`.cpp`, `IsScriptExtension`) оставлены как есть на стороне C# — это понятие самого сборщика,
     в `core` не существует, поэтому под правило не подпадает.
   - Найдены, но пока не тронуты (см. `docs/ARCHITECTURE.md` §4.4, пп.16-19): мёртвый `AssetsBuilderEngine.BuildPackage()`
     (дублирует оркестрацию сборки, вручную повторённую в `MainWindowViewModel.StartBuild`/`BuildHeadless`),
     мёртвый `GetVersion()`, неиспользуемые и частично несогласованные с реальными DLL-экспортами P/Invoke-объявления
     `NativeMethods.GetAssetType*`/`GetDataPackage*`, и разбиение `PackagePacker.cpp` (1250 строк, платформенный
     конфиг-парсинг + сцена-парсинг + сериализация всех типов ресурсов в одном файле).
5. **Тестирование и верификация (TODO):** [⏳ В процессе]
   - ~~Проверка создания каркаса в новой пустой папке (`ProjectCreator`)~~ — снято, фича удалена как мёртвый код (см. п.2.2).
   - Проверка строгой блокировки открытия повреждённого проекта (`ProjectValidator`).
   - **Тестирование сборщика с различными настройками:**
     - Сборка с отключенным таргетом (`None`) — проверка пропуска таргета и сборки только активных.
     - Сборка с изменённой целевой платформой (например, Linux / Android конфиг) — проверка генерации пакета и версий.
     - Сборка с дельтами скриптов/сцен (`add_*` / `remove_*`) — проверка корректности состава `ProjectManifestData` и `PrimaryViewData` в собранном `package.dat`.
     - Проверка отмены сборки при несохранённых изменениях и успешного сохранения при согласии.
   - Сквозная упаковка и запуск `game_win.exe`.

