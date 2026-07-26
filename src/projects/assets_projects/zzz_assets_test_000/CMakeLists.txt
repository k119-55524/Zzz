add_library(zzz_assets_test_000 SHARED
    Scripts/SampleGameScript.h
    Scripts/SampleGameScript.cpp
    Scripts/SampleSceneScript.h
    Scripts/SampleSceneScript.cpp
    Scripts/SampleViewScript.h
    Scripts/SampleViewScript.cpp
    Scripts/SampleObjectScript.h
    Scripts/SampleObjectScript.cpp
    RegisterAllScripts.cpp
)

target_include_directories(zzz_assets_test_000 PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/Scripts
    "${CMAKE_SOURCE_DIR}/src/engine"
    "${CMAKE_SOURCE_DIR}/src/engine/public/core/scene"
    "${CMAKE_SOURCE_DIR}/src/engine/public/core/userscripts"
    "${CMAKE_SOURCE_DIR}/src/engine/public/core/userscripts/base_script"
)

target_link_libraries(zzz_assets_test_000 PRIVATE
    engine_lib
    logger_lib
    common_lib
)

target_compile_features(zzz_assets_test_000 PUBLIC cxx_std_23)
target_compile_definitions(zzz_assets_test_000 PRIVATE Z_TARGET_NAME="zzz_assets_test_000")

add_custom_command(TARGET zzz_assets_test_000 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/Assets"
        "${CMAKE_BINARY_DIR}/bin/$<CONFIG>/Assets"
)
