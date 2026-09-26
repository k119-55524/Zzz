#pragma once

#include "pch.h"

namespace zzz::shader_builder
{
    /**
     * @brief Входной дескриптор для компиляции шейдера
     */
    struct ShaderBuildDescriptor
    {
        std::string sourceCode;                  ///< Исходный HLSL код или путь к файлу
        std::string sourceFilePath;              ///< Путь к файлу (для диагностических сообщений)
        std::string vsEntryPoint = "VSMain";     ///< Точка входа вершинного шейдера
        std::string psEntryPoint = "PSMain";     ///< Точка входа пиксельного шейдера
        std::vector<std::string> defines;        ///< Список дефайнов (#define)
        core::eShaderBinaryFormat targetFormat = core::eShaderBinaryFormat::DXIL; ///< Целевой формат байткода
    };

    /**
     * @brief Результат компиляции и сборки шейдера
     */
    struct ShaderBuildResult
    {
        bool success = false;
        std::string errorMessage;
        uint32_t requiredStreamsMask = 0;        ///< Битовая маска eMeshStreamFlags
        std::vector<uint8_t> vsBytecode;         ///< Байткод вершинного шейдера
        std::vector<uint8_t> psBytecode;         ///< Байткод пиксельного шейдера
        std::vector<uint8_t> fullBlob;           ///< Полный готовый бинарный блоб для data.dat
    };
}
