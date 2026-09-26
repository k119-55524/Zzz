#pragma once

#include <cstdint>
#include <string_view>
#include "math/utils/Types.h"

namespace zzz::core
{
	/**
	 * @enum eMeshStreamFlags
	 * @brief Битовая маска независимых потоков вершинных данных меша (Slot-Based Multi-Stream).
	 */
	enum class eMeshStreamFlags : zU32
	{
		None          = 0,
		Stream0_Base  = 1 << 0, ///< Базовый стрим: Position(12b) + Normal(12b) + Tangent(16b) + UV0(8b) = 48b
		Stream1_Skin  = 1 << 1, ///< Стрим скелетки: BoneIndices + BoneWeights
		Stream2_Color = 1 << 2, ///< Стрим цвета вершин: Color (RGBA8)
		Stream3_UV1   = 1 << 3  ///< Стрим вторичной развёртки: UV1 (float2)
	};

	[[nodiscard]] constexpr eMeshStreamFlags operator|(eMeshStreamFlags lhs, eMeshStreamFlags rhs) noexcept
	{
		return static_cast<eMeshStreamFlags>(static_cast<zU32>(lhs) | static_cast<zU32>(rhs));
	}

	[[nodiscard]] constexpr eMeshStreamFlags operator&(eMeshStreamFlags lhs, eMeshStreamFlags rhs) noexcept
	{
		return static_cast<eMeshStreamFlags>(static_cast<zU32>(lhs) & static_cast<zU32>(rhs));
	}

	constexpr eMeshStreamFlags& operator|=(eMeshStreamFlags& lhs, eMeshStreamFlags rhs) noexcept
	{
		lhs = lhs | rhs;
		return lhs;
	}

	constexpr eMeshStreamFlags& operator&=(eMeshStreamFlags& lhs, eMeshStreamFlags rhs) noexcept
	{
		lhs = lhs & rhs;
		return lhs;
	}

	[[nodiscard]] constexpr bool HasFlag(eMeshStreamFlags mask, eMeshStreamFlags flag) noexcept
	{
		return (static_cast<zU32>(mask) & static_cast<zU32>(flag)) == static_cast<zU32>(flag);
	}
}
