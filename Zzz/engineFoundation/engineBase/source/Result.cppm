
export module Result;

namespace zzz
{
	class bad_expected_access : public std::exception
	{
	public:
		bad_expected_access(const char* msg) noexcept : _msg(msg) {}
		const char* what() const noexcept override { return _msg; }

	private:
		const char* _msg;
	};

	export enum class eResult : unsigned int
	{
		success = 0,
		failure,
		exception,
		invalid_argument,
		out_of_memory,
		buffer_too_small,
		no_find,
		no_make_shared_ptr,
		io_error_open_file,
		not_initialized,
		not_found,
		invalid_format,
		already_created		// Объект уже создан
	};

	export class Unexpected
	{
	public:
		Unexpected() noexcept
			: m_code(eResult::success)
		{
		}

		Unexpected(
			eResult code,
			const std::source_location& loc = std::source_location::current()) noexcept
			: m_code(code)
		{
			m_message = FormatMessage(L"<no message>", loc);
		}

		explicit Unexpected(
			eResult code,
			std::wstring message,
			const std::source_location& loc = std::source_location::current()) noexcept
			: m_code(code)
			, m_message(FormatMessage(std::move(message), loc))
		{
		}

		template<typename... Args>
		Unexpected(
			eResult code,
			std::wformat_string<Args...> fmt,
			Args&&... args) noexcept
			: m_code(code)
		{
			const auto loc = std::source_location::current();

			try
			{
				m_message = FormatMessage(
					std::format(fmt, std::forward<Args>(args)...),
					loc);
			}
			catch (...)
			{
				m_message = FormatMessage(L"<format error>", loc);
			}
		}

		template<typename... Args>
		explicit Unexpected(
			std::wformat_string<Args...> fmt,
			Args&&... args) noexcept
			: m_code(eResult::failure)
		{
			const auto loc = std::source_location::current();

			try
			{
				m_message = FormatMessage(
					std::format(fmt, std::forward<Args>(args)...),
					loc);
			}
			catch (...)
			{
				m_message = FormatMessage(L"<format error>", loc);
			}
		}

	public:
		[[nodiscard]] eResult getCode() const noexcept { return m_code; }
		[[nodiscard]] const std::wstring& getMessage() const noexcept { return m_message; }

		bool operator==(const Unexpected& other) const noexcept
		{
			return m_code == other.m_code &&
				m_message == other.m_message;
		}

		bool operator!=(const Unexpected& other) const noexcept
		{
			return !(*this == other);
		}

	private:
		eResult m_code{};
		std::wstring m_message{};

		static std::wstring FormatMessage(
			std::wstring msg,
			const std::source_location& loc) noexcept
		{
			std::wstringstream ss;

			ss << L">>>>> ["
				<< loc.function_name()
				<< L"]\nExpected: "
				<< msg
				<< L"\n    Line: "
				<< loc.line()
				<< L"\n    File: "
				<< loc.file_name();

			return ss.str();
		}
	};

	// Основной шаблон Result<T>
	export template<typename _Ty = void>
	class Result
	{
	public:
		//static_assert(std::is_copy_constructible_v<_Ty>, ">>>>> [class Result]. Type _Ty must be copy constructible.");
		static_assert(std::is_move_constructible_v<_Ty>, ">>>>> [class Result]. Type _Ty must be move constructible.");

		Result(const _Ty& value) noexcept(std::is_nothrow_copy_constructible_v<_Ty>) : data(value) {}
		Result(_Ty&& value) noexcept(std::is_nothrow_move_constructible_v<_Ty>) : data(std::move(value)) {}
		Result(const Unexpected& error) noexcept : data(error) {}
		Result(Unexpected&& error) noexcept : data(std::move(error)) {}

		inline bool has_value() const noexcept { return std::holds_alternative<_Ty>(data); }

		_Ty& value()
		{
			assert(has_value() && ">>>>> [_Ty& Result.value()]. Attempt to access value of an error.");
			return std::get<_Ty>(data);
		}

		const _Ty& value() const
		{
			assert(has_value() && ">>>>> [const _Ty& Result.value()]. Attempt to access value of an error.");
			return std::get<_Ty>(data);
		}

		Unexpected& error()
		{
			assert(!has_value() && ">>>>> [Unexpected& Result.error()]. Attempt to access error of a value.");
			return std::get<Unexpected>(data);
		}

		const Unexpected& error() const
		{
			assert(!has_value() && ">>>>> [const Unexpected& Result.error()]. Attempt to access error of a value.");
			return std::get<Unexpected>(data);
		}

		bool operator==(const Result& other) const noexcept { return data == other.data; }
		bool operator!=(const Result& other) const noexcept { return !(*this == other); }

		_Ty& operator*() { return value(); }
		const _Ty& operator*() const { return value(); }

		_Ty* operator->() { return &value(); }
		const _Ty* operator->() const { return &value(); }

		//explicit operator Result<void>() const noexcept
		//{
		//	if (has_value())
		//	{
		//		return Result<void>{}; // Успех
		//	}
		//	return Result<void>{error()}; // Ошибка
		//}

		explicit operator bool() const noexcept { return has_value(); }

		template<typename U>
		_Ty value_or(U&& default_value) const noexcept(std::is_nothrow_constructible_v<_Ty, U>)
		{
			static_assert(std::is_convertible_v<U, _Ty>, "U type must be convertible to _Ty");
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
		auto or_else(Func&& func) const -> std::invoke_result_t<Func, const Unexpected&>
		{
			if (!has_value())
				return std::invoke(std::forward<Func>(func), error());

			return std::invoke_result_t<Func, const Unexpected&>(value());
		}

		template<typename Func>
		auto or_else(Func&& func) const -> Result<_Ty>
			requires std::is_same_v<std::invoke_result_t<Func, const Unexpected&>, void>
		{
			if (!has_value())
			{
				std::invoke(std::forward<Func>(func), error());
				return Result<_Ty>(error());
			}
			return *this;
		}

	private:
		std::variant<_Ty, Unexpected> data;
	};

	// Специализация для void
	export template<>
	class Result<void>
	{
	public:
		Result() noexcept : hasVal(true) {}
		Result(const Unexpected& error) noexcept : err(error)
		{
			hasVal = error.getCode() == eResult::success;
		}
		Result(Unexpected&& error) noexcept : err(std::move(error))
		{
			hasVal = error.getCode() == eResult::success;
		}

		inline bool has_value() const noexcept { return hasVal; }

		void value() const
		{
			assert(hasVal && ">>>>> [void Result.value()]. Attempt to access value of an error.");
		}

		const Unexpected& error() const
		{
			assert(!hasVal && ">>>>> [void Result.error()]. Attempt to access error of a value.");
			return err;
		}

		explicit operator bool() const noexcept { return hasVal; }

		template<typename Func>
		auto and_then(Func&& func) const
		{
			using R = std::invoke_result_t<Func>;

			if constexpr (std::is_same_v<R, void>)
			{
				// Случай 1: func возвращает void
				if (hasVal)
				{
					std::invoke(std::forward<Func>(func));
					return Result<void>{};
				}
				return Result<void>{err};
			}
			else if constexpr (std::is_same_v<R, Result<void>>)
			{
				// Случай 2: func возвращает Result<void>
				if (hasVal)
					return std::invoke(std::forward<Func>(func));
				else
					return Result<void>{err};
			}
			else
			{
				// ЭТОТ БЛОК ДОЛЖЕН БЫТЬ!
				// Случай 3: func возвращает Result<T> где T != void
				if (hasVal)
					return std::invoke(std::forward<Func>(func));
				else
					return R{ err };
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
					return Result<void>{err};
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
					return Result<void>{};
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
