# =============================================================================
#  Модуль интеграции Doxygen и темы Doxygen Awesome для Zzz Engine
# =============================================================================

include(FetchContent)

# 1. Поиск системного Doxygen или автоматическое скачивание portable-версии (Windows)
find_package(Doxygen QUIET)

if(NOT DOXYGEN_FOUND)
    if(WIN32)
        message(STATUS "Doxygen not found in PATH. Fetching portable Doxygen ${Z_DOXYGEN_VERSION}...")
        FetchContent_Declare(
            doxygen_bin
            URL "https://www.doxygen.nl/files/doxygen-${Z_DOXYGEN_VERSION}.windows.x64.bin.zip"
            DOWNLOAD_EXTRACT_TIMESTAMP TRUE
        )
        FetchContent_MakeAvailable(doxygen_bin)
        set(DOXYGEN_EXECUTABLE "${doxygen_bin_SOURCE_DIR}/doxygen.exe")
        set(DOXYGEN_FOUND TRUE)
    else()
        message(WARNING "Doxygen is not installed on this system. Documentation target 'docs' will not be available.")
    endif()
endif()

# 2. Поиск системного Graphviz (dot) или автоматическое скачивание portable-версии (Windows)
find_program(DOXYGEN_DOT_EXECUTABLE dot)

if(DOXYGEN_DOT_EXECUTABLE)
    get_filename_component(DOXYGEN_DOT_PATH "${DOXYGEN_DOT_EXECUTABLE}" DIRECTORY)
    set(DOXYGEN_HAVE_DOT "YES")
elseif(WIN32)
    message(STATUS "Graphviz (dot) not found in PATH. Fetching portable Graphviz ${Z_GRAPHVIZ_VERSION}...")
    FetchContent_Declare(
        graphviz_bin
        URL "https://gitlab.com/api/v4/projects/4207231/packages/generic/graphviz-releases/${Z_GRAPHVIZ_VERSION}/windows_10_cmake_Release_Graphviz-${Z_GRAPHVIZ_VERSION}-win64.zip"
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    FetchContent_MakeAvailable(graphviz_bin)
    set(DOXYGEN_DOT_EXECUTABLE "${graphviz_bin_SOURCE_DIR}/bin/dot.exe")
    set(DOXYGEN_DOT_PATH "${graphviz_bin_SOURCE_DIR}/bin")
    set(DOXYGEN_HAVE_DOT "YES")
else()
    set(DOXYGEN_HAVE_DOT "NO")
    set(DOXYGEN_DOT_PATH "")
endif()

# 3. Подгрузка современной CSS-темы doxygen-awesome-css
FetchContent_Declare(
    doxygen_awesome_css
    GIT_REPOSITORY "https://github.com/jothepro/doxygen-awesome-css.git"
    GIT_TAG "${Z_DOXYGEN_AWESOME_VERSION}"
)
FetchContent_MakeAvailable(doxygen_awesome_css)

# 4. Настройка путей и генерация Doxyfile
if(DOXYGEN_FOUND)
    set(DOXYGEN_AWESOME_CSS_DIR "${doxygen_awesome_css_SOURCE_DIR}")
    set(DOXYGEN_INPUT_DIR "\"${CMAKE_SOURCE_DIR}/src\" \"${CMAKE_SOURCE_DIR}/docs\" \"${CMAKE_SOURCE_DIR}/README.md\"")
    set(DOXYGEN_OUTPUT_DIR "${CMAKE_SOURCE_DIR}/.docs")

    configure_file(
        "${CMAKE_CURRENT_LIST_DIR}/Doxyfile.in"
        "${CMAKE_BINARY_DIR}/Doxyfile"
        @ONLY
    )

    add_custom_target(docs
        COMMAND "${DOXYGEN_EXECUTABLE}" "${CMAKE_BINARY_DIR}/Doxyfile"
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        COMMENT "Generating API documentation to ${CMAKE_SOURCE_DIR}/.docs/html"
        VERBATIM
    )
    message(STATUS "Doxygen documentation target 'docs' configured successfully.")
endif()
