#pragma once

#include <string_view>
#include "core/utils/ThrowWrappers.h"
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

	constexpr std::string_view ToString(eClearDepthMode mode)
	{
		switch (mode)
		{
		case eClearDepthMode::None:  return "None";
		case eClearDepthMode::Depth: return "Depth";
		}
		THROW_RUNTIME("Необработанный eClearDepthMode");
	}

	/**
	 * @enum eClearStencilMode
	 * @brief Режим очистки трафарета.
	 */
	enum class eClearStencilMode : zU8
	{
		None,    ///< Не чистить трафарет
		Stencil  ///< Чистка трафарета (uint8)
	};

	constexpr std::string_view ToString(eClearStencilMode mode)
	{
		switch (mode)
		{
		case eClearStencilMode::None:    return "None";
		case eClearStencilMode::Stencil: return "Stencil";
		}
		THROW_RUNTIME("Необработанный eClearStencilMode");
	}

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

		inline void LogFileBlock([[maybe_unused]] std::string_view indentation = {}) const
		{
#if Z_ADD_LOGGER
			const std::string nestedIndentation = std::string(indentation) + "  ";
			DOut(::zzz::core::GAPI, "{}[DepthBufferClearConfig]", indentation);
			DOut(::zzz::core::GAPI, "{}depthMode: {}", nestedIndentation, ToString(depthMode));
			DOut(::zzz::core::GAPI, "{}depth: {}", nestedIndentation, depth);
			DOut(::zzz::core::GAPI, "{}stencilMode: {}", nestedIndentation, ToString(stencilMode));
			DOut(::zzz::core::GAPI, "{}stencil: {}", nestedIndentation, stencil);
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
