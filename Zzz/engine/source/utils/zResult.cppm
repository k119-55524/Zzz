
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
		exeption,
		invalid_argument,
		out_of_memory,
		buffer_too_small
	};

	class Unexpected
	{
	public:
		Unexpected() noexcept : code(eResult::failure), message(L"") {}
		Unexpected(eResult code) noexcept : code(code), message(L"") {}
		Unexpected(eResult code, const std::wstring& message) noexcept : code(code), message(message) {}

		inline eResult getCode() const noexcept { return code; }
		inline const std::wstring& getMessage() const noexcept { return message; }

		inline bool operator==(const Unexpected& other) const noexcept { return code == other.code && message == other.message; }
		inline bool operator!=(const Unexpected& other) const noexcept { return !(*this == other); }

	private:
		eResult code;
		std::wstring message;
	};

	template<typename _Ty = eResult>
	class zResult
	{
		public:
			static_assert(std::is_copy_constructible_v<_Ty>, ">>>>> [class result]. Type _Ty must be copy constructible.");
			static_assert(std::is_move_constructible_v<_Ty>, ">>>>> [class result]. Type _Ty must be move constructible.");

			zResult(const _Ty& value) noexcept(std::is_nothrow_copy_constructible_v<_Ty>) : data(value) {}
			zResult(_Ty&& value) noexcept(std::is_nothrow_move_constructible_v<_Ty>)	: data(std::move(value)) {}
			zResult(const Unexpected& error) noexcept : data(error) {}
			zResult(Unexpected&& error) noexcept : data(std::move(error)) {}

			// Проверка, содержит ли объект успешное значение
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

			// Оператор * для удобного доступа к значению
			_Ty& operator*() { return value(); }
			const _Ty& operator*() const { return value(); }

			// Оператор -> для доступа к членам значения
			_Ty* operator->() { return &value(); }
			const _Ty* operator->() const { return &value(); }

			// Неявное преобразование в bool для проверки успеха
			explicit operator bool() const noexcept { return has_value(); }

			// Возвращает значение или значение по умолчанию
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
					return *this; // Возвращаем текущий zResult
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
			auto or_else(Func&& func) const -> zResult<_Ty> requires std::is_same_v<std::invoke_result_t<Func, const Unexpected&>, void>
			{
				if (!has_value())
				{
					std::invoke(std::forward<Func>(func), error());
					return zResult<_Ty>(error()); // Возвращаем текущую ошибку
				}

				return *this; // Возвращаем текущий zResult, если значение присутствует
			}

		private:
			std::variant<_Ty, Unexpected> data;
	};
}