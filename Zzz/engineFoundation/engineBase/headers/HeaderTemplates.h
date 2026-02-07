#pragma once

namespace zzz
{
	// TODO: пересмотреть все вызовы и убрать лишнее в текстовых сообщениях
	template<typename T>
	inline void ensure(T&& condition,
		const std::string& msg = "Ensure failed",
		const std::source_location& loc = std::source_location::current())
	{
		if (!condition)
			throw std::runtime_error(
				">>>>> [" +
				std::string(loc.function_name()) +
				"]. Line: " + std::to_string(loc.line()) +
				", File=: " + std::string(loc.file_name()) +
				"\n\nRoot message:\n    " + msg);
	}

	template<typename T>
	inline void SafeRelease(T*& ptr)
	{
		if (ptr)
		{
			ptr->Release();
			ptr = nullptr;
		}
	}

#pragma region SafeMake
	template<typename Creator, typename... Args>
	inline decltype(auto) safe_make_impl(
		Creator&& creator,
		const std::source_location& loc,
		Args&&... args) noexcept(false)
	{
		try
		{
			return creator(std::forward<Args>(args)...);
		}
		catch (const std::bad_alloc& e)
		{
			std::ostringstream oss;
			oss << "Memory allocation error. Exception: " << e.what()
				<< ". Method=" << loc.function_name()
				<< ", line=" << loc.line()
				<< ", file=" << loc.file_name();
			throw std::runtime_error(oss.str());
		}
	}

	// Безопасный make_shared
	template<typename T, typename... Args>
	inline std::shared_ptr<T> safe_make_shared(Args&&... args) noexcept(false)
	{
		return safe_make_impl(
			[](auto&&... a) { return std::make_shared<T>(std::forward<decltype(a)>(a)...); },
			std::source_location::current(),
			std::forward<Args>(args)...
		);
	}

	// Безопасный make_unique
	template<typename T, typename... Args>
	inline std::unique_ptr<T> safe_make_unique(Args&&... args) noexcept(false)
	{
		return safe_make_impl(
			[](auto&&... a) { return std::make_unique<T>(std::forward<decltype(a)>(a)...); },
			std::source_location::current(),
			std::forward<Args>(args)...
		);
	}
#pragma endregion SafeMake
}