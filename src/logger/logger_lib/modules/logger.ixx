module;

export module zlogger;

export namespace zlogger
{
	class Logger
	{
	public:
		template <typename... Args>
			inline void DebugOutput(const std::source_location& loc, std::wstring_view fmt, Args&&... args)
		{
#if defined(ZZZ_ENABLE_LOGGER)
			auto formatted = std::vformat(fmt, std::make_wformat_args(std::forward<Args>(args)...));
			auto output = MakeDebugOutputString(loc, formatted);
			DebugOutputRaw(output);
#endif // #if defined(ZZZ_ENABLE_LOGGER)
		}

	private:
		std::wstring MakeDebugOutputString(const std::source_location& loc, const std::wstring& msg);
		void DebugOutputRaw(const std::wstring& output) noexcept;
	};
}