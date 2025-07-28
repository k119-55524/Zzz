#include "pch.h"
export module result;

export namespace zzz::result
{
	class bad_expected_access : public std::exception
	{
	public:
		bad_expected_access(const char* msg) noexcept : _msg(msg) {}
		const char* what() const noexcept override { return _msg; }

	private:
		const char* _msg;
	};

	enum class eResult : unsigned int
	{
		success = 0,
		failure,
		exception,
		invalid_argument,
		out_of_memory,
		buffer_too_small
	};

	class Unexpected
	{
	public:
		Unexpected() noexcept : code(eResult::success), message() {}
		Unexpected(eResult code) noexcept : code(code), message() {}
		Unexpected(eResult code, const std::wstring& message) noexcept : code(code), message(message) {}

		inline eResult getCode() const noexcept { return code; }
		inline const std::wstring& getMessage() const noexcept { return message; }

		inline bool operator==(const Unexpected& other) const noexcept { return code == other.code && message == other.message; }
		inline bool operator!=(const Unexpected& other) const noexcept { return !(*this == other); }

	private:
		eResult code;
		std::wstring message;
	};

	// Основной шаблон zResult<T>
	template<typename _Ty = void>
	class zResult
	{
	public:
		static_assert(std::is_copy_constructible_v<_Ty>, ">>>>> [class result]. Type _Ty must be copy constructible.");
		static_assert(std::is_move_constructible_v<_Ty>, ">>>>> [class result]. Type _Ty must be move constructible.");

		zResult(const _Ty& value) noexcept(std::is_nothrow_copy_constructible_v<_Ty>) : data(value) {}
		zResult(_Ty&& value) noexcept(std::is_nothrow_move_constructible_v<_Ty>) : data(std::move(value)) {}
		zResult(const Unexpected& error) noexcept : data(error) {}
		zResult(Unexpected&& error) noexcept : data(std::move(error)) {}

		inline bool has_value() const noexcept { return std::holds_alternative<_Ty>(data); }

		_Ty& value()
		{
			assert(has_value() && ">>>>> [_Ty& result.value()]. Attempt to access value of an error.");
			return std::get<_Ty>(data);
		}

		const _Ty& value() const
		{
			assert(has_value() && ">>>>> [const _Ty& result.value()]. Attempt to access value of an error.");
			return std::get<_Ty>(data);
		}

		Unexpected& error()
		{
			assert(!has_value() && ">>>>> [Unexpected& result.error()]. Attempt to access error of a value.");
			return std::get<Unexpected>(data);
		}

		const Unexpected& error() const
		{
			assert(!has_value() && ">>>>> [const Unexpected& result.error()]. Attempt to access error of a value.");
			return std::get<Unexpected>(data);
		}

		bool operator==(const zResult& other) const noexcept { return data == other.data; }
		bool operator!=(const zResult& other) const noexcept { return !(*this == other); }

		_Ty& operator*() { return value(); }
		const _Ty& operator*() const { return value(); }

		_Ty* operator->() { return &value(); }
		const _Ty* operator->() const { return &value(); }

		explicit operator bool() const noexcept { return has_value(); }

		template<typename U>
		_Ty value_or(U&& default_value) const noexcept(std::is_nothrow_constructible_v<_Ty, U>)
		{
			static_assert(std::is_convertible_v<U, _Ty>, ">>>>> [result.value_or(U&& default_value)]. U must be convertible to _Ty");
			return has_value() ? std::get<_Ty>(data) : static_cast<_Ty>(std::forward<U>(default_value));
		}

		template<typename Func>
		auto and_then(Func&& func) const -> std::invoke_result_t<Func, const _Ty&>
		{
			if (has_value())
				return std::invoke(std::forward<Func>(func), value());

			return std::invoke_result_t<Func, const _Ty&>(error());
		}

		template<typename Func>
		auto and_then(Func&& func) const -> zResult<_Ty>
			requires std::is_same_v<std::invoke_result_t<Func, const _Ty&>, void>
		{
			if (has_value())
			{
				std::invoke(std::forward<Func>(func), value());
				return *this;
			}
			return zResult<_Ty>(error());
		}

		template<typename Func>
		auto or_else(Func&& func) const -> std::invoke_result_t<Func, const Unexpected&>
		{
			if (!has_value())
				return std::invoke(std::forward<Func>(func), error());

			return std::invoke_result_t<Func, const Unexpected&>(value());
		}

		template<typename Func>
		auto or_else(Func&& func) const -> zResult<_Ty>
			requires std::is_same_v<std::invoke_result_t<Func, const Unexpected&>, void>
		{
			if (!has_value())
			{
				std::invoke(std::forward<Func>(func), error());
				return zResult<_Ty>(error());
			}
			return *this;
		}

	private:
		std::variant<_Ty, Unexpected> data;
	};

	// Специализация для void
	template<>
	class zResult<void>
	{
	public:
		zResult() noexcept : hasVal(true) {}
		zResult(const Unexpected& error) noexcept : hasVal(false), err(error) {}
		zResult(Unexpected&& error) noexcept : hasVal(false), err(std::move(error)) {}

		inline bool has_value() const noexcept { return hasVal; }

		void value() const
		{
			assert(hasVal && ">>>>> [void result.value()]. Attempt to access value of an error.");
		}

		const Unexpected& error() const
		{
			assert(!hasVal && ">>>>> [void result.error()]. Attempt to access error of a value.");
			return err;
		}

		explicit operator bool() const noexcept { return hasVal; }

	private:
		template<typename Func>
		auto and_then_impl(Func&& func, std::true_type) const -> zResult<void>
		{
			if (hasVal)
			{
				std::invoke(std::forward<Func>(func));
				return zResult<void>{};
			}
			return zResult<void>{err};
		}

		template<typename Func>
		auto and_then_impl(Func&& func, std::false_type) const -> std::invoke_result_t<Func>
		{
			if (hasVal)
				return std::invoke(std::forward<Func>(func));

			return std::invoke_result_t<Func>{err};
		}

	public:
		template<typename Func>
		auto and_then(Func&& func) const
		{
			using R = std::invoke_result_t<Func>;
			constexpr bool isVoid = std::is_same_v<R, void>;

			return and_then_impl(std::forward<Func>(func), std::bool_constant<isVoid>{});
		}

	private:
		template<typename Func>
		auto or_else_impl(Func&& func, std::true_type) const -> zResult<void>
		{
			if (!hasVal)
			{
				std::invoke(std::forward<Func>(func), err);
				return zResult<void>{err};
			}

			return zResult<void>{};
		}

		template<typename Func>
		auto or_else_impl(Func&& func, std::false_type) const -> std::invoke_result_t<Func, const Unexpected&>
		{
			if (!hasVal)
				return std::invoke(std::forward<Func>(func), err);

			return std::invoke_result_t<Func, const Unexpected&>{};
		}

	public:
		template<typename Func>
		auto or_else(Func&& func) const
		{
			using R = std::invoke_result_t<Func, const Unexpected&>;
			constexpr bool isVoid = std::is_same_v<R, void>;

			return or_else_impl(std::forward<Func>(func), std::bool_constant<isVoid>{});
		}

	private:
		bool hasVal = true;
		Unexpected err;
	};
}
