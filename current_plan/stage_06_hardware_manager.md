# Этап 06: Рефакторинг подсистемы аппаратуры (`HardwareManager`, `HardwareState`)

## 1. Контекст и цели этапа
- **Номер пункта:** **Пункт 6** (Уровень 1: Математический и файловый фундамент).
- **Цель:** Вынести сбор телеметрии оборудования (CPU, RAM, Материнская плата, GPU, Storage, Network) из монолитного `Platform::GatherHardwareState()` (~250 строк на Windows, все категории вперемешку в одной функции) в чистый сервис `HardwareManager`, построенный по Правилу 21 (Compile-Time Type Aliases, 0 `#ifdef` в теле методов, изоляция нативных SDK — Win32 DXGI/SMBIOS, POSIX `/proc`, `sysctl`, IOKit и т.д. — каждый в своём файле).
- **Статус:** `✅ Выполнено` — реализация выполнена, собрана и успешно верифицирована.
- **Зависимости:** `src/core/hardware/*.h` (не изменялись), `src/engine/platforms/Platform.h`, `src/engine/platforms/PlatformCommon.cpp`, `src/engine/platforms/monitor/IMonitorProvider.h`, `src/engine/utils/GpuUtils.h`, `src/engine/utils/MonitorUtils.h`.

---

## 2. Ревизия существующего кода (Правило 15)

1. **`Platform::GatherHardwareState()` был реализован ТОЛЬКО для двух платформ:**
   - **Windows** (`PlatformWindows.cpp`) — рабочая, но монолитная реализация: мониторы (через `m_MonitorProvider`), GPU (DXGI `IDXGIFactory1::EnumAdapters1`), CPU (`GetSystemInfo`), RAM (`GlobalMemoryStatusEx` + разбор SMBIOS Type 17), материнская плата (реестр `HARDWARE\DESCRIPTION\System\BIOS` + разбор SMBIOS Type 1 для UUID — **SMBIOS-таблица читалась и парсилась ДВАЖДЫ**, отдельно для RAM и отдельно для UUID; устранено декомпозицией через `SmbiosReaderMSWin`), диски (`GetLogicalDriveStringsW` + `IOCTL_STORAGE_QUERY_PROPERTY`), сеть (`GetAdaptersAddresses`) — всё в одной функции.
   - **Editor** (`PlatformEditor.cpp`) — чистый статический стаб (по одному фейковому CPU/RAM/GPU/Monitor).
2. **`PlatformLinux.cpp`, `PlatformMacOS.mm`, `PlatformAndroid.cpp`, `PlatformiOS.mm` содержали мёртвый, не компилирующийся код.** Их `GatherHardwareState()` возвращал несуществующий тип `PlatformHardwareState` — остаток от модели, предшествовавшей текущему `HardwareState`. Устранено: метод и мёртвый тип полностью удалены из этих файлов и из `Platform.h` (заменены на `HardwareManager`, см. п. 5).
3. **`core/hardware/*.h` (CpuInfo, RamInfo, GpuInfo, MotherboardInfo, StorageInfo, NetworkAdapterInfo, MonitorInfo, HardwareState) не менялись** — чистые DTO, целевой контракт для новых коллекторов.
4. **`IMonitorProvider` не тронут.** `HardwareManager` только опрашивает существующий `m_MonitorProvider` за списком мониторов для заполнения `HardwareState::m_Monitors`.
5. **Пересечений с GPU-подсистемой рендера нет** (DXGI-перечисление адаптеров в `DirectX12API.cpp`/`Swapchain_DX.cpp` — отдельная задача выбора адаптера рендера, не телеметрия).
6. **Решение (Правило 15): монолит `PlatformWindows.cpp::GatherHardwareState()` декомпозирован в новую независимую подсистему `platforms/hardware/`.** Код Windows-реализации перенесён (не переписан с нуля) в новые классы-коллекторы почти 1:1, с устранением дублирования разбора SMBIOS.

---

## 3. Архитектура (Правило 21: Compile-Time Type Aliases + декомпозиция по SRP)

### 3.1. `HardwareManager` — тонкий агрегатор без `#ifdef`

```cpp
// src/engine/platforms/hardware/HardwareManager.h
#pragma once

#include "core/hardware/HardwareState.h"
#include "engine/platforms/hardware/CpuInfoCollector.h"
#include "engine/platforms/hardware/RamInfoCollector.h"
#include "engine/platforms/hardware/MotherboardInfoCollector.h"
#include "engine/platforms/hardware/GpuInfoCollector.h"
#include "engine/platforms/hardware/StorageInfoCollector.h"
#include "engine/platforms/hardware/NetworkAdapterInfoCollector.h"

namespace zzz::engine
{
	class IMonitorProvider;

	class HardwareManager final
	{
	public:
		explicit HardwareManager(const IMonitorProvider& monitorProvider);

		[[nodiscard]] const HardwareState& GetHardwareState() const noexcept { return m_HardwareState; }

	private:
		[[nodiscard]] HardwareState Gather(const IMonitorProvider& monitorProvider) const;

		CpuInfoCollector m_CpuCollector;
		RamInfoCollector m_RamCollector;
		MotherboardInfoCollector m_MotherboardCollector;
		GpuInfoCollector m_GpuCollector;
		StorageInfoCollector m_StorageCollector;
		NetworkAdapterInfoCollector m_NetworkCollector;

		HardwareState m_HardwareState;
	};
}
```

`HardwareManager.cpp` — единственное место, где `Gather()` вызывает все `Collect()` коллекторов и собирает результат в `HardwareState`. Ноль платформенных дефайнов.

### 3.2. Единый паттерн диспетчера коллектора (реализовано, на примере Motherboard)

Финальный стиль диспетчера: **все `#include` вынесены безусловно наверх** (каждый platform-заголовок сам себя гвардит через `#if defined(Z_...) ... #endif`, по прецеденту `core/headers/{Apple,MSWin,Linux,Android}.h`), и **один общий `namespace { #if/#elif ... }` блок** на весь файл — вместо повторения `namespace` внутри каждой ветки `#if`. Ветка `Z_EDITOR` идёт **первой**, до всех ОС-веток (тот же порядок, что в `Input.h`/`MainLoop.h`):

```cpp
// src/engine/platforms/hardware/MotherboardInfoCollector.h
#pragma once
#include "core/utils/Defines.h"

#include "platforms/motherboard/MotherboardInfoCollectorEditor.h"
#include "platforms/motherboard/MotherboardInfoCollectorMSWin.h"
#include "platforms/motherboard/MotherboardInfoCollectorLinux.h"
#include "platforms/motherboard/MotherboardInfoCollectorMacOS.h"
#include "platforms/motherboard/MotherboardInfoCollectorMobile.h"

namespace zzz::engine
{
#if defined(Z_EDITOR)
	using MotherboardInfoCollector = MotherboardInfoCollectorEditor;
#elif defined(Z_WINDOWS)
	using MotherboardInfoCollector = MotherboardInfoCollectorMSWin;
#elif defined(Z_LINUX)
	using MotherboardInfoCollector = MotherboardInfoCollectorLinux;
#elif defined(Z_MACOS)
	using MotherboardInfoCollector = MotherboardInfoCollectorMacOS;
#elif defined(Z_MOBILE)
	using MotherboardInfoCollector = MotherboardInfoCollectorMobile;
#else
	#error ">>>>> MotherboardInfoCollector: Unsupported platform."
#endif
}
```

Каждый `XxxInfoCollectorYyy` — маленький класс с единственным публичным методом `Collect() const`, файл целиком обёрнут в `#if defined(Z_...) ... #endif` (Правило 23).

**Появление ветки `Z_EDITOR` во всех 6 диспетчерах — отличие от первоначального плана**, потребовавшееся по ходу реализации: удаление `Platform::GatherHardwareState()` из `Platform.h` ломало собственное определение этого метода в `PlatformEditor.cpp`. Решение — 6 новых честных стабов `*CollectorEditor.h` (header-only), воспроизводящих исходные фейковые данные `PlatformEditor.cpp` 1:1 для CPU/RAM/Motherboard/GPU; Storage/Network-стабы для Editor — пустые списки (в оригинале их не было вовсе). См. важное следствие в п. 3.5.3.

### 3.3. Платформенная развилка по каждому коллектору

| Коллектор | Windows | Linux | macOS | iOS | Android | Editor |
|---|---|---|---|---|---|---|
| **CpuInfoCollector** | `CpuInfoCollectorMSWin` (`GetSystemInfo`, перенос) | `CpuInfoCollectorLinux` (`/proc/cpuinfo` + `sysconf`) | `CpuInfoCollectorApple` (`Z_APPLE`: `sysctlbyname`) | = macOS | `CpuInfoCollectorAndroid` (`/proc/cpuinfo`, изолирован) | `CpuInfoCollectorEditor` (фейковый 1 CPU) |
| **RamInfoCollector** | `RamInfoCollectorMSWin` (`GlobalMemoryStatusEx` + SMBIOS Type 17 через `SmbiosReaderMSWin`) | `RamInfoCollectorLinux` (`/proc/meminfo`) | `RamInfoCollectorApple` (`sysctl hw.memsize` + `host_statistics64`) | = macOS | `RamInfoCollectorAndroid` (`/proc/meminfo`) | `RamInfoCollectorEditor` (фейковое значение) |
| **MotherboardInfoCollector** | `MotherboardInfoCollectorMSWin` (реестр + SMBIOS Type 1 через `SmbiosReaderMSWin`) | `MotherboardInfoCollectorLinux` (`/sys/class/dmi/id/*`) | `MotherboardInfoCollectorMacOS` (IOKit `IOPlatformExpertDevice`) | = Mobile | `MotherboardInfoCollectorMobile` (единый честный стаб Android+iOS) | `MotherboardInfoCollectorEditor` (фейковые строки) |
| **StorageInfoCollector** | `StorageInfoCollectorMSWin` (`IOCTL_STORAGE_QUERY_PROPERTY`) | `StorageInfoCollectorLinux` (`/proc/mounts` + `statvfs`) | `StorageInfoCollectorMacOS` (`getmntinfo` + `statfs`) | = Mobile | `StorageInfoCollectorMobile` (**пустой вектор**, см. 3.5.1) | `StorageInfoCollectorEditor` (пустой вектор) |
| **NetworkAdapterInfoCollector** | `NetworkAdapterInfoCollectorMSWin` (`GetAdaptersAddresses`) | `NetworkAdapterInfoCollectorLinux` (`getifaddrs`, отдельный файл — вариант (а), см. 3.5.2) | `NetworkAdapterInfoCollectorMacOS` (`getifaddrs`, отдельный файл) | = Mobile | `NetworkAdapterInfoCollectorMobile` (**пустой вектор**, см. 3.5.1) | `NetworkAdapterInfoCollectorEditor` (пустой вектор) |
| **GpuInfoCollector** | `GpuInfoCollectorMSWin` (DXGI `EnumAdapters1`, перенос) | `GpuInfoCollectorStub` (`Z_LINUX \| Z_MACOS \| Z_MOBILE`, один элемент `"Unknown"`) | = Linux | = Linux | = Linux | `GpuInfoCollectorEditor` (фейковый 1 GPU) |

### 3.4. Устранение дублирования: `SmbiosReaderMSWin`

Файл `src/engine/platforms/hardware/platforms/common/SmbiosReaderMSWin.h/.cpp` — статический метод `ReadRawTable()` с **самокэширующимся результатом** (function-local `static const`, magic static): физический вызов `GetSystemFirmwareTable('RSMB', ...)` выполняется максимум один раз за время жизни процесса независимо от того, сколько коллекторов (`RamInfoCollectorMSWin`, `MotherboardInfoCollectorMSWin`) его вызывают. Это позволило оставить сигнатуру `Collect() const` единообразной для всех платформ, без передачи общего буфера через параметры или через `HardwareManager`.

Файл `src/engine/platforms/hardware/platforms/common/WCharUtilsMSWin.h` — общий инлайн-хелпер `WCharToUtf8MSWin(const WCHAR*)`, переиспользуемый MSWin-коллекторами Motherboard/Storage/Network/Gpu.

### 3.5. Решённые вопросы и отклонения от первоначального плана

1. **Mobile Storage/Network — упрощены до честных пустых стабов**, а не «`statfs` по песочнице приложения», как было заявлено в первоначальной версии плана (п. 3.3 старой редакции). Причина: реальный сбор потребовал бы протаскивать `NativeAppData` (путь песочницы) через конструктор коллектора, что сломало бы единообразный, не зависящий от платформы контракт конструирования `HardwareManager` (Правило 30, YAGNI — не усложняем ради гипотетической будущей потребности). При необходимости добавляется отдельным пунктом позже.
2. **`NetworkAdapterInfoCollector` для Linux/macOS — вариант (а).** Два отдельных файла (`NetworkAdapterInfoCollectorLinux.h`, `NetworkAdapterInfoCollectorMacOS.h`), каждый с собственным телом реализации через `getifaddrs`, без общего базового/хелпер-класса — по явному указанию пользователя.
3. **`PlatformEditor.cpp` — код не переведён на реальную Windows-телеметрию** (по указанию «эдитор пока пропускаем»), но само удаление `GatherHardwareState()` из `Platform.h` потребовало завести `Z_EDITOR`-ветки и коллекторы-стабы (см. 3.2). **Важное следствие, требующее внимания пользователя:** список мониторов (`HardwareState::m_Monitors`) для Editor-сборки теперь приходит из **реального** `m_MonitorProvider` (`MonitorProviderMSWin`, он и раньше существовал в `Platform`, просто не использовался для `HardwareState`), а не из единственного захардкоженного фейкового `"Editor Monitor"`, как было в старом `PlatformEditor.cpp::GatherHardwareState()`. Все остальные поля (CPU/RAM/Motherboard/GPU/Storage/Network) остаются честными Editor-стабами 1:1 со старым поведением. Технически это отклонение от строгого требования DoD «лог до/после совпадает 1:1» — но только для секции мониторов, и в сторону более честных данных, а не менее.

---

## 4. Файловая структура (реализовано)

```
src/engine/platforms/
├── Platform.h                                  # [MODIFIED] unique_ptr<HardwareManager> вместо HardwareState/GatherHardwareState
├── PlatformCommon.cpp                           # [MODIFIED] Создание m_HardwareManager; GetHardwareState() делегирует
├── PlatformWindows.cpp                          # [MODIFIED] 451 → 85 строк, GatherHardwareState() удалён целиком
├── PlatformLinux.cpp                            # [MODIFIED] Удалён мёртвый PlatformHardwareState/GatherHardwareState
├── PlatformMacOS.mm                             # [MODIFIED] Аналогично
├── PlatformAndroid.cpp                          # [MODIFIED] Аналогично
├── PlatformiOS.mm                               # [MODIFIED] Аналогично
├── PlatformEditor.cpp                           # [MODIFIED] GatherHardwareState() удалён, логика перенесена в *CollectorEditor.h
└── hardware/
    ├── HardwareManager.h / .cpp                  # [NEW]
    ├── CpuInfoCollector.h                        # [NEW] диспетчер
    ├── RamInfoCollector.h                        # [NEW] диспетчер
    ├── MotherboardInfoCollector.h                # [NEW] диспетчер
    ├── GpuInfoCollector.h                        # [NEW] диспетчер
    ├── StorageInfoCollector.h                    # [NEW] диспетчер
    ├── NetworkAdapterInfoCollector.h              # [NEW] диспетчер
    └── platforms/
        ├── common/
        │   ├── WCharUtilsMSWin.h                 # [NEW]
        │   └── SmbiosReaderMSWin.h / .cpp         # [NEW]
        ├── cpu/
        │   ├── CpuInfoCollectorMSWin.h / .cpp     # [NEW]
        │   ├── CpuInfoCollectorLinux.h / .cpp     # [NEW]
        │   ├── CpuInfoCollectorApple.h / .cpp     # [NEW] (macOS + iOS)
        │   ├── CpuInfoCollectorAndroid.h / .cpp   # [NEW]
        │   └── CpuInfoCollectorEditor.h           # [NEW]
        ├── ram/
        │   ├── RamInfoCollectorMSWin.h / .cpp     # [NEW]
        │   ├── RamInfoCollectorLinux.h / .cpp     # [NEW]
        │   ├── RamInfoCollectorApple.h / .cpp     # [NEW] (macOS + iOS)
        │   ├── RamInfoCollectorAndroid.h / .cpp   # [NEW]
        │   └── RamInfoCollectorEditor.h           # [NEW]
        ├── motherboard/
        │   ├── MotherboardInfoCollectorMSWin.h / .cpp  # [NEW]
        │   ├── MotherboardInfoCollectorLinux.h / .cpp  # [NEW]
        │   ├── MotherboardInfoCollectorMacOS.h / .cpp  # [NEW]
        │   ├── MotherboardInfoCollectorMobile.h        # [NEW] (Android + iOS, честный стаб)
        │   └── MotherboardInfoCollectorEditor.h        # [NEW]
        ├── storage/
        │   ├── StorageInfoCollectorMSWin.h / .cpp      # [NEW]
        │   ├── StorageInfoCollectorLinux.h / .cpp      # [NEW]
        │   ├── StorageInfoCollectorMacOS.h / .cpp      # [NEW]
        │   ├── StorageInfoCollectorMobile.h            # [NEW] (пустой вектор, см. 3.5.1)
        │   └── StorageInfoCollectorEditor.h            # [NEW]
        ├── network/
        │   ├── NetworkAdapterInfoCollectorMSWin.h / .cpp  # [NEW]
        │   ├── NetworkAdapterInfoCollectorLinux.h / .cpp  # [NEW]
        │   ├── NetworkAdapterInfoCollectorMacOS.h / .cpp  # [NEW]
        │   ├── NetworkAdapterInfoCollectorMobile.h        # [NEW] (пустой вектор, см. 3.5.1)
        │   └── NetworkAdapterInfoCollectorEditor.h        # [NEW]
        └── gpu/
            ├── GpuInfoCollectorMSWin.h / .cpp     # [NEW]
            ├── GpuInfoCollectorStub.h             # [NEW] (Linux + macOS + Mobile)
            └── GpuInfoCollectorEditor.h           # [NEW]
```

Итого 57 новых файлов под `hardware/`, разбитых по подпапкам-категориям (`cpu/`, `ram/`, `motherboard/`, `storage/`, `network/`, `gpu/`, `common/`) — плоская `platforms/` с ~50 файлами на одном уровне была признана неудобной и разбита по указанию пользователя.

`src/engine/CMakeLists.txt` обновлён (проверено `git diff`, +41 строка, только добавления):
- Базовый `target_sources`: `HardwareManager.h/.cpp`, 6 диспетчеров, 4 кросс-платформенных стаба (`MotherboardInfoCollectorMobile.h`, `StorageInfoCollectorMobile.h`, `NetworkAdapterInfoCollectorMobile.h`, `GpuInfoCollectorStub.h`).
- `WIN32`/`IS_EDITOR`-ветка: 6 `*CollectorEditor.h`.
- `WIN32`/`else()`-ветка: `WCharUtilsMSWin.h`, `SmbiosReaderMSWin.h/.cpp`, 6 пар `*CollectorMSWin.h/.cpp`.
- `UNIX AND NOT APPLE AND NOT ANDROID`: 5 пар `*CollectorLinux.h/.cpp`.
- `ANDROID`: `CpuInfoCollectorAndroid.h/.cpp`, `RamInfoCollectorAndroid.h/.cpp`.
- `APPLE` (IOS + macOS): `CpuInfoCollectorApple.h/.cpp`, `RamInfoCollectorApple.h/.cpp`; только macOS-ветка дополнительно: `MotherboardInfoCollectorMacOS.h/.cpp`, `StorageInfoCollectorMacOS.h/.cpp`, `NetworkAdapterInfoCollectorMacOS.h/.cpp`.

Сверка: все 57 файлов на диске присутствуют в CMakeLists.txt и наоборот (проверено скриптом), стрелых ссылок на старый плоский путь (`hardware/platforms/SmbiosReaderMSWin.h` и т.п.) и на удалённые `GatherHardwareState`/`PlatformHardwareState` в дереве `src` не найдено (`git grep` — 0 совпадений).

---

## 5. Изменения в `Platform` / `PlatformCommon.cpp`

**Указатель — `unique_ptr`, не `shared_ptr`** (скорректировано по вопросу пользователя в ходе обсуждения). `HardwareManager` не полиморфный и не расшаривается ни с кем — единственный владелец `Platform`. Указатель как таковой нужен не ради владения, а чтобы `Platform.h` мог обойтись форвард-декларацией `class HardwareManager;`, не затягивая в свой include-граф заголовки всех шести коллекторов (а через них — нативные SDK-хедеры вроде `<windows.h>`). Тот же приём, что и для `m_MonitorProvider`, только повод другой (там — полиморфизм через `IMonitorProvider`, здесь — изоляция инклудов).

```cpp
// Platform.h (реализовано)
class Platform final
{
public:
	...
	[[nodiscard]] const HardwareState& GetHardwareState() const noexcept;
	[[nodiscard]] const IMonitorProvider& GetMonitorProvider() const noexcept;
	...
private:
	void Initialize();
	void InitializePlatformSpecific();
	void ShutdownPlatformSpecific();

	std::shared_ptr<NativeAppData> m_NativeData;
	ProjectPlatformData m_PlatformData;
	std::shared_ptr<IMonitorProvider> m_MonitorProvider;
	std::unique_ptr<HardwareManager> m_HardwareManager;
};
```

```cpp
// PlatformCommon.cpp — конструктор (реализовано)
m_HardwareManager(safe_make_unique<HardwareManager>(*m_MonitorProvider))

const HardwareState& Platform::GetHardwareState() const noexcept { return m_HardwareManager->GetHardwareState(); }
```

`~Platform()` объявлен в `Platform.h`, определён в `PlatformCommon.cpp` (где `HardwareManager` — полный тип) — корректно для `unique_ptr` на неполный тип. Публичный контракт `Platform::GetHardwareState()` не изменился.

---

## 6. Чек-лист реализации и Definition of Done (DoD)

- [x] Создать `SmbiosReaderMSWin.h/.cpp`, устранить двойной парсинг SMBIOS
- [x] Создать 6 dispatcher-заголовков коллекторов с `Z_EDITOR`+ОС-ветками и `#error` на неподдерживаемой платформе
- [x] Создать `CpuInfoCollectorMSWin/Linux/Apple/Android/Editor`
- [x] Создать `RamInfoCollectorMSWin/Linux/Apple/Android/Editor`
- [x] Создать `MotherboardInfoCollectorMSWin/Linux/MacOS/Mobile/Editor`
- [x] Создать `StorageInfoCollectorMSWin/Linux/MacOS/Mobile/Editor` (Mobile — пустой вектор, см. 3.5.1)
- [x] Создать `NetworkAdapterInfoCollectorMSWin/Linux/MacOS/Mobile/Editor` (Mobile — пустой вектор, см. 3.5.1)
- [x] Создать `GpuInfoCollectorMSWin/Stub/Editor`
- [x] Создать `HardwareManager.h/.cpp`, реализовать `Gather()`
- [x] Удалить `Platform::GatherHardwareState()`; добавить `m_HardwareManager` (unique_ptr)
- [x] Обновить `PlatformCommon.cpp`
- [x] Очистить `PlatformWindows.cpp` (451 → 85 строк)
- [x] Починить мёртвый код в `PlatformLinux.cpp`/`PlatformMacOS.mm`/`PlatformAndroid.cpp`/`PlatformiOS.mm`
- [x] Обновить `PlatformEditor.cpp` (удалить `GatherHardwareState()`, логика → `*CollectorEditor.h`)
- [x] Зарегистрировать все 57 новых файлов в `src/engine/CMakeLists.txt` (проверено скриптом на полноту в обе стороны)
- [x] **Собрать `EngineTests.exe`, все тесты зелёные** (64/64 успешно пройдены)
- [x] **Собрать и запустить `game_win.exe`, сверить лог `HardwareState::LogFileBlock()` до/после** (успешный запуск и завершение)
- [x] Запросить утверждение у пользователя (утверждено пользователем)
- [x] Зафиксировать Git-коммит
- [x] Обновить статус Пункта 6 в `general_plan.md` на `✅ Выполнено`

---

## 7. Результаты валидации и тестирования

1. **Компиляция под MSVC x64 + Ninja:**
   - Сборка целей `EngineTests` и `game_win` прошла без ошибок и предупреждений (`exit code 0`).
   - Все 57 платформенных файлов успешно зарегистрированы в `src/engine/CMakeLists.txt` под своими платформенными ветками.
2. **Юнит-тесты (`EngineTests.exe`):**
   - Выполнено 64 теста из 14 тест-сьютов: `[  PASSED  ] 64 tests. (520 ms total)`, 0 failures.
3. **Сквозной запуск (`game_win.exe`):**
   - Приложение успешно инициализирует `HardwareManager` через `Platform`, выводит полный срез оборудования в лог и штатно завершается с кодом возврата 0.
