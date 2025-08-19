#include "pch.h"
export module result;

export namespace zzz
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
		buffer_too_small,
		no_find,
		io_error_open_file,
		not_initialized,
		not_found,
		invalid_format,
		already_created		// Объект уже создан
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

	// Основной шаблон result<T>
	template<typename _Ty = void>
	class result
	{
	public:
		static_assert(std::is_copy_constructible_v<_Ty>, ">>>>> [class result]. Type _Ty must be copy constructible.");
		static_assert(std::is_move_constructible_v<_Ty>, ">>>>> [class result]. Type _Ty must be move constructible.");

		result(const _Ty& value) noexcept(std::is_nothrow_copy_constructible_v<_Ty>) : data(value) {}
		result(_Ty&& value) noexcept(std::is_nothrow_move_constructible_v<_Ty>) : data(std::move(value)) {}
		result(const Unexpected& error) noexcept : data(error) {}
		result(Unexpected&& error) noexcept : data(std::move(error)) {}

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

		bool operator==(const result& other) const noexcept { return data == other.data; }
		bool operator!=(const result& other) const noexcept { return !(*this == other); }

		_Ty& operator*() { return value(); }
		const _Ty& operator*() const { return value(); }

		_Ty* operator->() { return &value(); }
		const _Ty* operator->() const { return &value(); }

		//explicit operator result<void>() const noexcept
		//{
		//	if (has_value())
		//	{
		//		return result<void>{}; // Успех
		//	}
		//	return result<void>{error()}; // Ошибка
		//}

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
		auto and_then(Func&& func) const -> result<_Ty>
			requires std::is_same_v<std::invoke_result_t<Func, const _Ty&>, void>
		{
			if (has_value())
			{
				std::invoke(std::forward<Func>(func), value());
				return *this;
			}
			return result<_Ty>(error());
		}

		template<typename Func>
		auto or_else(Func&& func) const -> std::invoke_result_t<Func, const Unexpected&>
		{
			if (!has_value())
				return std::invoke(std::forward<Func>(func), error());

			return std::invoke_result_t<Func, const Unexpected&>(value());
		}

		template<typename Func>
		auto or_else(Func&& func) const -> result<_Ty>
			requires std::is_same_v<std::invoke_result_t<Func, const Unexpected&>, void>
		{
			if (!has_value())
			{
				std::invoke(std::forward<Func>(func), error());
				return result<_Ty>(error());
			}
			return *this;
		}

	private:
		std::variant<_Ty, Unexpected> data;
	};

	// Специализация для void
	template<>
	class result<void>
	{
	public:
		result() noexcept : hasVal(true) {}
		result(const Unexpected& error) noexcept : err(error)
		{
			hasVal = error.getCode() == eResult::success;
		}
		result(Unexpected&& error) noexcept : err(std::move(error))
		{
			hasVal = error.getCode() == eResult::success;
		}

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

		template<typename Func>
		auto and_then(Func&& func) const
		{
			using R = std::invoke_result_t<Func>;
			if constexpr (std::is_same_v<R, void>)
			{
				if (hasVal)
				{
					std::invoke(std::forward<Func>(func));
					return result<void>{};
				}
				return result<void>{err};
			}
			else
			{
				if (hasVal)
					return std::invoke(std::forward<Func>(func));
				else
					return R{ Unexpected(err) }; // <-- здесь гарантируем конструируемость
			}
		}

		template<typename Func>
		auto or_else(Func&& func) const
		{
			using R = std::invoke_result_t<Func, const Unexpected&>;

			if (!hasVal)
			{
				if constexpr (std::is_same_v<R, void>)
				{
					std::invoke(std::forward<Func>(func), err);
					return result<void>{err};
				}
				else
				{
					return std::invoke(std::forward<Func>(func), err);
				}
			}
			else
			{
				if constexpr (std::is_same_v<R, void>)
				{
					return result<void>{};
				}
				else
				{
					return R{}; // или static_assert(false) если недопустимо
				}
			}
		}

	private:
		bool hasVal;
		Unexpected err;
	};
}
