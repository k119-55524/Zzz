#pragma once

#include "core/utils/Macroses.h"

namespace zzz::engine
{
	/**
	 * @brief Глобальный отладочный флаг интерактивного ресайза окна (перетаскивание рамок мышью).
	 * Включается в true на OnWindowResizeStart и сбрасывается в false на OnWindowResizeEnd.
	 */
	Z_LOG_GLOBAL_VAR(bool, g_IsResizing, false);
}
