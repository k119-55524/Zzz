[Русский](README.md) | [English](README.en.md)

# Zzz

A multiplatform C++23 framework for building modern UI applications with integrated 3D graphics.

## Project Goals

The primary goal of the project is to build a robust and lightweight technological foundation for a planned commercial application (details undisclosed), combining a rich user interface, 3D graphics, and high power efficiency:

- **WPF-like UI Layer (MVVM):** custom UI controls with On-Demand rendering (frames are rendered only when visual changes occur), significantly preserving battery life on mobile devices and laptops without continuous animation.
- **Integrated 3D Graphics:** full 3D engine capabilities on top of a cross-platform GAPI (DirectX 12, Vulkan, Metal) with seamless embedding of interactive 3D elements and shader effects directly into UI components.
- **C++ Scripting:** UI logic and scene behavior implemented in pure C++ for maximum responsiveness without virtual machine or garbage collector overhead.
- **Asset Infrastructure:** dedicated **Assets Builder** to compile raw assets into compact binary packages (`package.dat`, `data.dat`, `.meta`) with asynchronous streaming at runtime.

### Current Milestone (MVP)
Execution of the end-to-end pipeline: from preparing assets in Assets Builder to asynchronous loading and rendering of a textured spinning 3D cube controlled by a user C++ script via GAPI (DX12/Vulkan), providing the base for the subsequent integration of the ZzzGUI layer.

---

## Environment Requirements (Windows)

- Visual Studio 2022 / 2026 (MSVC with C++23 standard support).
- CMake 3.28+ and Ninja.
- .NET 8 / 9 SDK (for building C# tools and the editor).

> Detailed instructions for setting up environments on other platforms (Linux/WSL, macOS, iOS) are available in **[docs/DEVELOPMENT.md](docs/DEVELOPMENT.md)**.

---

## Building and Getting Started

### 1. Build Configuration Setup

Configuring active modules, tests, and selecting the graphics backend (DirectX 12 / Vulkan) is managed via `build_configs/current.cmake`.

To configure and switch presets, use:
- **CLI Config Switcher (C++):** `build_configs/build_configurator_switch.exe` (initial build: `automation\build_config_switcher.bat`).
- **GUI Configurator (WPF):** `build_configs/BuildConfigurator.exe` (initial build: `automation\build_configurator.bat`).

### 2. Building on Windows

Building can be done in two main ways:
- **Via Batch Script:** run `run_build.bat` in the repository root (compiles via CMake + Ninja in Debug mode).
- **Via Visual Studio:** open the repository root as a folder (**Open Folder**). Visual Studio will automatically detect `CMakePresets.json`. Select the desired configuration (`Debug`, `Release`, `Development`) and build.

All compiled binaries, libraries, and tools are placed into:
`dist/<Config>/` (e.g., `dist/Debug/`).

---

## Developer Tools

All developer tools and supporting C# projects are grouped into a single solution **`src/tools/Tools.sln`** for convenient building and joint debugging with the C++ engine:

| Tool | Description | Binary Path | Build Method |
| :--- | :--- | :--- | :--- |
| **Assets Builder** | Compiles assets into binary resource packages (`package.dat`, `data.dat`) | `dist/<Config>/assets_builder_gui.exe` | Automatically in CMake (`assets_builder_gui_build`) or via `Tools.sln` |
| **Editor** | C# WPF scene and project editor | `dist/<Config>/editor.exe` | Via `src/tools/Tools.sln` |
| **Remote Log Viewer** | Real-time network log viewer for engine diagnostics | `dist/<Config>/RemoteLogViewer.exe` | Automatically in CMake or via `Tools.sln` (root shortcut: `RemoteLogViewer.lnk`) |
| **Build Configurator** | GUI profile and feature configurator (WPF) | `build_configs/BuildConfigurator.exe` | `automation\build_configurator.bat` or `Tools.sln` |
| **Config Switcher** | CLI profile switcher (C++) | `build_configs/build_configurator_switch.exe` | `automation\build_config_switcher.bat` |

---

## Tests and Benchmarks

The project uses Google Test and Google Benchmark (`src/qa/tests` and `src/qa/benchmark`).

- Test compilation is toggled via the `Z_ADD_PROJECT_TESTS_IN_BUILD` definition in the active profile (`build_configs/current.cmake`).
- Test executables are built into `dist/<Config>/EngineTests.exe` and `dist/<Config>/EngineBenchmarks.exe`.

---

## Documentation

- **[docs/ARCHITECTURE.md](docs/ARCHITECTURE.md)** — engine architecture, subsystems, resource formats, and user data directories.
- **[docs/DEVELOPMENT.md](docs/DEVELOPMENT.md)** — detailed platform guides for macOS, iOS, Linux (WSL), and remote logging.
- **[current_plan/RULES.md](current_plan/RULES.md)** — architectural rules, code standards, and workflow requirements.
