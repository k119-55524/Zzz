#pragma once

#include "IBroadcaster.h"

#if Z_WINDOWS && (Z_ADD_LOGGER || Z_DEVELOPMENT_BUILD)
#include <format>
#include <chrono>
#include <iostream>
#include <common/common.h>

#define WIN32_LEAN_AND_MEAN
#include <stdio.h>
#include <Windows.h>

namespace zzz::logger
{
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

				HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
				if (hStdOut != INVALID_HANDLE_VALUE)
				{
					// Устанавливаем жирный шрифт (Consolas)
					CONSOLE_FONT_INFOEX cfi{};
					cfi.cbSize = sizeof(cfi);
					// Сначала получаем текущий шрифт, чтобы не затереть системные параметры
					if (GetCurrentConsoleFontEx(hStdOut, FALSE, &cfi))
					{
						cfi.dwFontSize.X = 0;
						cfi.dwFontSize.Y = 16; // Высота шрифта
						cfi.FontFamily = FF_DONTCARE;
						cfi.FontWeight = 900; // FW_HEAVY (самый толстый вариант, толще чем FW_BOLD)
						wcscpy_s(cfi.FaceName, L"Consolas");
						SetCurrentConsoleFontEx(hStdOut, FALSE, &cfi);
					}

					CONSOLE_SCREEN_BUFFER_INFO csbi;
					GetConsoleScreenBufferInfo(hStdOut, &csbi);

					// Расширяем консоль только в ширину (высоту не трогаем)
					short consoleWidth = 140;
					
					// Увеличиваем буфер
					COORD bufferSize = csbi.dwSize;
					bufferSize.X = consoleWidth;
					SetConsoleScreenBufferSize(hStdOut, bufferSize);
					
					// Увеличиваем окно
					SMALL_RECT windowSize = csbi.srWindow;
					windowSize.Right = windowSize.Left + consoleWidth - 1;
					SetConsoleWindowInfo(hStdOut, TRUE, &windowSize);

					// Подменяем палитру консоли, чтобы "синий" был настоящим олдскульным DarkBlue,
					// так как в современных Windows 10/11 стандартный синий слишком яркий.
					CONSOLE_SCREEN_BUFFER_INFOEX infoEx;
					infoEx.cbSize = sizeof(CONSOLE_SCREEN_BUFFER_INFOEX);
					if (GetConsoleScreenBufferInfoEx(hStdOut, &infoEx))
					{
						infoEx.ColorTable[1] = RGB(0, 0, 139);     // DarkBlue (фон)
						infoEx.ColorTable[2] = RGB(0, 170, 0);     // DarkGreen (сообщения)
						infoEx.ColorTable[4] = RGB(170, 0, 0);     // DarkRed (ошибки)
						infoEx.ColorTable[6] = RGB(220, 220, 0);   // Yellow (ворнинги)
						infoEx.ColorTable[7] = RGB(192, 192, 192); // LightGray (шапка)
						SetConsoleScreenBufferInfoEx(hStdOut, &infoEx);
					}

					// Устанавливаем синий фон для всего окна консоли
					if (GetConsoleScreenBufferInfo(hStdOut, &csbi))
					{
						DWORD written;
						COORD coord = {0, 0};
						DWORD size = csbi.dwSize.X * csbi.dwSize.Y;
						// Классический темно-синий фон с серым текстом
						WORD defaultColor = BACKGROUND_BLUE | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
						FillConsoleOutputCharacterA(hStdOut, ' ', size, coord, &written);
						FillConsoleOutputAttribute(hStdOut, defaultColor, size, coord, &written);
						SetConsoleTextAttribute(hStdOut, defaultColor);
					}
					
					// Вывод шапки
					std::cout << ">>>>> Start ZzzEngine\n";
					std::cout.flush();
				}
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
			using eType = zzz::common::eLogMessageType;

			const bool isWarning = !!(entry.type & eType::Warning);
			const bool isError = !!(entry.type & (eType::Error | eType::Exception | eType::Critical | eType::Fatal));

			HANDLE hConsole = GetStdHandle(isError ? STD_ERROR_HANDLE : STD_OUTPUT_HANDLE);
			WORD colorAttribute = BACKGROUND_BLUE; // Синий фон

			if (isError)
			{
				colorAttribute |= FOREGROUND_RED; // Темно-красный текст
			}
			else if (isWarning)
			{
				colorAttribute |= FOREGROUND_RED | FOREGROUND_GREEN; // Темно-желтый текст
			}
			else
			{
				colorAttribute |= FOREGROUND_GREEN; // Темно-зеленый текст
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
				output = std::format("{} [{}] {} -> [{}] {}:{}\n", timeStr, zzz::common::EnumToString::ToString(entry.type), entry.text, entry.function, entry.file, entry.line);
			else
				output = std::format("{} [{}] {}\n", timeStr, zzz::common::EnumToString::ToString(entry.type), entry.text);

			auto& stream = isError ? std::cerr : std::cout;
			stream << output;
			stream.flush(); // Обязательно сбрасываем буфер до смены цвета обратно

			// Возвращаем стандартный цвет (серый на синем)
			SetConsoleTextAttribute(hConsole, BACKGROUND_BLUE | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
		}

	private:
		bool m_ConsoleCreated = false;
	};
}
#endif
