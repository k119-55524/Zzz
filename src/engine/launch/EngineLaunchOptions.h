#pragma once

#include "core/CoreIncludes.h"
#include <string>
#include <string_view>
#include <span>

namespace zzz::engine
{
	/**
	 * @struct EngineLaunchOptions
	 * @brief Параметры запуска движка, переданные через аргументы командной строки платформы.
	 */
	struct EngineLaunchOptions
	{
		uint32_t threadCap{ 0 }; // 0 = Auto (бюджет определяется PlatformTaskPolicy по топологии)

		[[nodiscard]] static EngineLaunchOptions Parse(int argc, const char* const* argv)
		{
			EngineLaunchOptions options;
			for (int i = 1; i < argc; ++i)
			{
				if (!argv[i])
					continue;
				std::string_view arg(argv[i]);
				ParseArg(options, arg);
			}
			return options;
		}

		[[nodiscard]] static EngineLaunchOptions ParseWindowsCommandLine(std::wstring_view cmdLine)
		{
			EngineLaunchOptions options;
			if (cmdLine.empty())
				return options;

			std::string utf8Line;
			utf8Line.reserve(cmdLine.size());
			for (wchar_t wc : cmdLine)
			{
				if (wc < 128)
					utf8Line.push_back(static_cast<char>(wc));
				else
					utf8Line.push_back('?');
			}

			size_t start = 0;
			while (start < utf8Line.size())
			{
				while (start < utf8Line.size() && (utf8Line[start] == ' ' || utf8Line[start] == '\t'))
					start++;
				if (start >= utf8Line.size())
					break;

				size_t end = start;
				while (end < utf8Line.size() && utf8Line[end] != ' ' && utf8Line[end] != '\t')
					end++;

				std::string_view token(utf8Line.data() + start, end - start);
				ParseArg(options, token);
				start = end;
			}

			return options;
		}

	private:
		static void ParseArg(EngineLaunchOptions& options, std::string_view arg)
		{
			constexpr std::string_view prefix = "--threads=";
			if (arg.starts_with(prefix))
			{
				std::string_view valStr = arg.substr(prefix.size());
				try
				{
					int val = std::stoi(std::string(valStr));
					if (val >= 0)
						options.threadCap = static_cast<uint32_t>(val);
				}
				catch (...)
				{
				}
			}
		}
	};
}
