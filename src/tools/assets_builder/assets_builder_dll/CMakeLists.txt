add_library(assets_builder_dll SHARED
    BuilderExport.h
    BuilderApi.h
    BuilderApi.cpp
    AssetExtensions.h
    AssetScanner.h
    AssetScanner.cpp
    PackagePacker.h
    PackagePacker.cpp
    ArchiveWriter.h
    ArchiveWriter.cpp
    IAssetImporter.h
    AssetImporterRegistry.h
    AssetImporterRegistry.cpp
    AssetImportPipeline.h
    AssetImportPipeline.cpp
    ObjImporter.h
    ObjImporter.cpp
    MaterialImporter.h
    MaterialImporter.cpp
    ShaderImporter.h
    ShaderImporter.cpp
    PrefabImporter.h
    ProjectIdentityValidator.h
    ProjectIdentityValidator.cpp
    importers/TextureImporter.h
    importers/TextureImporter.cpp
    importers/AudioImporter.h
    importers/AudioImporter.cpp
    stages/StageValidationResult.h
    stages/StagePackPlan.h
    stages/Stage1_ProjectValidator.h
    stages/Stage1_ProjectValidator.cpp
    stages/Stage2_PackPlanner.h
    stages/Stage2_PackPlanner.cpp
    stages/Stage3_PakBuilder.h
    stages/Stage3_PakBuilder.cpp
    stages/pak_builders/IPakBuilder.h
    stages/pak_builders/StandardExternalPakBuilder.h
    stages/pak_builders/StandardExternalPakBuilder.cpp
    stages/pak_builders/InlineDataDatBuilder.h
    stages/pak_builders/InlineDataDatBuilder.cpp
    stages/pak_builders/PackageDatBuilder.h
    stages/pak_builders/PackageDatBuilder.cpp
    stages/validation/BuiltPackageValidator.h
    stages/validation/BuiltPackageValidator.cpp
)

target_include_directories(assets_builder_dll PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    "${CMAKE_SOURCE_DIR}/src"
    "${CMAKE_SOURCE_DIR}/src/engine"
)

target_link_libraries(assets_builder_dll PRIVATE
    core_lib
    texture_builder
)

target_compile_definitions(assets_builder_dll PRIVATE ASSETS_BUILDER_EXPORTS Z_TARGET_NAME="assets_builder_dll")
target_compile_features(assets_builder_dll PUBLIC cxx_std_23)

