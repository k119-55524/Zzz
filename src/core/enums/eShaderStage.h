#pragma once

#include <cstdint>
#include "math/utils/Types.h"

namespace zzz::core
{
	/**
	 * @enum eShaderStage
	 * @brief Битовая маска стадий шейдера.
	 */
	enum class eShaderStage : zU32
	{
		None    = 0,
		Vertex  = 1 << 0, ///< Вершинный шейдер
		Pixel   = 1 << 1, ///< Пиксельный (фрагментный) шейдер
		Compute = 1 << 2  ///< Вычислительный шейдер
	};

	[[nodiscard]] constexpr eShaderStage operator|(eShaderStage lhs, eShaderStage rhs) noexcept
	{
		return static_cast<eShaderStage>(static_cast<zU32>(lhs) | static_cast<zU32>(rhs));
	}

	[[nodiscard]] constexpr eShaderStage operator&(eShaderStage lhs, eShaderStage rhs) noexcept
	{
		return static_cast<eShaderStage>(static_cast<zU32>(lhs) & static_cast<zU32>(rhs));
	}

	constexpr eShaderStage& operator|=(eShaderStage& lhs, eShaderStage rhs) noexcept
	{
		lhs = lhs | rhs;
		return lhs;
	}

	constexpr eShaderStage& operator&=(eShaderStage& lhs, eShaderStage rhs) noexcept
	{
		lhs = lhs & rhs;
		return lhs;
	}

	[[nodiscard]] constexpr bool HasFlag(eShaderStage mask, eShaderStage flag) noexcept
	{
		return (static_cast<zU32>(mask) & static_cast<zU32>(flag)) == static_cast<zU32>(flag);
	}
}
