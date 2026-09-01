#pragma once

#include "math/vector/Vec3.h"
#include "math/MathIncludes.h"

namespace zzz::math
{
	template<Arithmetic T>
	struct Mat4;

	/**
	 * @struct Mat3
	 * @brief Шаблонная структура матрицы 3x3 (Row-Major в памяти, стандартная раскладка POD).
	 *        Используется для ориентации/базиса, трансформации нормалей (Normal Matrix) и 2D-преобразований.
	 * 
	 * @tparam T Арифметический тип данных (по умолчанию zF32).
	 */
#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable: 4201) // nameless struct/union
#endif

	template<Arithmetic T = zF32>
	struct Mat3
	{
		union
		{
			T m[3][3];
			T elements[9];
			struct
			{
				T _11, _12, _13;
				T _21, _22, _23;
				T _31, _32, _33;
			};
		};

#if defined(_MSC_VER)
#pragma warning(pop)
#endif

		/// @brief Конструктор по умолчанию: создает единичную матрицу (Identity).
		constexpr Mat3() noexcept
			: _11{ static_cast<T>(1) }, _12{ static_cast<T>(0) }, _13{ static_cast<T>(0) }
			, _21{ static_cast<T>(0) }, _22{ static_cast<T>(1) }, _23{ static_cast<T>(0) }
			, _31{ static_cast<T>(0) }, _32{ static_cast<T>(0) }, _33{ static_cast<T>(1) }
		{
		}

		/// @brief Диагональная матрица (все элементы диагонали равны diagonal, остальные 0).
		explicit constexpr Mat3(T diagonal) noexcept
			: _11{ diagonal }, _12{ static_cast<T>(0) }, _13{ static_cast<T>(0) }
			, _21{ static_cast<T>(0) }, _22{ diagonal }, _23{ static_cast<T>(0) }
			, _31{ static_cast<T>(0) }, _32{ static_cast<T>(0) }, _33{ diagonal }
		{
		}

		/// @brief Поэлементный конструктор со всеми 9 значениями.
		constexpr Mat3(
			T m00, T m01, T m02,
			T m10, T m11, T m12,
			T m20, T m21, T m22) noexcept
			: _11{ m00 }, _12{ m01 }, _13{ m02 }
			, _21{ m10 }, _22{ m11 }, _23{ m12 }
			, _31{ m20 }, _32{ m21 }, _33{ m22 }
		{
		}

		/// @brief Конструктор из плоского массива из 9 элементов.
		explicit constexpr Mat3(const T el[9]) noexcept
			: _11{ el[0] }, _12{ el[1] }, _13{ el[2] }
			, _21{ el[3] }, _22{ el[4] }, _23{ el[5] }
			, _31{ el[6] }, _32{ el[7] }, _33{ el[8] }
		{
		}

		/// @brief Конструктор из 3 векторов-строк.
		constexpr Mat3(const Vec3<T>& r0, const Vec3<T>& r1, const Vec3<T>& r2) noexcept
			: _11{ r0.x }, _12{ r0.y }, _13{ r0.z }
			, _21{ r1.x }, _22{ r1.y }, _23{ r1.z }
			, _31{ r2.x }, _32{ r2.y }, _33{ r2.z }
		{
		}

		template<Arithmetic U> requires SafelyConvertibleTo<U, T>
		constexpr Mat3(const Mat3<U>& other) noexcept
		{
			for (size_t i = 0; i < 9; ++i)
				elements[i] = static_cast<T>(other.elements[i]);
		}

		// --- Доступ к элементам ---

		[[nodiscard]] constexpr const T* data() const noexcept { return elements; }
		[[nodiscard]] constexpr T* data() noexcept { return elements; }

		[[nodiscard]] constexpr const T& operator()(size_t row, size_t col) const noexcept
		{
			assert(row < 3 && col < 3 && "Mat3 index out of range");
			return m[row][col];
		}

		[[nodiscard]] constexpr T& operator()(size_t row, size_t col) noexcept
		{
			assert(row < 3 && col < 3 && "Mat3 index out of range");
			return m[row][col];
		}

		[[nodiscard]] constexpr const T& operator[](size_t index) const noexcept
		{
			assert(index < 9 && "Mat3 element index out of range");
			return elements[index];
		}

		[[nodiscard]] constexpr T& operator[](size_t index) noexcept
		{
			assert(index < 9 && "Mat3 element index out of range");
			return elements[index];
		}

		[[nodiscard]] constexpr Vec3<T> GetRow(size_t row) const noexcept
		{
			assert(row < 3 && "Mat3 row index out of range");
			return Vec3<T>{ m[row][0], m[row][1], m[row][2] };
		}

		constexpr void SetRow(size_t row, const Vec3<T>& v) noexcept
		{
			assert(row < 3 && "Mat3 row index out of range");
			m[row][0] = v.x;
			m[row][1] = v.y;
			m[row][2] = v.z;
		}

		[[nodiscard]] constexpr Vec3<T> GetColumn(size_t col) const noexcept
		{
			assert(col < 3 && "Mat3 col index out of range");
			return Vec3<T>{ m[0][col], m[1][col], m[2][col] };
		}

		constexpr void SetColumn(size_t col, const Vec3<T>& v) noexcept
		{
			assert(col < 3 && "Mat3 col index out of range");
			m[0][col] = v.x;
			m[1][col] = v.y;
			m[2][col] = v.z;
		}

		// --- Статические фабрики ---

		[[nodiscard]] static constexpr Mat3 Identity() noexcept
		{
			return Mat3{};
		}

		[[nodiscard]] static constexpr Mat3 Zero() noexcept
		{
			Mat3 result{};
			for (size_t i = 0; i < 9; ++i)
				result.elements[i] = static_cast<T>(0);
			return result;
		}

		[[nodiscard]] static constexpr Mat3 Scaling(T x, T y, T z) noexcept
		{
			Mat3 result = Zero();
			result._11 = x;
			result._22 = y;
			result._33 = z;
			return result;
		}

		[[nodiscard]] static constexpr Mat3 Scaling(T uniformScale) noexcept
		{
			return Scaling(uniformScale, uniformScale, uniformScale);
		}

		[[nodiscard]] static constexpr Mat3 Scaling(const Vec3<T>& scale) noexcept
		{
			return Scaling(scale.x, scale.y, scale.z);
		}

		[[nodiscard]] static Mat3 RotationX(T radians) noexcept
		{
			const T c = std::cos(radians);
			const T s = std::sin(radians);
			Mat3 result = Identity();
			result._22 = c;
			result._23 = s;
			result._32 = -s;
			result._33 = c;
			return result;
		}

		[[nodiscard]] static Mat3 RotationY(T radians) noexcept
		{
			const T c = std::cos(radians);
			const T s = std::sin(radians);
			Mat3 result = Identity();
			result._11 = c;
			result._13 = -s;
			result._31 = s;
			result._33 = c;
			return result;
		}

		[[nodiscard]] static Mat3 RotationZ(T radians) noexcept
		{
			const T c = std::cos(radians);
			const T s = std::sin(radians);
			Mat3 result = Identity();
			result._11 = c;
			result._12 = s;
			result._21 = -s;
			result._22 = c;
			return result;
		}

		[[nodiscard]] static Mat3 RotationAxis(const Vec3<T>& axis, T radians) noexcept
		{
			const Vec3<T> n = axis.Normalized();
			const T c = std::cos(radians);
			const T s = std::sin(radians);
			const T oneMinusC = static_cast<T>(1) - c;

			Mat3 result = Identity();
			result._11 = c + n.x * n.x * oneMinusC;
			result._12 = n.x * n.y * oneMinusC + n.z * s;
			result._13 = n.x * n.z * oneMinusC - n.y * s;

			result._21 = n.y * n.x * oneMinusC - n.z * s;
			result._22 = c + n.y * n.y * oneMinusC;
			result._23 = n.y * n.z * oneMinusC + n.x * s;

			result._31 = n.z * n.x * oneMinusC + n.y * s;
			result._32 = n.z * n.y * oneMinusC - n.x * s;
			result._33 = c + n.z * n.z * oneMinusC;

			return result;
		}

		[[nodiscard]] constexpr Mat3 operator*(const Mat3& other) const noexcept
		{
			Mat3 result{};
			for (size_t r = 0; r < 3; ++r)
			{
				for (size_t c = 0; c < 3; ++c)
				{
					result.m[r][c] =
						m[r][0] * other.m[0][c] +
						m[r][1] * other.m[1][c] +
						m[r][2] * other.m[2][c];
				}
			}
			return result;
		}

		constexpr Mat3& operator*=(const Mat3& other) noexcept
		{
			return *this = *this * other;
		}

		[[nodiscard]] constexpr Mat3 operator*(T scalar) const noexcept
		{
			Mat3 result{};
			for (size_t i = 0; i < 9; ++i)
				result.elements[i] = elements[i] * scalar;
			return result;
		}

		constexpr Mat3& operator*=(T scalar) noexcept
		{
			for (size_t i = 0; i < 9; ++i)
				elements[i] *= scalar;
			return *this;
		}

		[[nodiscard]] constexpr bool operator==(const Mat3& other) const noexcept
		{
			for (size_t i = 0; i < 9; ++i)
			{
				if (elements[i] != other.elements[i])
					return false;
			}
			return true;
		}

		[[nodiscard]] constexpr bool operator!=(const Mat3& other) const noexcept
		{
			return !(*this == other);
		}

		/// @brief Возвращает транспонированную матрицу.
		[[nodiscard]] constexpr Mat3 Transpose() const noexcept
		{
			return Mat3{
				_11, _21, _31,
				_12, _22, _32,
				_13, _23, _33
			};
		}

		/// @brief Вычисляет определитель матрицы 3x3.
		[[nodiscard]] constexpr T Determinant() const noexcept
		{
			return _11 * (_22 * _33 - _23 * _32)
				 - _12 * (_21 * _33 - _23 * _31)
				 + _13 * (_21 * _32 - _22 * _31);
		}

		/// @brief Вычисляет обратную матрицу 3x3. Если вырождена, возвращает Identity и *outInvertible = false.
		[[nodiscard]] Mat3 Inverse(bool* outInvertible = nullptr) const noexcept
		{
			const T det = Determinant();
			if (std::abs(det) <= static_cast<T>(1e-6))
			{
				if (outInvertible) *outInvertible = false;
				return Identity();
			}

			if (outInvertible) *outInvertible = true;
			const T invDet = static_cast<T>(1) / det;

			Mat3 result{};
			result._11 = (_22 * _33 - _23 * _32) * invDet;
			result._12 = (_13 * _32 - _12 * _33) * invDet;
			result._13 = (_12 * _23 - _13 * _22) * invDet;

			result._21 = (_23 * _31 - _21 * _33) * invDet;
			result._22 = (_11 * _33 - _13 * _31) * invDet;
			result._23 = (_13 * _21 - _11 * _23) * invDet;

			result._31 = (_21 * _32 - _22 * _31) * invDet;
			result._32 = (_12 * _31 - _11 * _32) * invDet;
			result._33 = (_11 * _22 - _12 * _21) * invDet;

			return result;
		}

		/// @brief Трансформирует 3D вектор (умножение вектора-строки на матрицу: v * M).
		[[nodiscard]] constexpr Vec3<T> TransformVector(const Vec3<T>& v) const noexcept
		{
			return Vec3<T>{
				v.x * _11 + v.y * _21 + v.z * _31,
				v.x * _12 + v.y * _22 + v.z * _32,
				v.x * _13 + v.y * _23 + v.z * _33
			};
		}

		[[nodiscard]] inline std::string ToString() const noexcept
		{
			return std::format(
				"[({:.3f}, {:.3f}, {:.3f}), ({:.3f}, {:.3f}, {:.3f}), ({:.3f}, {:.3f}, {:.3f})]",
				_11, _12, _13,
				_21, _22, _23,
				_31, _32, _33
			);
		}
	};

	/// @brief Оператор умножения вектора на матрицу 3x3: v * M
	template<Arithmetic T>
	constexpr Vec3<T> operator*(const Vec3<T>& v, const Mat3<T>& m) noexcept
	{
		return m.TransformVector(v);
	}

	using Mat3f = Mat3<zF32>;
	using Mat3d = Mat3<zF64>;
	using Mat3i = Mat3<zI32>;

	static_assert(std::is_standard_layout_v<Mat3<zF32>>, "Mat3 must be standard layout");
	static_assert(sizeof(Mat3<zF32>) == 36, "Mat3<zF32> must be exactly 36 bytes");
	static_assert(sizeof(Mat3<zF64>) == 72, "Mat3<zF64> must be exactly 72 bytes");
}

template<zzz::math::Arithmetic T>
struct std::formatter<zzz::math::Mat3<T>> : std::formatter<std::string>
{
	auto format(const zzz::math::Mat3<T>& m, std::format_context& ctx) const
	{
		return std::formatter<std::string>::format(m.ToString(), ctx);
	}
};
