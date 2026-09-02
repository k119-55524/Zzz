#pragma once

#include <string_view>
#include "math/utils/Types.h"
#include "core/utils/ThrowWrappers.h"

namespace zzz::core
{
	enum class eMSWinWindowMode : zU8
	{
		Windowed,            // Оконный режим
		BorderlessFullscreen,// Полноэкранный режим без рамок
		ExclusiveFullscreen  // Эксклюзивный полноэкранный режим
	};

	constexpr std::string_view ToString(eMSWinWindowMode type)
	{
		switch (type)
		{
		case eMSWinWindowMode::Windowed:            return "Windowed";
		case eMSWinWindowMode::BorderlessFullscreen:return "BorderlessFullscreen";
		case eMSWinWindowMode::ExclusiveFullscreen: return "ExclusiveFullscreen";
		}
		THROW_RUNTIME("Необработанный eMSWinWindowMode");
	}

	enum class eMSWinWindowStyle : zU8
	{
		OverlappedWindow,    // Обычное окно с рамкой и заголовком
		PopUp,               // Всплывающее окно без рамок
		ToolWindow           // Компактное окно инструмента
	};

	constexpr std::string_view ToString(eMSWinWindowStyle type)
	{
		switch (type)
		{
		case eMSWinWindowStyle::OverlappedWindow: return "OverlappedWindow";
		case eMSWinWindowStyle::PopUp:            return "PopUp";
		case eMSWinWindowStyle::ToolWindow:       return "ToolWindow";
		}
		THROW_RUNTIME("Необработанный eMSWinWindowStyle");
	}
}
