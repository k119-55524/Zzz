# Скрипт для рекурсивного копирования только .h файлов
# Ожидает переменные:
# SRC_DIR - исходная папка
# DST_DIR - целевая папка

if(NOT DEFINED SRC_DIR OR NOT DEFINED DST_DIR)
    message(FATAL_ERROR "SRC_DIR or DST_DIR is not defined!")
endif()

file(GLOB_RECURSE HEADERS RELATIVE "${SRC_DIR}" "${SRC_DIR}/*.h")
foreach(HEADER ${HEADERS})
    # Игнорируем приватные хедеры и предкомпилированные хедеры
    if(NOT "${HEADER}" MATCHES "private/" AND NOT "${HEADER}" MATCHES "pch/")
        configure_file("${SRC_DIR}/${HEADER}" "${DST_DIR}/${HEADER}" COPYONLY)
    endif()
endforeach()
