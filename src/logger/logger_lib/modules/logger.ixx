module;

export module logger;

using namespace std;

export namespace zzz::logger
{
	class Logger
	{
	public:
		template <typename... Args>
			static void DebugOutput(
				const std::source_location& loc,
				std::wstring_view fmt,
				Args&&... args)
		{
#if ZZZ_ENABLE_LOGGER
			auto formatted = std::vformat(fmt, std::make_wformat_args(std::forward<Args>(args)...));
			auto output = MakeDebugOutputString(loc, formatted);
			DebugOutputRaw(output);
#endif // #if ZZZ_ENABLE_LOGGER
		}

	private:
		static std::wstring MakeDebugOutputString(const std::source_location& loc, const std::wstring& msg);
		static void DebugOutputRaw(const std::wstring& output) noexcept;
	};
}