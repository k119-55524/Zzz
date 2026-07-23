add_library(zzz_scripts SHARED
    SampleGameScript.h
    SampleGameScript.cpp
    SampleSceneScript.h
    SampleSceneScript.cpp
    SampleViewScript.h
    SampleViewScript.cpp
    SampleObjectScript.h
    SampleObjectScript.cpp
    RegisterAllScripts.cpp
)

target_include_directories(zzz_scripts PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    "${CMAKE_SOURCE_DIR}/src/engine"
    "${CMAKE_SOURCE_DIR}/src/engine/public/core/scene"
    "${CMAKE_SOURCE_DIR}/src/engine/public/core/userscripts"
    "${CMAKE_SOURCE_DIR}/src/engine/public/core/userscripts/base_script"
)

target_link_libraries(zzz_scripts PRIVATE
    engine_lib
    logger_lib
    common_lib
)

target_compile_features(zzz_scripts PUBLIC cxx_std_23)
target_compile_definitions(zzz_scripts PRIVATE Z_TARGET_NAME="zzz_scripts")
