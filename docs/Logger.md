# Logging System Architecture

Централизованная система логирования с поддержкой асинхронной рассылки и фильтрации.

## 1. Потоковая модель

### Игровой поток (Engine)
- Записывает сообщения в `DoubleBufferedVector<LogEntry>` (lock-free swap, без длительных блокировок)
- Основной поток игры никогда не занимается выводом в консоль или ожиданием сетевых сокетов

### Поток раздатчика (Logger)
- Работает событийно (Event-driven) по `std::condition_variable`
- При появлении логов делает `swap` буферов и передает текущий пакет каждому слушателю через `IBroadcaster::PushLogsBatch`

### Поток сетевого бродкастера (NetworkBroadcaster)
- Имеет собственную очередь и отдельный фоновый поток I/O
- Это исключает задержки сетевого соединения на главный поток или другие бродкастеры

## 2. Категории логирования

Каждое сообщение относится к категории логирования, определённой в `core/utils/LogCategory.h`.

### Встроенные GUARANTEED категории

Встроенные категории **никогда не фильтруются рантаймом** — даже на уровне `Message` (DOut):

| Категория | Имя в логе | Использование |
|-----------|-----------|---|
| `LogGeneral` | `General` | По умолчанию для файлов без Z_SET_LOG_CATEGORY |
| `LogEngine` | `Engine` | Встроенные логи движка |
| `GAPIVerbose` | `GAPIVerbose` | GAPI debug layer сообщения (DirectX12, Vulkan) |

Объявление:
```cpp
// src/core/utils/LogCategory.h
inline constexpr LogCategory LogGeneral{ "General", eLogCategoryGroup::Engine, true };
inline constexpr LogCategory LogEngine{ "Engine", eLogCategoryGroup::Engine, true };
inline constexpr LogCategory GAPIVerbose{ "GAPIVerbose", eLogCategoryGroup::Engine, true };
```

### Фильтруемые категории

Обычные категории, которые могут быть отключены рантаймом через фильтр (но не на уровне Warning/Error/Fatal):

| Категория | Объявление |
|-----------|-----------|
| GAPI | `Z_DECLARE_LOG_CATEGORY_ENGINE(GAPI)` |
| ECS | `Z_DECLARE_LOG_CATEGORY_ENGINE(ECS)` |
| Audio | `Z_DECLARE_LOG_CATEGORY_ENGINE(Audio)` |
| Physics | `Z_DECLARE_LOG_CATEGORY_ENGINE(Physics)` |
| Assets | `Z_DECLARE_LOG_CATEGORY_ENGINE(Assets)` |
| Network | `Z_DECLARE_LOG_CATEGORY_ENGINE(Network)` |
| UI | `Z_DECLARE_LOG_CATEGORY_ENGINE(UI)` |

Объявлены в `src/core/utils/Constants.h`.

### Специальные пользовательские категории

Фильтруемые категории, объявляемые в пользовательском коде:
```cpp
Z_DECLARE_LOG_CATEGORY_USER(MyCustomCategory);
```

### Использование категорий

**Явно:**
```cpp
DOut(MyCategory, "Message format: {}", value);
DOutWarning(MyCategory, "Warning");
```

**Через ambient категорию файла:**
```cpp
// В начале .cpp файла
Z_SET_LOG_CATEGORY(MyCategory);

DOut("Будет использована MyCategory");  // Использует ambient категорию
```

**По умолчанию:**
```cpp
DOut("Использует General (встроенная по умолчанию)");
```

### Фильтрация категорий рантаймом

Проверка `IsCategoryEnabled(category)` выполняется **только для уровня Message (DOut)**. Early Exit происходит ещё в `LogMacros.h`, до форматирования строки.

Порядок проверки:
1. `m_BypassAllFilters` — отладочный "рубильник", отключающий все фильтры
2. `category.IsGuaranteed()` — встроенные категории всегда проходят
3. Мастер-переключатель группы (Engine/User)
4. Точечный opt-out список отключённых категорий по имени

**Исключение:** Warning, Error, Exception, Critical, Fatal — гарантированная доставка (категория НЕ фильтруется рантаймом).

## 3. GAPI Debug Logging

### Z_GAPI_VERBOSE_DEBUG_LAYER define

Управляет **компиляцией** GAPI debug callback'а:

```cpp
#if (Z_DEBUG_BUILD || Z_DEVELOPMENT_BUILD) && Z_GAPI_VERBOSE_DEBUG_LAYER
    // Debug callback регистрируется
#endif
```

**На уровне компиляции** (в DirectX12API.cpp и VulkanAPI.cpp):
- Определяет, будет ли зарегистрирован debug message callback
- Определяет, будут ли включены VERBOSE сообщения в Vulkan validation layer

**На уровне выполнения** (в GAPIDebugLogger.cpp):
- GAPIVerbose — GUARANTEED категория, всегда выводит, если callback зарегистрирован

```cpp
// src/engine/gapi/GAPIDebugLogger.cpp
case eLogMessageType::Message:
    DOut(GAPIVerbose, "[GAPI:{}] {}", EnumToString::ToString(backend), message);
    break;
```

### DirectX12 debug callback

```cpp
// src/engine/gapi/directx12/DirectX12API.cpp
void DirectX12API::RegisterDebugMessageCallback()
{
#if (Z_DEBUG_BUILD || Z_DEVELOPMENT_BUILD) && Z_GAPI_VERBOSE_DEBUG_LAYER
    // Регистрирует DX12DebugMessageCallback через ID3D12InfoQueue
    // Все DX12 debug сообщения будут маршрутизированы в GAPIDebugLogger::Report()
#endif
}
```

### Vulkan debug messenger

```cpp
// src/engine/gapi/vulkan/VulkanAPI.cpp
void VulkanAPI::EnableDebugMessenger()
{
    VkDebugUtilsMessengerCreateInfoEXT debugInfo{};
    // ... базовые severity (INFO, WARNING, ERROR) ...
    
#if Z_GAPI_VERBOSE_DEBUG_LAYER
    debugInfo.messageSeverity |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;
#endif
    // Все Vulkan validation сообщения будут маршрутизированы в debug callback
}
```

## 4. Правила гарантированного вывода логов

### Уровни, которые ВСЕГДА выводятся (при Z_ADD_LOGGER)

- `DOutWarning()` — Warning
- `DOutError()` — Error
- `DOutException()` — Exception
- `DOutCritical()` — Critical
- `DOutFatal()` — Fatal

**Категория для этих уровней НЕ фильтруется рантаймом**, но внешние слушатели (RemoteLogViewer) могут независимо скрывать категории в своих собственных View Filters.

### Уровень Message (DOut)

- Обрабатывается только при активном `Z_ADD_LOGGER` (включает Z_DEVELOPMENT_BUILD)
- **Проходит фильтрацию категорий** рантаймом
- GUARANTEED категории (General, Engine, GAPIVerbose) всегда выводятся

### IDE Output

Если взведен макрос `Z_IDE_OUT_LOGS`, лог напрямую выводится в отладочную консоль IDE (`OutputDebugString` на Windows / `__android_log` на Android), даже если список слушателей пуст.

## 5. Управление очередью

- Если слушатели отсутствуют (`m_Listeners.empty()`), логи в память фонового буфера рассылки не записываются
- Размер сетевой очереди управляется значением из `ProjectManifestData` через вызов `SetMaxNetworkLogQueueSize`

## 6. RemoteLogViewer

RemoteLogViewer получает логи через сетевой протокол и отображает их с полной фильтрацией:

- Категория отображается с именем из `LogCategory.name` (General, Engine, GAPIVerbose)
- View Filters позволяют скрывать конкретные категории и группы
- Категории типа Warning/Error/Fatal отображаются всегда (независимо от рантайм-фильтра движка)

## 7. Макросы логирования

### Базовые макросы

```cpp
DOut(category, "format", args...)          // Message (filterable)
DOutWarning(category, "format", args...)   // Warning (guaranteed)
DOutError(category, "format", args...)     // Error (guaranteed)
DOutException(category, "format", args...) // Exception (guaranteed)
DOutCritical(category, "format", args...)  // Critical (guaranteed)
DOutFatal(category, "format", args...)     // Fatal (guaranteed)
```

### Throttled логирование

```cpp
DOut(category, 1.5f, "format", args...)    // Один раз в 1.5 секунды
DOut(category, 10, "format", args...)      // Один раз каждые 10 вызовов
```

### Conditional логирование

```cpp
DOut(category, condition, "format", args...) // Выводит только если condition == true
```

### Ambient категория файла

```cpp
// В начале .cpp файла
Z_SET_LOG_CATEGORY(MyCategory);

// Все DOut* в этом файле будут использовать MyCategory
CurrentFileLogCategory  // Текущая ambient категория
```

Если Z_SET_LOG_CATEGORY не вызван — используется `General`.

## 8. Использование примеры

### GAPI debug логирование

```cpp
// DirectX12 debug callback автоматически маршрутизирует сообщения в:
DOut(GAPIVerbose, "[GAPI:DirectX12] Device created");

// Доступно только если Z_GAPI_VERBOSE_DEBUG_LAYER взведен при компиляции
```

### Engine логирование

```cpp
Z_SET_LOG_CATEGORY(Engine);

DOut("Инициализация подсистемы...");
DOutWarning("Сомнительное состояние");
DOutError("Ошибка инициализации");
```

### Встроенное логирование

```cpp
// General используется по умолчанию
DOut("Это сообщение использует General категорию");
```

## 9. Отладка фильтрации

```cpp
// Включить обход всех фильтров категорий
g_Logger.SetBypassAllFilters(true);  // IsCategoryEnabled всегда вернёт true

// Проверить конкретную категорию
if (g_Logger.IsCategoryEnabled(GAPIVerbose)) {
    // GAPIVerbose проходит фильтр
}

// Отключить категорию рантаймом
g_Logger.SetCategoryEnabled("GAPI", false);

// Отключить всю группу категорий
g_Logger.SetGroupEnabled(eLogCategoryGroup::Engine, false);
```

