#pragma once

#include "ShaderBuilderTypes.h"

namespace zzz::shader_builder
{
    /**
     * @brief Основной сервис компиляции, извлечения рефлексии и сборки блоба шейдеров
     */
    class ShaderBuilder
    {
    public:
        ShaderBuilder();
        ~ShaderBuilder();

        /**
         * @brief Компилирует шейдер согласно дескриптору и пакует в готовый бинарный блоб
         */
        ShaderBuildResult Build(const ShaderBuildDescriptor& desc);

        /**
         * @brief Компилирует шейдер из файла на диске
         */
        ShaderBuildResult BuildFromFile(const std::filesystem::path& filePath, core::eShaderBinaryFormat targetFormat);
    };
}
