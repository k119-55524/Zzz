#pragma once

#include <string_view>
#include "math/utils/Types.h"
#include "core/utils/ThrowWrappers.h"
namespace zzz::core
{
	enum class eResourceState : zU8
	{
		Unloaded = 0,   ///< Ресурс не загружен
		Loading,        ///< В процессе чтения / парсинга
		Ready,          ///< Ресурс готов к использованию (в RAM / на GPU)
		Failed          ///< Ошибка загрузки
	};

	[[nodiscard]] constexpr std::string_view ToString(eResourceState state)
	{
		switch (state)
		{
		case eResourceState::Unloaded: return "Unloaded";
		case eResourceState::Loading:  return "Loading";
		case eResourceState::Ready:    return "Ready";
		case eResourceState::Failed:   return "Failed";
		}
		THROW_RUNTIME("Необработанный eResourceState");
	}
}
