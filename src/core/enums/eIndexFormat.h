#pragma once

#include "math/utils/Types.h"

namespace zzz::core
{
	/**
	 * @enum eIndexFormat
	 * @brief Формат и разрядность индексов в индексных буферах IIndexBuffer (1 байт).
	 */
	enum class eIndexFormat : zU8
	{
		UInt16 = 0, ///< 2 байта (zU16): до 65 535 вершин — идеально для куба, UI и базовых мешей (экономит 50% VRAM)
		UInt32 = 1  ///< 4 байта (zU32): для высокополигональных мешей (> 65k вершин)
	};

	namespace IndexFormatUtils
	{
		/// @brief Возвращает размер одного индекса в байтах.
		[[nodiscard]] constexpr zU32 GetIndexFormatBytes(eIndexFormat format) noexcept
		{
			return (format == eIndexFormat::UInt16) ? sizeof(zU16) : sizeof(zU32);
		}
	}
}
