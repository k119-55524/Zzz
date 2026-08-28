#pragma once

#include "core/Serialize/Serializer.h"
#include <logger/logger.h>

namespace zzz::engine
{
	using namespace zzz::core;

	/**
	 * @enum eClearDepthMode
	 * @brief Режим очистки глубины.
	 */
	enum class eClearDepthMode : zU8
	{
		None,  ///< Не чистить глубину
		Depth  ///< Чистка глубины (float)
	};

	/**
	 * @enum eClearStencilMode
	 * @brief Режим очистки трафарета.
	 */
	enum class eClearStencilMode : zU8
	{
		None,    ///< Не чистить трафарет
		Stencil  ///< Чистка трафарета (uint8)
	};

	/**
	 * @class DepthBufferClearConfig
	 * @brief Конфигурация очистки буфера глубины и трафарета (DepthBuffer).
	 */
	class DepthBufferClearConfig final : public ISerializable
	{
	public:
		eClearDepthMode   depthMode{ eClearDepthMode::Depth };     ///< Режим очистки глубины.
		zF32              depth{ 1.0f };                          ///< Значение глубины [0.0f..1.0f].
		eClearStencilMode stencilMode{ eClearStencilMode::None }; ///< Режим очистки трафарета.
		zU8               stencil{ 0 };                           ///< Значение трафарета [0..255].

		constexpr DepthBufferClearConfig() noexcept = default;
		constexpr DepthBufferClearConfig(eClearDepthMode depthMode, zF32 depth = 1.0f, eClearStencilMode stencilMode = eClearStencilMode::None, zU8 stencil = 0) noexcept
			: depthMode(depthMode)
			, depth(depth)
			, stencilMode(stencilMode)
			, stencil(stencil)
		{}

		constexpr bool operator==(const DepthBufferClearConfig&) const noexcept = default;

		inline void LogFileBlock(std::string_view indentation = {}) const
		{
#if Z_ADD_LOGGER
			const std::string nestedIndentation = std::string(indentation) + "  ";
			DOut("{}[DepthBufferClearConfig]", indentation);
			DOut("{}depthMode: {}", nestedIndentation, depthMode == eClearDepthMode::Depth ? "Depth" : "None");
			DOut("{}depth: {}", nestedIndentation, depth);
			DOut("{}stencilMode: {}", nestedIndentation, stencilMode == eClearStencilMode::Stencil ? "Stencil" : "None");
			DOut("{}stencil: {}", nestedIndentation, stencil);
#endif
		}

	protected:
		[[nodiscard]] std::expected<void, std::string> Serialize(std::vector<std::byte>& buffer, const Serializer& serializer) const override
		{
			return serializer.Serialize(buffer, depthMode)
				.and_then([&]() { return serializer.Serialize(buffer, depth); })
				.and_then([&]() { return serializer.Serialize(buffer, stencilMode); })
				.and_then([&]() { return serializer.Serialize(buffer, stencil); });
		}

		[[nodiscard]] std::expected<void, std::string> Deserialize(std::span<const std::byte> buffer, std::size_t& offset, const Serializer& serializer) override
		{
			return serializer.Deserialize(buffer, offset, depthMode)
				.and_then([&]() { return serializer.Deserialize(buffer, offset, depth); })
				.and_then([&]() { return serializer.Deserialize(buffer, offset, stencilMode); })
				.and_then([&]() { return serializer.Deserialize(buffer, offset, stencil); });
		}
	};
}
