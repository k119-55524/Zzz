#pragma once

#include "math/vector/Vec3.h"
#include "math/vector/Vec4.h"
#include "math/matrix/Mat3.h"
#include "math/MathIncludes.h"

namespace zzz::math
{
	/**
	 * @struct Mat4
	 * @brief Шаблонная структура матрицы 4x4 (Row-Major в памяти, стандартная раскладка POD).
	 *        Система координат: Левосторонняя (Left-Handed: Y-up, Z-forward).
	 *        Диапазон глубины проекций NDC Z: [0, 1] (DirectX 12, Vulkan, Metal).
	 * 
	 * @tparam T Арифметический тип данных (по умолчанию zF32).
	 */
#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable: 4201) // nameless struct/union
#endif

	template<Arithmetic T = zF32>
	struct alignas(sizeof(T) * 4) Mat4
	{
		union
		{
			T m[4][4];
			T elements[16];
			struct
			{
				T _11, _12, _13, _14;
				T _21, _22, _23, _24;
				T _31, _32, _33, _34;
				T _41, _42, _43, _44;
			};
		};

#if defined(_MSC_VER)
#pragma warning(pop)
#endif

		/// @brief Конструктор по умолчанию: создает единичную матрицу (Identity).
		constexpr Mat4() noexcept
			: _11{ static_cast<T>(1) }, _12{ static_cast<T>(0) }, _13{ static_cast<T>(0) }, _14{ static_cast<T>(0) }
			, _21{ static_cast<T>(0) }, _22{ static_cast<T>(1) }, _23{ static_cast<T>(0) }, _24{ static_cast<T>(0) }
			, _31{ static_cast<T>(0) }, _32{ static_cast<T>(0) }, _33{ static_cast<T>(1) }, _34{ static_cast<T>(0) }
			, _41{ static_cast<T>(0) }, _42{ static_cast<T>(0) }, _43{ static_cast<T>(0) }, _44{ static_cast<T>(1) }
		{
		}

		/// @brief Диагональная матрица (все элементы диагонали равны diagonal, остальные 0).
		explicit constexpr Mat4(T diagonal) noexcept
			: _11{ diagonal }, _12{ static_cast<T>(0) }, _13{ static_cast<T>(0) }, _14{ static_cast<T>(0) }
			, _21{ static_cast<T>(0) }, _22{ diagonal }, _23{ static_cast<T>(0) }, _24{ static_cast<T>(0) }
			, _31{ static_cast<T>(0) }, _32{ static_cast<T>(0) }, _33{ diagonal }, _34{ static_cast<T>(0) }
			, _41{ static_cast<T>(0) }, _42{ static_cast<T>(0) }, _43{ static_cast<T>(0) }, _44{ diagonal }
		{
		}

		/// @brief Поэлементный конструктор со всеми 16 значениями.
		constexpr Mat4(
			T m00, T m01, T m02, T m03,
			T m10, T m11, T m12, T m13,
			T m20, T m21, T m22, T m23,
			T m30, T m31, T m32, T m33) noexcept
			: _11{ m00 }, _12{ m01 }, _13{ m02 }, _14{ m03 }
			, _21{ m10 }, _22{ m11 }, _23{ m12 }, _24{ m13 }
			, _31{ m20 }, _32{ m21 }, _33{ m22 }, _34{ m23 }
			, _41{ m30 }, _42{ m31 }, _43{ m32 }, _44{ m33 }
		{
		}

		/// @brief Конструктор из плоского массива из 16 элементов.
		explicit constexpr Mat4(const T el[16]) noexcept
			: _11{ el[0] }, _12{ el[1] }, _13{ el[2] }, _14{ el[3] }
			, _21{ el[4] }, _22{ el[5] }, _23{ el[6] }, _24{ el[7] }
			, _31{ el[8] }, _32{ el[9] }, _33{ el[10] }, _34{ el[11] }
			, _41{ el[12] }, _42{ el[13] }, _43{ el[14] }, _44{ el[15] }
		{
		}

		/// @brief Конструктор из 4 векторов-строк.
		constexpr Mat4(const Vec4<T>& r0, const Vec4<T>& r1, const Vec4<T>& r2, const Vec4<T>& r3) noexcept
			: _11{ r0.x }, _12{ r0.y }, _13{ r0.z }, _14{ r0.w }
			, _21{ r1.x }, _22{ r1.y }, _23{ r1.z }, _24{ r1.w }
			, _31{ r2.x }, _32{ r2.y }, _33{ r2.z }, _34{ r2.w }
			, _41{ r3.x }, _42{ r3.y }, _43{ r3.z }, _44{ r3.w }
		{
		}

		/// @brief Конструктор из 3x3 матрицы ориентации/базиса и вектора трансляции.
		constexpr Mat4(const Mat3<T>& rot, const Vec3<T>& trans = Vec3<T>::Zero()) noexcept
			: _11{ rot._11 }, _12{ rot._12 }, _13{ rot._13 }, _14{ static_cast<T>(0) }
			, _21{ rot._21 }, _22{ rot._22 }, _23{ rot._23 }, _24{ static_cast<T>(0) }
			, _31{ rot._31 }, _32{ rot._32 }, _33{ rot._33 }, _34{ static_cast<T>(0) }
			, _41{ trans.x }, _42{ trans.y }, _43{ trans.z }, _44{ static_cast<T>(1) }
		{
		}

		template<Arithmetic U> requires SafelyConvertibleTo<U, T>
		constexpr Mat4(const Mat4<U>& other) noexcept
		{
			for (size_t i = 0; i < 16; ++i)
				elements[i] = static_cast<T>(other.elements[i]);
		}

		// --- Доступ к элементам ---

		[[nodiscard]] constexpr const T* data() const noexcept { return elements; }
		[[nodiscard]] constexpr T* data() noexcept { return elements; }

		[[nodiscard]] constexpr const T& operator()(size_t row, size_t col) const noexcept
		{
			assert(row < 4 && col < 4 && "Mat4 index out of range");
			return m[row][col];
		}

		[[nodiscard]] constexpr T& operator()(size_t row, size_t col) noexcept
		{
			assert(row < 4 && col < 4 && "Mat4 index out of range");
			return m[row][col];
		}

		[[nodiscard]] constexpr const T& operator[](size_t index) const noexcept
		{
			assert(index < 16 && "Mat4 element index out of range");
			return elements[index];
		}

		[[nodiscard]] constexpr T& operator[](size_t index) noexcept
		{
			assert(index < 16 && "Mat4 element index out of range");
			return elements[index];
		}

		[[nodiscard]] constexpr Vec4<T> GetRow(size_t row) const noexcept
		{
			assert(row < 4 && "Mat4 row index out of range");
			return Vec4<T>{ m[row][0], m[row][1], m[row][2], m[row][3] };
		}

		constexpr void SetRow(size_t row, const Vec4<T>& v) noexcept
		{
			assert(row < 4 && "Mat4 row index out of range");
			m[row][0] = v.x;
			m[row][1] = v.y;
			m[row][2] = v.z;
			m[row][3] = v.w;
		}

		[[nodiscard]] constexpr Vec4<T> GetColumn(size_t col) const noexcept
		{
			assert(col < 4 && "Mat4 col index out of range");
			return Vec4<T>{ m[0][col], m[1][col], m[2][col], m[3][col] };
		}

		constexpr void SetColumn(size_t col, const Vec4<T>& v) noexcept
		{
			assert(col < 4 && "Mat4 col index out of range");
			m[0][col] = v.x;
			m[1][col] = v.y;
			m[2][col] = v.z;
			m[3][col] = v.w;
		}

		// --- Статические фабрики ---

		[[nodiscard]] static constexpr Mat4 Identity() noexcept
		{
			return Mat4{};
		}

		[[nodiscard]] static constexpr Mat4 Zero() noexcept
		{
			Mat4 result{};
			for (size_t i = 0; i < 16; ++i)
				result.elements[i] = static_cast<T>(0);
			return result;
		}

		[[nodiscard]] static constexpr Mat4 Translation(T x, T y, T z) noexcept
		{
			Mat4 result = Identity();
			result._41 = x;
			result._42 = y;
			result._43 = z;
			return result;
		}

		[[nodiscard]] static constexpr Mat4 Translation(const Vec3<T>& pos) noexcept
		{
			return Translation(pos.x, pos.y, pos.z);
		}

		[[nodiscard]] static constexpr Mat4 Scaling(T x, T y, T z) noexcept
		{
			Mat4 result = Zero();
			result._11 = x;
			result._22 = y;
			result._33 = z;
			result._44 = static_cast<T>(1);
			return result;
		}

		[[nodiscard]] static constexpr Mat4 Scaling(T uniformScale) noexcept
		{
			return Scaling(uniformScale, uniformScale, uniformScale);
		}

		[[nodiscard]] static constexpr Mat4 Scaling(const Vec3<T>& scale) noexcept
		{
			return Scaling(scale.x, scale.y, scale.z);
		}

		[[nodiscard]] static Mat4 RotationX(T radians) noexcept
		{
			const T c = std::cos(radians);
			const T s = std::sin(radians);
			Mat4 result = Identity();
			result._22 = c;
			result._23 = s;
			result._32 = -s;
			result._33 = c;
			return result;
		}

		[[nodiscard]] static Mat4 RotationY(T radians) noexcept
		{
			const T c = std::cos(radians);
			const T s = std::sin(radians);
			Mat4 result = Identity();
			result._11 = c;
			result._13 = -s;
			result._31 = s;
			result._33 = c;
			return result;
		}

		[[nodiscard]] static Mat4 RotationZ(T radians) noexcept
		{
			const T c = std::cos(radians);
			const T s = std::sin(radians);
			Mat4 result = Identity();
			result._11 = c;
			result._12 = s;
			result._21 = -s;
			result._22 = c;
			return result;
		}

		[[nodiscard]] static Mat4 RotationAxis(const Vec3<T>& axis, T radians) noexcept
		{
			const Vec3<T> n = axis.Normalized();
			const T c = std::cos(radians);
			const T s = std::sin(radians);
			const T oneMinusC = static_cast<T>(1) - c;

			Mat4 result = Identity();
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

		/// @brief Создает комбинированную матрицу трансформации: Translation * Rotation(YXZ euler) * Scaling
		[[nodiscard]] static Mat4 TRS(const Vec3<T>& translation, const Vec3<T>& rotationEuler, const Vec3<T>& scale) noexcept
		{
			const Mat4 s = Scaling(scale);
			const Mat4 rX = RotationX(rotationEuler.x);
			const Mat4 rY = RotationY(rotationEuler.y);
			const Mat4 rZ = RotationZ(rotationEuler.z);
			const Mat4 r = rZ * rX * rY;
			const Mat4 t = Translation(translation);

			// Порядок применения трансформации к вектору: v * (S * R * T)
			return s * r * t;
		}

		// --- Проекции и камеры (Левосторонняя система координат LH, NDC Z [0, 1]) ---

		/**
		 * @brief Создает левостороннюю матрицу вида (View Matrix).
		 * @param eye Позиция камеры в мировом пространстве.
		 * @param target Точка, на которую смотрит камера.
		 * @param up Вектор направления «вверх» мира (обычно {0, 1, 0}).
		 */
		[[nodiscard]] static Mat4 LookAtLH(const Vec3<T>& eye, const Vec3<T>& target, const Vec3<T>& up) noexcept
		{
			const Vec3<T> zAxis = (target - eye).Normalized(); // Взгляд вперед (+Z)
			const Vec3<T> xAxis = up.Cross(zAxis).Normalized(); // Вправо (+X)
			const Vec3<T> yAxis = zAxis.Cross(xAxis); // Вверх (+Y)

			Mat4 result{};
			result._11 = xAxis.x;
			result._12 = yAxis.x;
			result._13 = zAxis.x;
			result._14 = static_cast<T>(0);

			result._21 = xAxis.y;
			result._22 = yAxis.y;
			result._23 = zAxis.y;
			result._24 = static_cast<T>(0);

			result._31 = xAxis.z;
			result._32 = yAxis.z;
			result._33 = zAxis.z;
			result._34 = static_cast<T>(0);

			result._41 = -xAxis.Dot(eye);
			result._42 = -yAxis.Dot(eye);
			result._43 = -zAxis.Dot(eye);
			result._44 = static_cast<T>(1);

			return result;
		}

		/**
		 * @brief Создает левостороннюю перспективную матрицу проекции с диапазоном глубины NDC Z [0, 1].
		 * @param fovYRadians Угол обзора по вертикали в радианах.
		 * @param aspect Соотношение сторон вьюпорта (width / height).
		 * @param nearZ Расстояние до ближней плоскости отсечения (> 0).
		 * @param farZ Расстояние до дальней плоскости отсечения (> nearZ).
		 */
		[[nodiscard]] static Mat4 PerspectiveFovLH(T fovYRadians, T aspect, T nearZ, T farZ) noexcept
		{
			assert(nearZ > static_cast<T>(0) && farZ > nearZ && "Invalid near/far plane distance");
			assert(aspect > static_cast<T>(0) && "Aspect ratio must be positive");

			const T yScale = static_cast<T>(1) / std::tan(fovYRadians * static_cast<T>(0.5));
			const T xScale = yScale / aspect;
			const T range = farZ / (farZ - nearZ);

			Mat4 result = Zero();
			result._11 = xScale;
			result._22 = yScale;
			result._33 = range;
			result._34 = static_cast<T>(1);
			result._43 = -nearZ * range;
			return result;
		}

		/**
		 * @brief Создает левостороннюю ортографическую матрицу проекции с диапазоном глубины NDC Z [0, 1].
		 * @param width Ширина зоны видимости.
		 * @param height Высота зоны видимости.
		 * @param nearZ Ближняя плоскость отсечения.
		 * @param farZ Дальняя плоскость отсечения.
		 */
		[[nodiscard]] static Mat4 OrthographicLH(T width, T height, T nearZ, T farZ) noexcept
		{
			assert(width > static_cast<T>(0) && height > static_cast<T>(0));
			assert(farZ != nearZ);

			const T range = static_cast<T>(1) / (farZ - nearZ);

			Mat4 result = Zero();
			result._11 = static_cast<T>(2) / width;
			result._22 = static_cast<T>(2) / height;
			result._33 = range;
			result._43 = -nearZ * range;
			result._44 = static_cast<T>(1);
			return result;
		}

		/**
		 * @brief Создает левостороннюю ортографическую проекцию с настраиваемыми границами (для 2D/UI).
		 */
		[[nodiscard]] static Mat4 OrthographicOffCenterLH(T left, T right, T bottom, T top, T nearZ, T farZ) noexcept
		{
			assert(right != left && top != bottom && farZ != nearZ);

			const T recWidth = static_cast<T>(1) / (right - left);
			const T recHeight = static_cast<T>(1) / (top - bottom);
			const T range = static_cast<T>(1) / (farZ - nearZ);

			Mat4 result = Zero();
			result._11 = static_cast<T>(2) * recWidth;
			result._22 = static_cast<T>(2) * recHeight;
			result._33 = range;
			result._41 = -(left + right) * recWidth;
			result._42 = -(top + bottom) * recHeight;
			result._43 = -nearZ * range;
			result._44 = static_cast<T>(1);
			return result;
		}

		// --- Алгебраические операции ---

		[[nodiscard]] constexpr Mat4 operator*(const Mat4& other) const noexcept
		{
			Mat4 result{};
			for (size_t r = 0; r < 4; ++r)
			{
				for (size_t c = 0; c < 4; ++c)
				{
					result.m[r][c] =
						m[r][0] * other.m[0][c] +
						m[r][1] * other.m[1][c] +
						m[r][2] * other.m[2][c] +
						m[r][3] * other.m[3][c];
				}
			}
			return result;
		}

		constexpr Mat4& operator*=(const Mat4& other) noexcept
		{
			return *this = *this * other;
		}

		[[nodiscard]] constexpr Mat4 operator*(T scalar) const noexcept
		{
			Mat4 result{};
			for (size_t i = 0; i < 16; ++i)
				result.elements[i] = elements[i] * scalar;
			return result;
		}

		constexpr Mat4& operator*=(T scalar) noexcept
		{
			for (size_t i = 0; i < 16; ++i)
				elements[i] *= scalar;
			return *this;
		}

		[[nodiscard]] constexpr bool operator==(const Mat4& other) const noexcept
		{
			for (size_t i = 0; i < 16; ++i)
			{
				if (elements[i] != other.elements[i])
					return false;
			}
			return true;
		}

		[[nodiscard]] constexpr bool operator!=(const Mat4& other) const noexcept
		{
			return !(*this == other);
		}

		/// @brief Возвращает транспонированную матрицу.
		[[nodiscard]] constexpr Mat4 Transpose() const noexcept
		{
			return Mat4{
				_11, _21, _31, _41,
				_12, _22, _32, _42,
				_13, _23, _33, _43,
				_14, _24, _34, _44
			};
		}

		/// @brief Вычисляет определитель матрицы 4x4.
		[[nodiscard]] constexpr T Determinant() const noexcept
		{
			const T sub00 = _33 * _44 - _34 * _43;
			const T sub01 = _32 * _44 - _34 * _42;
			const T sub02 = _32 * _43 - _33 * _42;
			const T sub03 = _31 * _44 - _34 * _41;
			const T sub04 = _31 * _43 - _33 * _41;
			const T sub05 = _31 * _42 - _32 * _41;

			const T c00 = _22 * sub00 - _23 * sub01 + _24 * sub02;
			const T c01 = -(_21 * sub00 - _23 * sub03 + _24 * sub04);
			const T c02 = _21 * sub01 - _22 * sub03 + _24 * sub05;
			const T c03 = -(_21 * sub02 - _22 * sub04 + _23 * sub05);

			return _11 * c00 + _12 * c01 + _13 * c02 + _14 * c03;
		}

		/// @brief Вычисляет обратную матрицу. Если определитель равен 0, возвращает единичную матрицу и *outInvertible = false.
		[[nodiscard]] Mat4 Inverse(bool* outInvertible = nullptr) const noexcept
		{
			const T a00 = _11, a01 = _12, a02 = _13, a03 = _14;
			const T a10 = _21, a11 = _22, a12 = _23, a13 = _24;
			const T a20 = _31, a21 = _32, a22 = _33, a23 = _34;
			const T a30 = _41, a31 = _42, a32 = _43, a33 = _44;

			const T b00 = a00 * a11 - a01 * a10;
			const T b01 = a00 * a12 - a02 * a10;
			const T b02 = a00 * a13 - a03 * a10;
			const T b03 = a01 * a12 - a02 * a11;
			const T b04 = a01 * a13 - a03 * a11;
			const T b05 = a02 * a13 - a03 * a12;
			const T b06 = a20 * a31 - a21 * a30;
			const T b07 = a20 * a32 - a22 * a30;
			const T b08 = a20 * a33 - a23 * a30;
			const T b09 = a21 * a32 - a22 * a31;
			const T b10 = a21 * a33 - a23 * a31;
			const T b11 = a22 * a33 - a23 * a32;

			const T det = b00 * b11 - b01 * b10 + b02 * b09 + b03 * b08 - b04 * b07 + b05 * b06;

			if (std::abs(det) <= static_cast<T>(1e-6))
			{
				if (outInvertible) *outInvertible = false;
				return Identity();
			}

			if (outInvertible) *outInvertible = true;
			const T invDet = static_cast<T>(1) / det;

			Mat4 result{};
			result._11 = (a11 * b11 - a12 * b10 + a13 * b09) * invDet;
			result._12 = (-a01 * b11 + a02 * b10 - a03 * b09) * invDet;
			result._13 = (a31 * b05 - a32 * b04 + a33 * b03) * invDet;
			result._14 = (-a21 * b05 + a22 * b04 - a23 * b03) * invDet;

			result._21 = (-a10 * b11 + a12 * b08 - a13 * b07) * invDet;
			result._22 = (a00 * b11 - a02 * b08 + a03 * b07) * invDet;
			result._23 = (-a30 * b05 + a32 * b02 - a33 * b01) * invDet;
			result._24 = (a20 * b05 - a22 * b02 + a23 * b01) * invDet;

			result._31 = (a10 * b10 - a11 * b08 + a13 * b06) * invDet;
			result._32 = (-a00 * b10 + a01 * b08 - a03 * b06) * invDet;
			result._33 = (a30 * b04 - a31 * b02 + a33 * b00) * invDet;
			result._34 = (-a20 * b04 + a21 * b02 - a23 * b00) * invDet;

			result._41 = (-a10 * b09 + a11 * b07 - a12 * b06) * invDet;
			result._42 = (a00 * b09 - a01 * b07 + a02 * b06) * invDet;
			result._43 = (-a30 * b03 + a31 * b01 - a32 * b00) * invDet;
			result._44 = (a20 * b03 - a21 * b01 + a22 * b00) * invDet;

			return result;
		}

		/// @brief Извлекает верхний левый 3x3 блок матрицы (базис ориентации и масштаба).
		[[nodiscard]] constexpr Mat3<T> ToMat3() const noexcept
		{
			return Mat3<T>{
				_11, _12, _13,
				_21, _22, _23,
				_31, _32, _33
			};
		}

		/// @brief Вычисляет матрицу нормалей: Transpose(Inverse(ToMat3())).
		[[nodiscard]] Mat3<T> GetNormalMatrix() const noexcept
		{
			return ToMat3().Inverse().Transpose();
		}

		/// @brief Умножает 4D вектор-строку на матрицу: v * M
		[[nodiscard]] constexpr Vec4<T> TransformVector4(const Vec4<T>& v) const noexcept
		{
			return Vec4<T>{
				v.x * _11 + v.y * _21 + v.z * _31 + v.w * _41,
				v.x * _12 + v.y * _22 + v.z * _32 + v.w * _42,
				v.x * _13 + v.y * _23 + v.z * _33 + v.w * _43,
				v.x * _14 + v.y * _24 + v.z * _34 + v.w * _44
			};
		}

		/// @brief Трансформирует 3D-точку (Point: W=1) с учетом перспективного деления (деление на результирующий W).
		[[nodiscard]] constexpr Vec3<T> TransformPoint(const Vec3<T>& pt) const noexcept
		{
			const Vec4<T> v4 = TransformVector4(Vec4<T>{ pt, static_cast<T>(1) });
			if (v4.w != static_cast<T>(0) && v4.w != static_cast<T>(1))
			{
				const T invW = static_cast<T>(1) / v4.w;
				return Vec3<T>{ v4.x * invW, v4.y * invW, v4.z * invW };
			}
			return Vec3<T>{ v4.x, v4.y, v4.z };
		}

		/// @brief Трансформирует 3D-направление/нормаль (Vector: W=0) без учета смещения.
		[[nodiscard]] constexpr Vec3<T> TransformVector(const Vec3<T>& vec) const noexcept
		{
			return Vec3<T>{
				vec.x * _11 + vec.y * _21 + vec.z * _31,
				vec.x * _12 + vec.y * _22 + vec.z * _32,
				vec.x * _13 + vec.y * _23 + vec.z * _33
			};
		}

		[[nodiscard]] inline std::string ToString() const noexcept
		{
			return std::format(
				"[({:.3f}, {:.3f}, {:.3f}, {:.3f}), ({:.3f}, {:.3f}, {:.3f}, {:.3f}), ({:.3f}, {:.3f}, {:.3f}, {:.3f}), ({:.3f}, {:.3f}, {:.3f}, {:.3f})]",
				_11, _12, _13, _14,
				_21, _22, _23, _24,
				_31, _32, _33, _34,
				_41, _42, _43, _44
			);
		}
	};

	/// @brief Оператор умножения вектора на матрицу: v * M
	template<Arithmetic T>
	constexpr Vec4<T> operator*(const Vec4<T>& v, const Mat4<T>& m) noexcept
	{
		return m.TransformVector4(v);
	}

	using Mat4f = Mat4<zF32>;
	using Mat4d = Mat4<zF64>;
	using Mat4i = Mat4<zI32>;

	static_assert(std::is_standard_layout_v<Mat4<zF32>>, "Mat4 must be standard layout");
	static_assert(sizeof(Mat4<zF32>) == 64, "Mat4<zF32> must be exactly 64 bytes");
	static_assert(alignof(Mat4<zF32>) == 16, "Mat4<zF32> must be aligned to 16 bytes");
	static_assert(sizeof(Mat4<zF64>) == 128, "Mat4<zF64> must be exactly 128 bytes");
	static_assert(alignof(Mat4<zF64>) == 32, "Mat4<zF64> must be aligned to 32 bytes");
}

template<zzz::math::Arithmetic T>
struct std::formatter<zzz::math::Mat4<T>> : std::formatter<std::string>
{
	auto format(const zzz::math::Mat4<T>& m, std::format_context& ctx) const
	{
		return std::formatter<std::string>::format(m.ToString(), ctx);
	}
};
