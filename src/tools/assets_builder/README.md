# Assets Builder

Инструмент сборки пакетов ресурсов и скриптов (**Assets Packages**) для экосистемы **Zzz Engine**.

## 📁 Структура компонентов

* **`assets_builder_dll/`** *(C++ Dynamic Library)*  
  Нативный C-API мост к C++ движку Zzz. Подключает `core_lib` и вызывает оригинальный сериализатор `zzz::serialize::Serializer`.

* **`assets_builder_lib/`** *(C# Class Library — `.dll`)*  
  Ядро сборщика (`AssetsBuilderEngine`). Отвечает за логику оркестрации сборки, обработку конфигов, вызов C++ сериализации через P/Invoke (`NativeMethods.cs`) и управление сессией (`SessionManager`).

* **`assets_builder_gui/`** *(C# WPF Application — `.exe`)*  
  Графический интерфейс тулзы сборки. Выполнен в фирменном стиле **Zzz Editor**: тёмно-синяя палитра, кастомная шапка окна, жёлтая обводка и панель состояния.

---

## ⚙️ Хранение настроек сессии

Пользовательские пути к исходным проектам пакетов и директориям назначения сохраняются в единую папку экосистемы компании в `AppData`:

`%LOCALAPPDATA%/Zzz/AssetsBuilder/1.0.0/session_config.json`

---

## 🛠 Сборка и интеграция

* C++ библиотека включена в корневой [CMakeLists.txt](file:///c:/Workspaces/ZzzTest/CMakeLists.txt#L115).
* C# проекты привязаны к общего решению [Tools.sln](file:///c:/Workspaces/ZzzTest/src/tools/Tools.sln).
