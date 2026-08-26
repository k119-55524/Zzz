#pragma once

#include "SurfaceClearConfig.h"
#include "DepthBufferClearConfig.h"

namespace zzz::engine
{
	using namespace zzz::core;

	/**
	 * @class ViewClearConfig
	 * @brief Агрегированная конфигурация очистки для View, объединяющая настройки поверхности рендеринга и буфера глубины.
	 */
	class ViewClearConfig final : public ISerializable
	{
	public:
		SurfaceClearConfig     surface;     ///< Настройки очистки поверхности цвета (Swapchain).
		DepthBufferClearConfig depthBuffer; ///< Настройки очистки буфера глубины и трафарета (DepthBuffer).

		constexpr ViewClearConfig() noexcept = default;
		constexpr ViewClearConfig(SurfaceClearConfig surface, DepthBufferClearConfig depthBuffer) noexcept
			: surface(std::move(surface))
			, depthBuffer(std::move(depthBuffer))
		{}

		constexpr bool operator==(const ViewClearConfig&) const noexcept = default;

		inline void LogFileBlock(std::string_view indentation = {}) const
		{
#if Z_ADD_LOGGER || Z_DEVELOPMENT_BUILD
			const std::string nestedIndentation = std::string(indentation) + "  ";
			DOut("{}[ViewClearConfig]", indentation);
			surface.LogFileBlock(nestedIndentation);
			depthBuffer.LogFileBlock(nestedIndentation);
#endif
		}

	protected:
		[[nodiscard]] std::expected<void, std::string> Serialize(std::vector<std::byte>& buffer, const Serializer& serializer) const override
		{
			return serializer.Serialize(buffer, surface)
				.and_then([&]() { return serializer.Serialize(buffer, depthBuffer); });
		}

		[[nodiscard]] std::expected<void, std::string> Deserialize(std::span<const std::byte> buffer, std::size_t& offset, const Serializer& serializer) override
		{
			return serializer.Deserialize(buffer, offset, surface)
				.and_then([&]() { return serializer.Deserialize(buffer, offset, depthBuffer); });
		}
	};
}
