add_library(assets_builder_dll SHARED
    BuilderExport.h
    BuilderApi.h
    BuilderApi.cpp
    PackagePacker.h
    PackagePacker.cpp
    IAssetImporter.h
    AssetImporterRegistry.h
    AssetImporterRegistry.cpp
    ObjImporter.h
    ObjImporter.cpp
)

target_include_directories(assets_builder_dll PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    "${CMAKE_SOURCE_DIR}/src"
    "${CMAKE_SOURCE_DIR}/src/engine"
)

target_link_libraries(assets_builder_dll PRIVATE
    core_lib
    logger_lib
)

target_compile_definitions(assets_builder_dll PRIVATE ASSETS_BUILDER_EXPORTS Z_TARGET_NAME="assets_builder_dll")
target_compile_features(assets_builder_dll PUBLIC cxx_std_23)
