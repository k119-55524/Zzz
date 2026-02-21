#pragma once

namespace zzz
{
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