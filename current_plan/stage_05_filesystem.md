# Этап 05: Кроссплатформенная файловая система (`FileSystem`, `eFileLocation`, адаптеры платформ)

## 1. Контекст и цели этапа
- **Номер пункта:** **Пункт 5** (Завершающий этап **Уровня 1: Математический и файловый фундамент**).
- **Цель:** Создать единую, строго типизированную и высокопроизводительную подсистему файловой системы `FileSystem`, которая полностью скрывает от движка платформенные особенности хранения данных (чтение из `.apk` через `AAssetManager` на Android, бандл `NSBundle` на iOS, физический диск на Windows/Linux/macOS) и изолирует класс `Path` внутри подсистемы IO.
- **Статус:** Выполнено по генплану. Оптимизация переиспользования файлового handle отдельно в TODO 8; чтение заголовка/TOC без payload уже реализовано. Новые прогоны при ревизии 2026-09-10 не выполнялись.
- **Зависимости:** `src/core/io/Path.h`, `src/core/utils/NativeAppData.h`, `src/core/utils/Ensure.h`, `src/core/enums/eEnumToString.h`, `src/core/serialize/Serializer.h`, `src/engine/package/PackageManager.h`, `src/engine/package/UserSettingsManager.h`.

---

## 2. Архитектурные требования и проектирование

### 2.1. Политика размещения данных на платформах (Desktop vs Mobile)

> [!IMPORTANT]
> **Разделение дискового пространства по типам платформ:**
> 1. **Десктопы (Windows / Linux / macOS):**
>    - **Тяжелые данные (Пакеты ассетов `package.dat`, `data.dat`, загружаемые паки в `assets/paks/`):** всегда располагаются **в каталоге установки игры рядом с `.exe` (`eFileLocation::App`)**. Это предотвращает переполнение системного диска `C:\` (`%LOCALAPPDATA%`) и сохраняет данные на том диске, который выбрал пользователь (например, `D:\SteamLibrary\...`).
>    - **Легковесные данные (Конфиги `user.dat`, сейвы `Saves/`, логи `Logs/`, шейдерный кэш `Cache/`):** направляются в профиль пользователя (`%LOCALAPPDATA%` на Windows, `~/.config` на Linux).
> 2. **Мобильные платформы (Android / iOS):**
>    - **Встроенные ассеты:** читаются напрямую из закрытого пакета приложения (`.apk` через NDK `AAssetManager` на Android, `.app` ресурсы через `NSBundle` на iOS).
>    - **Пользовательские данные и скачиваемый кэш:** направляются во внутреннюю песочницу приложения (`internalDataPath` на Android, `NSApplicationSupportDirectory` на iOS).

---

### 2.2. Перечисления `eFileLocation` и `eAssetDirectoryKind::Paks`

#### 1. `eFileLocation` (`src/core/enums/eFileLocation.h`):
```cpp
namespace zzz::core
{
    /**
     * @enum eFileLocation
     * @brief Логическая область размещения файлов в приложении (1 байт).
     */
    enum class eFileLocation : zU8
    {
        App = 0,    ///< Ресурсы приложения / папка установки игры (Read-Only на Android/iOS, Read/Write в папке .exe на ПК)
        User,       ///< Пользовательские данные / конфиги (Read-Write: LOCALAPPDATA / internalDataPath)
        Saves,      ///< Папка сохранений игры (Read-Write)
        Cache,      ///< Папка временного кэша (Read-Write)
        Logs        ///< Папка лог-файлов (Read-Write)
    };
}
```

#### 2. Добавление `Paks` в `eAssetDirectoryKind` (`src/core/enums/eAssetDirectoryKind.h`):
```cpp
enum class eAssetDirectoryKind : zU32
{
    Textures = 1,
    Video    = 2,
    Audio    = 3,
    Fonts    = 4,
    Paks     = 5    ///< Каталог пакетов ассетов (assets/paks)
};
```
Константа в `PackageConstants.h`: `inline constexpr std::string_view c_PaksDirectoryRelativePath = "assets/paks";`.

> [!NOTE]
> **Принцип нумерации перечислений каталогов (Sentinel Value 0):**
> Как в `eUserDirectoryKind` (`Cache = 1, Saves = 2, Logs = 3`), так и в `eAssetDirectoryKind` (`Textures = 1, ..., Paks = 5`), нумерация строго начинается с **`1`**.
> Значение **`0`** намеренно зарезервировано как неинициализированное/невалидное состояние (sentinel value). При случайной передаче неинициализированной переменной `eAssetDirectoryKind{}` метод `Path::GetDirectory()` гарантированно попадает в ветку `default: THROW_RUNTIME(...)`, предотвращая тихие логические ошибки.

---

### 2.3. Архитектура `FileSystem` (Правило 21: Compile-Time Type Aliases, Zero `#ifdef`)

Вместо динамического полиморфизма с виртуальными таблицами (`vtable`) и лишними аллокациями памяти, подсистема `FileSystem` построена по каноничному стандарту ZzzEngine (`Input`, `MainLoop`):

1. **`FileSystemBase` (`src/core/io/FileSystemBase.h` / `.cpp`):**
   - Инкапсулирует `m_Path` и `m_NativeData`.
   - Содержит всю общую дисковую логику для `User`, `Saves`, `Cache`, `Logs` (на всех ОС), а также стандартную дисковую реализацию `App` для десктопа.
   - **Строгий YAGNI-интерфейс (ровно 4 метода):**
     1. `FileExists(location, relativePath)` — проверка наличия файла;
     2. `ReadBytes(location, relativePath, offset, size)` — точечное чтение диапазона байт из пакетов ассетов;
     3. `ReadAllBytes(location, relativePath)` — полное чтение файла (манифест, конфиги);
     4. `WriteAllBytes(location, relativePath, bytes)` — атомарная запись буфера байт (сохранения, конфиги).
   - Никаких неиспользуемых строковых обёрток (`ReadAllText`, `WriteAllText`, `AppendText`, `GetFileSize`), никаких `DiskXxx` / `protected static`, а также удалены геттеры `GetPath()` и `GetNativeData()` (строгая изоляция `Path` внутри подсистемы IO и соблюдение YAGNI).
   - **Инкапсуляция `InitializeUserData`:** метод перенесён в `private`, чтобы исключить повторную инициализацию или подмену путей в рантайме. Вызов разрешён строго ядру `zzz::engine::Engine` через `friend class`. Никаких сторонних классов или тестовых фикстур в друзьях нет.
   - **Гарантия контракта инициализации:** порядок инициализации ядра в `Engine::Engine` гарантирует, что `InitializeUserData()` вызывается до создания любых сервисов, работающих с данными пользователя (`UserSettingsManager`). Инвариант инициализации контролируется каноничным `ensure(!m_Path->GetUserDataDirectory().empty(), ...)` по Правилу 25 — нулевые накладные расходы (zero overhead) в Release и немедленный ассерт разработчику в Debug при нарушении порядка вызовов.
   - **Ноль `#ifdef` в реализации.**

2. **`FileSystemDesktop` (`src/core/io/platforms/FileSystemDesktop.h` / `.cpp`):**
   - Консолидированная реализация для настольных платформ (Windows, Linux, macOS).
   - Реализует чтение/запись `eFileLocation::App` через физический диск в каталоге исполняемого файла (`m_Path->GetExecutableDirectory() / relativePath`).
   - Исключает дублирование идентичного кода между Windows, Linux и macOS, обеспечивает единую компиляцию и тестирование на ПК.

3. **`FileSystemAndroid` (`src/core/io/platforms/FileSystemAndroid.h` / `.cpp`):**
   - Чтение `App` напрямую из `.apk` через NDK `AAssetManager`.
   - Приватный хелпер `TryOpenAsset` инкапсулирует валидацию NativeAppData и вызов `AAssetManager_open`.
   - Оптимизированный однопроходный метод `ReadAllBytes`: ассет открывается ровно один раз, читается в буфер и закрывается без повторного вызова `GetFileSize`.
   - Заголовок `<android/asset_manager.h>` строго изолирован в этом файле.
   - Запись в `App` запрещена (`UNEXPECTED`).

4. **`FileSystemiOS` (`src/core/io/platforms/FileSystemiOS.h` / `.cpp`):**
   - Заготовка под Apple `NSBundle` (полная реализация запланирована на шаге КП-5).

5. **Центральный заголовок `FileSystem.h` (`src/core/io/FileSystem.h`):**
   - Единственное место с блоком `#if Z_...`, экспортирующее конкретный платформенный класс через compile-time type alias:
     ```cpp
     #if Z_WINDOWS || Z_LINUX || Z_MACOS
     #include "platforms/FileSystemDesktop.h"
     namespace zzz::core { using FileSystem = FileSystemDesktop; }
     #elif Z_ANDROID
     #include "platforms/FileSystemAndroid.h"
     namespace zzz::core { using FileSystem = FileSystemAndroid; }
     #elif Z_IOS
     #include "platforms/FileSystemiOS.h"
     namespace zzz::core { using FileSystem = FileSystemiOS; }
     #else
     #error ">>>>> FileSystem: Unsupported platform."
     #endif
     ```

---

### 2.4. Архитектурный контракт порядка инициализации путей (Правило 25)

В архитектуре ZzzEngine действует принцип строгой контрактной модели (Design by Contract) в сочетании с Zero-Overhead в релизных сборках:

1. **Гарантия контракта жизненным циклом старта ядра (`Engine::Engine`):**
   - **Шаг 1:** `m_FileSystem = safe_make_shared<FileSystem>(nativeData);` — доступна область `eFileLocation::App` (каталог исполняемого файла на ПК / APK на Android). Каталог `m_UserDataDirectory` ещё пуст, так как имя игры неизвестно до чтения пакета.
   - **Шаг 2:** `m_PackageManager = safe_make_shared<PackageManager>(m_FileSystem);` — читает манифест `package.dat` строго из `eFileLocation::App`.
   - **Шаг 3:** `m_FileSystem->InitializeUserData(companyName, appName);` — строго инициализирует пути `User`, `Saves`, `Cache`, `Logs`. При возникновении ошибки движок немедленно аварийно завершает старт через `THROW_RUNTIME`.
   - **Шаг 4:** Создаются сервисы, работающие с пользовательскими путями (`UserSettingsManager` и др.).
   
   Благодаря этой строгой последовательности код рантайма движка и игры физически **не может** обратиться к неинициализированным путям пользователя.

2. **Отсутствие защитных рантайм-ветвлений в Release:**
   - Защитные рантайм-проверки вида `if (m_Path->GetUserDataDirectory().empty()) return UNEXPECTED(...)` в методах резолва путей **запрещены как избыточные**, поскольку они тратят процессорные такты на проверку инварианта, на 100% гарантированного архитектурой.
   - Контроль соблюдения контракта возложен на каноничный макрос `ensure(!m_Path->GetUserDataDirectory().empty(), ...)` по Правилу 25:
     - **В Debug/Dev:** немедленно ловит нарушение контракта разработчиком (например, попытку чтения сохранения в обход порядка инициализации или в новом юнит-тесте) и выдает стек-трейс с точным местом (`source_location`).
     - **В Release:** компилируется в `(void)0` (полный zero-overhead no-op).

---

### 2.5. Модификация `Path.h` / `Path.cpp` (`src/core/io/Path.h`)

Класс `Path` обновлен для полной инкапсуляции и поддержки `FileSystem`:
1. `friend class FileSystemBase;` — эксклюзивный доступ, метод `InitializeUserData` остаётся строго `private`.
2. Полный список открытых геттеров:
   - `[[nodiscard]] const std::filesystem::path& GetExecutableDirectory() const noexcept { return m_ExecutableDirectory; }`
   - `[[nodiscard]] const std::filesystem::path& GetUserDataDirectory() const noexcept { return m_UserDataDirectory; }`
   - `[[nodiscard]] std::filesystem::path GetPackageDatPath() const { return m_ExecutableDirectory / c_GamePackageRelativePath; }`
   - `[[nodiscard]] std::filesystem::path GetDataDatPath() const { return m_ExecutableDirectory / c_DataPackageRelativePath; }`
   - `[[nodiscard]] std::filesystem::path GetUserDatPath() const { return m_UserDataDirectory / c_ConfigFileName; }`
   - `[[nodiscard]] std::filesystem::path GetDirectory(eUserDirectoryKind kind) const;`
   - `[[nodiscard]] std::filesystem::path GetDirectory(eAssetDirectoryKind kind) const;`
3. В `GetDirectory(eAssetDirectoryKind::Paks)` возвращается `m_ExecutableDirectory / c_PaksDirectoryRelativePath`.

---

### 2.6. Интеграция в `Engine`, `PackageManager` и `UserSettingsManager`

```
                                [ Engine ]
                                    │
                  1. Создает FileSystem(nativeData)
                  2. Внутри FileSystem создается Path
                  3. Вызывает InitializeUserData(...)
                                    │
            ┌───────────────────────┴───────────────────────┐
            ▼                                               ▼
[ PackageManager(m_FileSystem) ]            [ UserSettingsManager(m_FileSystem) ]
- Читает "assets/package.dat"               - Читает/пишет "user.dat"
  через ReadBytes(App, ..., offset, size)     через ReadAllBytes/WriteAllBytes(User, ...)
```

1. **`Engine` (`src/engine/Engine.h` / `Engine.cpp`):**
   - Поле: `std::shared_ptr<FileSystem> m_FileSystem;` (заменяет прямое хранение `m_Path`).
   - Инициализация и гарантия контракта путей:
     ```cpp
     m_FileSystem = safe_make_shared<FileSystem>(nativeData);
     m_PackageManager = safe_make_shared<PackageManager>(m_FileSystem);
     
     if (auto res = m_FileSystem->InitializeUserData(m_PackageManager->GetCompanyName(), m_PackageManager->GetAppName()); !res)
         THROW_RUNTIME("Не удалось инициализировать каталог пользовательских данных: {}", res.error());
     
     m_UserSettingsManager = safe_make_shared<UserSettingsManager>(m_FileSystem);
     ```

2. **`PackageManager` (`src/engine/package/PackageManager.h` / `.cpp`):**
   - Конструктор принимает `std::shared_ptr<FileSystem> fileSystem`.
   - Заголовок и манифест пакета читаются при старте в `Initialize()`.
   - `LoadPackageData<T>` читает **только запрашиваемый фрагмент байт** через `m_FileSystem->ReadBytes(eFileLocation::App, c_GamePackageRelativePath, entry.GetOffset(), entry.GetSize())` без перечитывания и аллокации всего `package.dat`.

3. **`UserSettingsManager` (`src/engine/package/UserSettingsManager.h` / `.cpp`):**
   - Конструктор принимает `std::shared_ptr<FileSystem> fileSystem`.
   - Чтение и сохранение настроек выполняется через `m_FileSystem->ReadAllBytes(eFileLocation::User, c_ConfigFileName)` и `m_FileSystem->WriteAllBytes(...)`.

---

### 2.7. Поддержка `EnumToString` и сериализации

1. **`EnumToString` (`src/core/enums/eEnumToString.h`):**
   - Добавление `static constexpr std::string_view ToString(eFileLocation location)`.
2. **Сериализация:**
   - Поддержка `eFileLocation` через концепт `SerializablePrimitive`.

---

## 3. Файловая структура этапа

```
src/core/
├── constants/
│   └── PackageConstants.h          # [MODIFY] Добавление c_PaksDirectoryRelativePath = "assets/paks"
├── enums/
│   ├── eAssetDirectoryKind.h       # [MODIFY] Добавление Paks = 0
│   ├── eFileLocation.h             # [NEW] Перечисление логических областей (App, User, Saves, Cache, Logs)
│   └── eEnumToString.h             # [MODIFY] Добавление ToString(eFileLocation)
├── io/
│   ├── FileSystemBase.h            # [NEW] Базовый класс (чистый C++23, 0 дефайнов, User/Saves/Cache/Logs)
│   ├── FileSystemBase.cpp          # [NEW] Реализация дисковых примитивов и управления путями
│   ├── FileSystem.h                # [NEW] Точка входа: блок #if Z_... и using FileSystem = FileSystem...;
│   ├── platforms/
│   │   ├── FileSystemDesktop.h     # [NEW] Консолидированная реализация десктопа (Windows, Linux, macOS)
│   │   ├── FileSystemDesktop.cpp   # [NEW] Чтение/запись App с диска через std::filesystem
│   │   ├── FileSystemAndroid.h     # [NEW] Чтение App из APK через AAssetManager (TryOpenAsset)
│   │   ├── FileSystemAndroid.cpp   # [NEW] Реализация NDK с однопроходным ReadAllBytes
│   │   ├── FileSystemiOS.h         # [NEW] Заготовка под NSBundle (КП-5)
│   │   └── FileSystemiOS.cpp       # [NEW] Заготовка под NSBundle (КП-5)
│   ├── Path.h                      # [MODIFY] friend class FileSystemBase/Desktop, геттеры GetExecutableDirectory, GetUserDataDirectory
│   └── Path.cpp                    # [MODIFY] Поддержка eAssetDirectoryKind::Paks
src/engine/
├── Engine.h                        # [MODIFY] Хранение std::shared_ptr<FileSystem> m_FileSystem
├── Engine.cpp                      # [MODIFY] Инициализация FileSystem и проброс в менеджеры
└── package/
    ├── PackageManager.h            # [MODIFY] Прием std::shared_ptr<FileSystem>
    ├── PackageManager.cpp          # [MODIFY] Ranged-чтение записей package.dat через FileSystem::ReadBytes
    ├── UserSettingsManager.h       # [MODIFY] Прием std::shared_ptr<FileSystem>
    └── UserSettingsManager.cpp     # [MODIFY] Чтение/запись user.dat через FileSystem
## 4. Чек-лист реализации и Definition of Done (DoD)

- [x] Добавить `c_PaksDirectoryRelativePath` в `src/core/constants/PackageConstants.h`
- [x] Добавить `Paks = 5` в `src/core/enums/eAssetDirectoryKind.h` (нумерация с 1, 0 зарезервирован как невалидный sentinel)
- [x] Модифицировать `src/core/io/Path.h` и `Path.cpp` (строго `friend class FileSystemBase`, геттеры `GetExecutableDirectory`, `GetUserDataDirectory`, ветка `Paks`)
- [x] Создать `src/core/enums/eFileLocation.h` (`App`, `User`, `Saves`, `Cache`, `Logs`)
- [x] Добавить перегрузку `ToString(eFileLocation)` в `src/core/enums/eEnumToString.h`
- [x] Добавить Правило 21 в `general_plan.md` (платформенная специализация через Compile-Time Type Aliases, Zero `#ifdef`)
- [x] Создать `FileSystemBase.h` / `FileSystemBase.cpp` (чистый C++23 без `#ifdef`, 4 каноничных метода без YAGNI, `InitializeUserData` в `private`, контрактная модель по Правилу 25)
- [x] Создать `FileSystemDesktop.h` / `FileSystemDesktop.cpp` (единая десктопная реализация Windows/Linux/macOS)
- [x] Создать `FileSystemAndroid.h` / `FileSystemAndroid.cpp` (с хелпером `TryOpenAsset` и однопроходным `ReadAllBytes`)
- [x] Создать `FileSystemiOS.h` / `FileSystemiOS.cpp` (заготовка под КП-5)
- [x] Создать центральный заголовок `FileSystem.h` с `using FileSystem = FileSystem...;`
- [x] Зарегистрировать новые файлы в `src/core/CMakeLists.txt`
- [x] Обновить `PackageManager` для точечного чтения через `FileSystem::ReadBytes` и чтения манифеста через `ReadAllBytes`
- [x] Обновить `UserSettingsManager` для работы через `FileSystem` (`FileExists`, `ReadAllBytes`, `WriteAllBytes`)
- [x] Обновить `Engine.h` / `Engine.cpp` для владения `FileSystem` и обязательной валидации инициализации путей
- [x] Собрать и прогнать все тесты `EngineTests.exe` (100% PASS: 64 из 64 тестов)
- [x] Собрать и запустить `game_win.exe` (сквозная интеграционная валидация FileSystem: чтение манифеста package.dat и работа с user.dat без исключений)
- [ ] Запросить утверждение у пользователя
- [ ] Зафиксировать Git-коммит: `feat(core): completed stage 05 - FileSystem compile-time aliases, FileSystemBase, FileSystemDesktop, eFileLocation`
- [ ] Обновить статус Пункта 5 в `general_plan.md` на `✅ Выполнено`

---

## 5. Результаты валидации и тестирования
1. **Набор тестов движка `EngineTests.exe`:**
```
[==========] 64 tests from 14 test suites ran. (515 ms total)
[  PASSED  ] 64 tests.
```
2. **Сквозной запуск игрового приложения `game_win.exe`:**
Приложение успешно собирается, линкуется и стартует:
- `FileSystem` создаётся в `Engine::Engine`;
- `PackageManager` успешно считывает манифест `assets/package.dat` из `eFileLocation::App` через `m_FileSystem->ReadAllBytes`;
- Каталог пользователя инициализируется через `InitializeUserData`;
- `UserSettingsManager` успешно выполняет проверку `FileExists` и чтение конфигурации из `eFileLocation::User`.
