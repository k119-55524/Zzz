# =============================================================================
#  Централизованный реестр версий внешних пакетов и зависимостей движка Zzz
# 
#  ДАННЫЙ ФАЙЛ ЯВЛЯЕТСЯ ЕДИНСТВЕННЫМ ИСТОЧНИКОМ ИСТИНЫ ДЛЯ ВСЕХ ВЕРСИЙ ПАКЕТОВ,
#  БИБЛИОТЕК И ИНСТРУМЕНТОВ, ИСПОЛЬЗУЕМЫХ ВО ВСЕМ ПРОЕКТЕ.
# 
#  Все подмодули CMake (D3D12, GTest, GBenchmark, .NET SDK и т.д.) обязаны бережно
#  ссылаться на переменные из этого файла. Изменение версии в этом файле
#  гарантирует синхронное обновление зависимостей на всех рабочих машинах
#  и исключает конфликты сборок между разными разработчиками.
# =============================================================================

# -----------------------------------------------------------------------------
# Пакеты Direct3D 12 (для Windows сборок)
# -----------------------------------------------------------------------------
set(Z_WINPIX_VERSION "1.0.240308001" CACHE STRING "WinPixEventRuntime NuGet Version")
set(Z_DXC_VERSION "v1.8.2502" CACHE STRING "DXC Version Tag")
set(Z_DXC_DATE "2025_02_20" CACHE STRING "DXC Release Archive Date")

# -----------------------------------------------------------------------------
# Модули QA / Тестирование и Бенчмарки
# -----------------------------------------------------------------------------
set(Z_GTEST_VERSION "v1.14.0" CACHE STRING "GoogleTest Git Tag")
set(Z_GBENCH_VERSION "v1.9.5" CACHE STRING "GoogleBenchmark Git Tag")

# -----------------------------------------------------------------------------
# Вспомогательные утилиты / .NET SDK
# -----------------------------------------------------------------------------
set(Z_DOTNET_CHANNEL "10.0" CACHE STRING ".NET SDK Channel Version")

# -----------------------------------------------------------------------------
# Генератор документации Doxygen, темы и Graphviz
# -----------------------------------------------------------------------------
set(Z_DOXYGEN_VERSION "1.12.0" CACHE STRING "Doxygen Windows Portable Version")
set(Z_DOXYGEN_AWESOME_VERSION "v2.4.1" CACHE STRING "Doxygen Awesome CSS Git Tag")
set(Z_GRAPHVIZ_VERSION "12.2.1" CACHE STRING "Graphviz Windows Portable Version")

# -----------------------------------------------------------------------------
# Библиотеки обработки текстур
# -----------------------------------------------------------------------------
set(Z_STB_VERSION "master" CACHE STRING "stb Git Commit/Tag")
set(Z_DIRECTXTEX_VERSION "oct2024" CACHE STRING "DirectXTex Release Tag")

# -----------------------------------------------------------------------------
# Библиотеки компиляции и трансляции шейдеров
# -----------------------------------------------------------------------------
set(Z_SPIRV_CROSS_VERSION "vulkan-sdk-1.3.296.0" CACHE STRING "SPIRV-Cross Git Tag")

# -----------------------------------------------------------------------------
# Автоматический контроль кэша зависимостей (.cache/)
# Проверяем наличие ключевых маркерных файлов.
# Если все зависимости присутствуют — включаем полный оффлайн-режим (0 обращений к сети).
# Если хотя бы одной нет или кэш пуст — разрешаем FetchContent докачать недостающее.
# -----------------------------------------------------------------------------
if(EXISTS "${FETCHCONTENT_BASE_DIR}/googletest-src/googletest/include/gtest/gtest.h"
   AND EXISTS "${FETCHCONTENT_BASE_DIR}/googlebenchmark-src/include/benchmark/benchmark.h"
   AND EXISTS "${FETCHCONTENT_BASE_DIR}/directxtex-src/DirectXTex/DirectXTex.h"
   AND EXISTS "${FETCHCONTENT_BASE_DIR}/stb-src/stb_image.h"
   AND EXISTS "${FETCHCONTENT_BASE_DIR}/dxc-src/inc/dxcapi.h"
   AND EXISTS "${FETCHCONTENT_BASE_DIR}/spirv_cross-src/spirv_cross.hpp")
    set(FETCHCONTENT_FULLY_DISCONNECTED ON CACHE BOOL "Fully disconnect FetchContent when dependencies exist" FORCE)
else()
    set(FETCHCONTENT_FULLY_DISCONNECTED OFF CACHE BOOL "Allow download when dependencies missing" FORCE)
    set(FETCHCONTENT_UPDATES_DISCONNECTED ON CACHE BOOL "Skip update step for already-populated FetchContent dependencies" FORCE)
endif()
