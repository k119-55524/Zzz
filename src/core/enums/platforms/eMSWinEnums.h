#pragma once

namespace zzz::common
{
	enum class eMSWinWindowMode : zU8
	{
		Windowed,            // Оконный режим
		BorderlessFullscreen,// Полноэкранный режим без рамок
		ExclusiveFullscreen  // Эксклюзивный полноэкранный режим
	};

	enum class eMSWinWindowStyle : zU8
	{
		OverlappedWindow,    // Обычное окно с рамкой и заголовком
		PopUp,               // Всплывающее окно без рамок
		ToolWindow           // Компактное окно инструмента
	};
}
