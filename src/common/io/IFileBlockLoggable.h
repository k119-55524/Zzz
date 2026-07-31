#pragma once

namespace zzz::io
{
	/// @brief Интерфейс для структур и блоков файлов, поддерживающих логирование состава своего содержимого.
	class IFileBlockLoggable
	{
	public:
		virtual ~IFileBlockLoggable() = default;

		/// @brief Выводит состав и детали текущего блока файла в лог.
		virtual void LogFileBlock() const = 0;
	};
}
