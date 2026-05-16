module;

export module zzz.logger;

export namespace zzz::logger
{
	class Logger
	{
	public:
		void Info(std::string_view message);

		void Warning(std::string_view message);

		void Error(std::string_view message);
	};
}