#include "pch.h"
export module coroutine_1;

// ќпредел€ем структуру дл€ генератора
export struct Generator
{
	struct promise_type
	{
		int32_t value_;
		std::suspend_always initial_suspend() { return {}; }
		std::suspend_always final_suspend() noexcept { return {}; }
		void return_value(int32_t v) { value_ = v; }
		std::suspend_always yield_value(int32_t v)
		{
			value_ = v;
			return {};
		}
		void unhandled_exception() {}
		Generator get_return_object() { return Generator{ this }; }
		int32_t value() const { return value_; }
	};

	using handle_type = std::coroutine_handle<promise_type>;

	handle_type coro_;

	Generator(promise_type* p) : coro_(handle_type::from_promise(*p)) {}
	Generator(Generator&& other) noexcept : coro_(other.coro_) { other.coro_ = nullptr; }
	~Generator() { if (coro_) coro_.destroy(); }

	int32_t value() const { return coro_.promise().value(); }
	bool resume()
	{
		if (!coro_.done())
		{
			coro_.resume();
			return true;
		}

		return false;
	}
};

// Ёкспортируем минимальную корутину
export Generator simple_coroutine()
{
	co_yield 42; // ¬озвращаем одно значение
	co_return 100; // «авершаем с финальным значением
}