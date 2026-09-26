#include "pch.h"
#include "ShaderBuilder.h"

namespace zzz::shader_builder
{
    ShaderBuilder::ShaderBuilder() = default;
    ShaderBuilder::~ShaderBuilder() = default;

    ShaderBuildResult ShaderBuilder::Build(const ShaderBuildDescriptor& desc)
    {
        ShaderBuildResult result;
        result.success = false;
        result.errorMessage = "Not implemented yet";
        return result;
    }

    ShaderBuildResult ShaderBuilder::BuildFromFile(const std::filesystem::path& filePath, core::eShaderBinaryFormat targetFormat)
    {
        ShaderBuildDescriptor desc;
        desc.sourceFilePath = filePath.string();
        desc.targetFormat = targetFormat;
        return Build(desc);
    }
}
