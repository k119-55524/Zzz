module;

#include "pch.h"

module zzz.logger;

namespace zzz::logger
{
	void Logger::Info(std::string_view message)
	{
		std::cout << "[Info] " << message << '\n';
	}

	void Logger::Warning(std::string_view message)
	{
		std::cout << "[Warning] " << message << '\n';
	}

	void Logger::Error(std::string_view message)
	{
		std::cout << "[Error] " << message << '\n';
	}
}