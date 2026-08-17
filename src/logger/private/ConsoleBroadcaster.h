#pragma once

#include "IBroadcaster.h"

#if Z_WINDOWS
#include <format>
#include <chrono>
#include <iostream>
#include <stdio.h>

namespace zzz::logger
{
	using namespace zzz::core;
	class ConsoleBroadcaster final : public IBroadcaster
	{
	public:
		ConsoleBroadcaster()
		{
			// Создаем консоль для вывода, если ее еще нет
			if (AllocConsole())
			{
				FILE* dummy;
				freopen_s(&dummy, "CONOUT$", "w", stdout);
				freopen_s(&dummy, "CONOUT$", "w", stderr);
				freopen_s(&dummy, "CONIN$", "r", stdin);
				
				// Поддержка русского языка (UTF-8)
				SetConsoleOutputCP(CP_UTF8);
				SetConsoleCP(CP_UTF8);

				m_ConsoleCreated = true;
			}
		}

		~ConsoleBroadcaster()
		{
			// Освобождаем консоль, если мы ее создали
			if (m_ConsoleCreated)
				FreeConsole();
		}

		void OnLog(const LogEntry& entry) override
		{
			using eType = eLogMessageType;

			const bool isWarning = (entry.type & eType::Warning) != eType::None;
			const bool isError   = (entry.type & (eType::Error | eType::Exception | eType::Critical | eType::Fatal)) != eType::None;

			HANDLE hConsole = GetStdHandle(isError ? STD_ERROR_HANDLE : STD_OUTPUT_HANDLE);
			WORD originalAttrs = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;

			CONSOLE_SCREEN_BUFFER_INFO csbi;
			if (GetConsoleScreenBufferInfo(hConsole, &csbi))
			{
				originalAttrs = csbi.wAttributes;
			}

			WORD colorAttribute = originalAttrs & ~(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);

			if (isError)
			{
				colorAttribute |= FOREGROUND_RED | FOREGROUND_INTENSITY;
			}
			else if (isWarning)
			{
				colorAttribute |= FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
			}
			else
			{
				colorAttribute |= FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE;
			}
			
			SetConsoleTextAttribute(hConsole, colorAttribute);

			// Форматируем время
			auto timePt = std::chrono::time_point<std::chrono::system_clock>(std::chrono::milliseconds(entry.timestamp));
			auto in_time_t = std::chrono::system_clock::to_time_t(timePt);
			std::tm bt{};

			localtime_s(&bt, &in_time_t);
			auto ms = entry.timestamp % 1000;
			std::string timeStr = std::format("[{:02}ч {:02}м {:02}с {:03}мс]", bt.tm_hour, bt.tm_min, bt.tm_sec, ms);

			std::string output;
			if (isError)
				output = std::format("{} [{}] {} -> [{}] {}:{}\n", timeStr, EnumToString::ToString(entry.type), entry.text, entry.function, entry.file, entry.line);
			else
				output = std::format("{} [{}] {}\n", timeStr, EnumToString::ToString(entry.type), entry.text);

			auto& stream = isError ? std::cerr : std::cout;
			stream << output;
			stream.flush();

			// Возвращаем исходный цвет консоли
			SetConsoleTextAttribute(hConsole, originalAttrs);
		}

	private:
		bool m_ConsoleCreated = false;
	};
}
#endif

