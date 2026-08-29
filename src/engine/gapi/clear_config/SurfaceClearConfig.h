#pragma once

#include "math/Color.h"
#include "core/utils/Guid.h"
#include "core/Serialize/Serializer.h"
#include <logger/logger.h>

namespace zzz::engine
{
	using namespace zzz::math;
	using namespace zzz::core;

	/**
	 * @enum eSurfaceClearMode
	 * @brief Режим очистки/заливки поверхности рендеринга.
	 */
	enum class eSurfaceClearMode : zU8
	{
		None,   ///< Не чистить (пропустить очистку поверхности)
		Color,  ///< Чистка цветом (Color4<zF32>)
		Shader  ///< Чистка полноэкранным фоновым шейдером
	};

	/**
	 * @class SurfaceClearConfig
	 * @brief Конфигурация очистки поверхности рендеринга (Swapchain / Render Target).
	 */
	class SurfaceClearConfig final : public ISerializable
	{
	public:
		eSurfaceClearMode mode{ eSurfaceClearMode::Color };   ///< Режим очистки поверхности.
		Color4<zF32>      color{ Palette4::Black };            ///< Цвет очистки (zF32 [0.0f..1.0f]).
		Guid              shaderGuid{};                        ///< GUID фонового шейдера (для режима Shader).

		constexpr SurfaceClearConfig() noexcept = default;
		constexpr SurfaceClearConfig(eSurfaceClearMode mode, Color4<zF32> color, Guid shaderGuid = {}) noexcept
			: mode(mode)
			, color(color)
			, shaderGuid(shaderGuid)
		{}

		constexpr bool operator==(const SurfaceClearConfig&) const noexcept = default;

		inline void LogFileBlock(std::string_view indentation = {}) const
		{
#if Z_ADD_LOGGER
			const std::string nestedIndentation = std::string(indentation) + "  ";
			DOut(::zzz::core::GAPI, "{}[SurfaceClearConfig]", indentation);
			DOut(::zzz::core::GAPI, "{}mode: {}", nestedIndentation, mode == eSurfaceClearMode::Color ? "Color" : (mode == eSurfaceClearMode::Shader ? "Shader" : "None"));
			DOut(::zzz::core::GAPI, "{}color: {}", nestedIndentation, color.ToString());
			DOut(::zzz::core::GAPI, "{}shaderGuid: {}", nestedIndentation, shaderGuid.ToString());
#endif
		}

	protected:
		[[nodiscard]] std::expected<void, std::string> Serialize(std::vector<std::byte>& buffer, const Serializer& serializer) const override
		{
			return serializer.Serialize(buffer, mode)
				.and_then([&]() { return serializer.Serialize(buffer, color); })
				.and_then([&]() { return serializer.Serialize(buffer, shaderGuid); });
		}

		[[nodiscard]] std::expected<void, std::string> Deserialize(std::span<const std::byte> buffer, std::size_t& offset, const Serializer& serializer) override
		{
			return serializer.Deserialize(buffer, offset, mode)
				.and_then([&]() { return serializer.Deserialize(buffer, offset, color); })
				.and_then([&]() { return serializer.Deserialize(buffer, offset, shaderGuid); });
		}
	};
}
