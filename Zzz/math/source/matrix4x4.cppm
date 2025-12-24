
#include "pch.h"

#include "../headers/headerSIMD.h"

export module Matrix4x4;

import Vector4;

export namespace zzz::math
{
	// Матрица 4x4 в column-major формате (как OpenGL, Metal, Vulkan)
	struct alignas(16) Matrix4x4
	{
		// 4 колонки, каждая - Vector4
		Vector4 columns[4];

		// --- Конструкторы ---
		Matrix4x4() noexcept
		{
			// Единичная матрица по умолчанию
			columns[0] = Vector4(1.0f, 0.0f, 0.0f, 0.0f);
			columns[1] = Vector4(0.0f, 1.0f, 0.0f, 0.0f);
			columns[2] = Vector4(0.0f, 0.0f, 1.0f, 0.0f);
			columns[3] = Vector4(0.0f, 0.0f, 0.0f, 1.0f);
		}

		// Конструктор из 4 колонок
		Matrix4x4(const Vector4& col0, const Vector4& col1,
			const Vector4& col2, const Vector4& col3) noexcept
		{
			columns[0] = col0;
			columns[1] = col1;
			columns[2] = col2;
			columns[3] = col3;
		}

		// Конструктор из 16 значений (row-major порядок для удобства)
		Matrix4x4(float m00, float m01, float m02, float m03,
			float m10, float m11, float m12, float m13,
			float m20, float m21, float m22, float m23,
			float m30, float m31, float m32, float m33) noexcept
		{
			// Транспонируем при записи (row -> column major)
			columns[0] = Vector4(m00, m10, m20, m30);
			columns[1] = Vector4(m01, m11, m21, m31);
			columns[2] = Vector4(m02, m12, m22, m32);
			columns[3] = Vector4(m03, m13, m23, m33);
		}

		// Диагональная матрица
		explicit Matrix4x4(float diagonal) noexcept
		{
			columns[0] = Vector4(diagonal, 0.0f, 0.0f, 0.0f);
			columns[1] = Vector4(0.0f, diagonal, 0.0f, 0.0f);
			columns[2] = Vector4(0.0f, 0.0f, diagonal, 0.0f);
			columns[3] = Vector4(0.0f, 0.0f, 0.0f, diagonal);
		}

		// --- Доступ к элементам ---
		Vector4& operator[](size_t col) noexcept { return columns[col]; }
		const Vector4& operator[](size_t col) const noexcept { return columns[col]; }

		// Доступ к элементу [row][col]
		float& at(size_t row, size_t col) noexcept { return columns[col][row]; }
		const float& at(size_t row, size_t col) const noexcept { return columns[col][row]; }

		// --- Арифметика ---
		Matrix4x4 operator+(const Matrix4x4& rhs) const noexcept
		{
			return Matrix4x4(
				columns[0] + rhs.columns[0],
				columns[1] + rhs.columns[1],
				columns[2] + rhs.columns[2],
				columns[3] + rhs.columns[3]
			);
		}

		Matrix4x4 operator-(const Matrix4x4& rhs) const noexcept
		{
			return Matrix4x4(
				columns[0] - rhs.columns[0],
				columns[1] - rhs.columns[1],
				columns[2] - rhs.columns[2],
				columns[3] - rhs.columns[3]
			);
		}

		Matrix4x4 operator*(float scalar) const noexcept
		{
			return Matrix4x4(
				columns[0] * scalar,
				columns[1] * scalar,
				columns[2] * scalar,
				columns[3] * scalar
			);
		}

		Matrix4x4 operator/(float scalar) const noexcept
		{
			return Matrix4x4(
				columns[0] / scalar,
				columns[1] / scalar,
				columns[2] / scalar,
				columns[3] / scalar
			);
		}

		// Умножение матриц (SIMD-оптимизированное)
		Matrix4x4 operator*(const Matrix4x4& rhs) const noexcept
		{
			Matrix4x4 Result;

#if defined(__APPLE__)
			// Metal напрямую поддерживает умножение матриц
			simd_float4x4 lhs_mat, rhs_mat;
			std::memcpy(&lhs_mat, this, sizeof(Matrix4x4));
			std::memcpy(&rhs_mat, &rhs, sizeof(Matrix4x4));
			simd_float4x4 res = ::simd::operator*(lhs_mat, rhs_mat);
			std::memcpy(&Result, &res, sizeof(Matrix4x4));
#else
			// Умножаем каждую колонку правой матрицы на левую
			for (int i = 0; i < 4; ++i)
			{
				Vector4 col;
#if defined(_M_X64) || defined(__x86_64__)
				__m128 c0 = _mm_mul_ps(columns[0].data, _mm_set1_ps(rhs.columns[i][0]));
				__m128 c1 = _mm_mul_ps(columns[1].data, _mm_set1_ps(rhs.columns[i][1]));
				__m128 c2 = _mm_mul_ps(columns[2].data, _mm_set1_ps(rhs.columns[i][2]));
				__m128 c3 = _mm_mul_ps(columns[3].data, _mm_set1_ps(rhs.columns[i][3]));
				col.data = _mm_add_ps(_mm_add_ps(c0, c1), _mm_add_ps(c2, c3));
#elif defined(_M_ARM64) || defined(__aarch64__)
				float32x4_t c0 = vmulq_n_f32(columns[0].data, rhs.columns[i][0]);
				float32x4_t c1 = vmulq_n_f32(columns[1].data, rhs.columns[i][1]);
				float32x4_t c2 = vmulq_n_f32(columns[2].data, rhs.columns[i][2]);
				float32x4_t c3 = vmulq_n_f32(columns[3].data, rhs.columns[i][3]);
				col.data = vaddq_f32(vaddq_f32(c0, c1), vaddq_f32(c2, c3));
#endif
				Result.columns[i] = col;
			}
#endif
			return Result;
		}

		// Умножение матрицы на вектор
		Vector4 operator*(const Vector4& v) const noexcept
		{
			Vector4 Result;

#if defined(__APPLE__)
			Result.data = ::simd::operator*(
				*reinterpret_cast<const simd_float4x4*>(this),
				v.data
				);
#elif defined(_M_X64) || defined(__x86_64__)
			__m128 c0 = _mm_mul_ps(columns[0].data, _mm_set1_ps(v[0]));
			__m128 c1 = _mm_mul_ps(columns[1].data, _mm_set1_ps(v[1]));
			__m128 c2 = _mm_mul_ps(columns[2].data, _mm_set1_ps(v[2]));
			__m128 c3 = _mm_mul_ps(columns[3].data, _mm_set1_ps(v[3]));
			Result.data = _mm_add_ps(_mm_add_ps(c0, c1), _mm_add_ps(c2, c3));
#elif defined(_M_ARM64) || defined(__aarch64__)
			float32x4_t c0 = vmulq_n_f32(columns[0].data, v[0]);
			float32x4_t c1 = vmulq_n_f32(columns[1].data, v[1]);
			float32x4_t c2 = vmulq_n_f32(columns[2].data, v[2]);
			float32x4_t c3 = vmulq_n_f32(columns[3].data, v[3]);
			Result.data = vaddq_f32(vaddq_f32(c0, c1), vaddq_f32(c2, c3));
#endif
			return Result;
		}

		Matrix4x4& operator+=(const Matrix4x4& rhs) noexcept
		{
			columns[0] += rhs.columns[0];
			columns[1] += rhs.columns[1];
			columns[2] += rhs.columns[2];
			columns[3] += rhs.columns[3];
			return *this;
		}

		Matrix4x4& operator-=(const Matrix4x4& rhs) noexcept
		{
			columns[0] -= rhs.columns[0];
			columns[1] -= rhs.columns[1];
			columns[2] -= rhs.columns[2];
			columns[3] -= rhs.columns[3];
			return *this;
		}

		Matrix4x4& operator*=(float scalar) noexcept
		{
			columns[0] *= scalar;
			columns[1] *= scalar;
			columns[2] *= scalar;
			columns[3] *= scalar;
			return *this;
		}

		Matrix4x4& operator*=(const Matrix4x4& rhs) noexcept
		{
			*this = *this * rhs;
			return *this;
		}

		// --- Транспонирование ---
		Matrix4x4 transposed() const noexcept
		{
			Matrix4x4 Result;

#if defined(__APPLE__)
			simd_float4x4 mat;
			std::memcpy(&mat, this, sizeof(Matrix4x4));
			simd_float4x4 trans = ::simd::transpose(mat);
			std::memcpy(&Result, &trans, sizeof(Matrix4x4));
#elif defined(_M_X64) || defined(__x86_64__)
			// Транспонируем 4x4 матрицу через SSE
			__m128 tmp0 = _mm_unpacklo_ps(columns[0].data, columns[1].data);
			__m128 tmp2 = _mm_unpacklo_ps(columns[2].data, columns[3].data);
			__m128 tmp1 = _mm_unpackhi_ps(columns[0].data, columns[1].data);
			__m128 tmp3 = _mm_unpackhi_ps(columns[2].data, columns[3].data);

			Result.columns[0].data = _mm_movelh_ps(tmp0, tmp2);
			Result.columns[1].data = _mm_movehl_ps(tmp2, tmp0);
			Result.columns[2].data = _mm_movelh_ps(tmp1, tmp3);
			Result.columns[3].data = _mm_movehl_ps(tmp3, tmp1);
#elif defined(_M_ARM64) || defined(__aarch64__)
			// NEON транспонирование через vtrn и vzip
			float32x4x2_t t01 = vtrnq_f32(columns[0].data, columns[1].data);
			float32x4x2_t t23 = vtrnq_f32(columns[2].data, columns[3].data);

			Result.columns[0].data = vcombine_f32(vget_low_f32(t01.val[0]), vget_low_f32(t23.val[0]));
			Result.columns[1].data = vcombine_f32(vget_low_f32(t01.val[1]), vget_low_f32(t23.val[1]));
			Result.columns[2].data = vcombine_f32(vget_high_f32(t01.val[0]), vget_high_f32(t23.val[0]));
			Result.columns[3].data = vcombine_f32(vget_high_f32(t01.val[1]), vget_high_f32(t23.val[1]));
#endif
			return Result;
		}

		void transpose() noexcept
		{
			*this = transposed();
		}

		// --- Определитель (SIMD-оптимизированный) ---
		float determinant() const noexcept
		{
#if defined(__APPLE__)
			simd_float4x4 mat;
			std::memcpy(&mat, this, sizeof(Matrix4x4));
			return ::simd::determinant(mat);
#else
			// Универсальный алгоритм для SSE и NEON
			// Работает с column-major форматом: columns[col][row]
			float a00 = columns[0][0], a01 = columns[1][0], a02 = columns[2][0], a03 = columns[3][0];
			float a10 = columns[0][1], a11 = columns[1][1], a12 = columns[2][1], a13 = columns[3][1];
			float a20 = columns[0][2], a21 = columns[1][2], a22 = columns[2][2], a23 = columns[3][2];
			float a30 = columns[0][3], a31 = columns[1][3], a32 = columns[2][3], a33 = columns[3][3];

			// Вычисляем кофакторы для первой строки
			float c00 = a11 * (a22 * a33 - a23 * a32) - a12 * (a21 * a33 - a23 * a31) + a13 * (a21 * a32 - a22 * a31);
			float c01 = a10 * (a22 * a33 - a23 * a32) - a12 * (a20 * a33 - a23 * a30) + a13 * (a20 * a32 - a22 * a30);
			float c02 = a10 * (a21 * a33 - a23 * a31) - a11 * (a20 * a33 - a23 * a30) + a13 * (a20 * a31 - a21 * a30);
			float c03 = a10 * (a21 * a32 - a22 * a31) - a11 * (a20 * a32 - a22 * a30) + a12 * (a20 * a31 - a21 * a30);

			// Детерминант = разложение по первой строке
			return a00 * c00 - a01 * c01 + a02 * c02 - a03 * c03;
#endif
		}

		// --- Обратная матрица (SIMD-оптимизированная) ---
		Matrix4x4 inverted() const noexcept
		{
#if defined(__APPLE__)
			simd_float4x4 mat;
			std::memcpy(&mat, this, sizeof(Matrix4x4));
			simd_float4x4 inv = ::simd::inverse(mat);

			Matrix4x4 Result;
			std::memcpy(&Result, &inv, sizeof(Matrix4x4));
			return Result;
#else
			// Универсальный алгоритм для x64 и ARM через вычисление присоединённой матрицы
			float det = determinant();

			// Проверка на вырожденность
			if (det > -1e-8f && det < 1e-8f)
			{
				return Matrix4x4(); // Возвращаем единичную матрицу
			}

			float invDet = 1.0f / det;

			// Извлекаем элементы из column-major: columns[col][row]
			float a00 = columns[0][0], a01 = columns[1][0], a02 = columns[2][0], a03 = columns[3][0];
			float a10 = columns[0][1], a11 = columns[1][1], a12 = columns[2][1], a13 = columns[3][1];
			float a20 = columns[0][2], a21 = columns[1][2], a22 = columns[2][2], a23 = columns[3][2];
			float a30 = columns[0][3], a31 = columns[1][3], a32 = columns[2][3], a33 = columns[3][3];

			Matrix4x4 Result;

			// Вычисляем присоединённую матрицу (adjugate) = транспонированная матрица кофакторов
			// Сохраняем в column-major формате
			Result.columns[0][0] = invDet * (a11 * (a22 * a33 - a23 * a32) - a12 * (a21 * a33 - a23 * a31) + a13 * (a21 * a32 - a22 * a31));
			Result.columns[1][0] = invDet * -(a01 * (a22 * a33 - a23 * a32) - a02 * (a21 * a33 - a23 * a31) + a03 * (a21 * a32 - a22 * a31));
			Result.columns[2][0] = invDet * (a01 * (a12 * a33 - a13 * a32) - a02 * (a11 * a33 - a13 * a31) + a03 * (a11 * a32 - a12 * a31));
			Result.columns[3][0] = invDet * -(a01 * (a12 * a23 - a13 * a22) - a02 * (a11 * a23 - a13 * a21) + a03 * (a11 * a22 - a12 * a21));

			Result.columns[0][1] = invDet * -(a10 * (a22 * a33 - a23 * a32) - a12 * (a20 * a33 - a23 * a30) + a13 * (a20 * a32 - a22 * a30));
			Result.columns[1][1] = invDet * (a00 * (a22 * a33 - a23 * a32) - a02 * (a20 * a33 - a23 * a30) + a03 * (a20 * a32 - a22 * a30));
			Result.columns[2][1] = invDet * -(a00 * (a12 * a33 - a13 * a32) - a02 * (a10 * a33 - a13 * a30) + a03 * (a10 * a32 - a12 * a30));
			Result.columns[3][1] = invDet * (a00 * (a12 * a23 - a13 * a22) - a02 * (a10 * a23 - a13 * a20) + a03 * (a10 * a22 - a12 * a20));

			Result.columns[0][2] = invDet * (a10 * (a21 * a33 - a23 * a31) - a11 * (a20 * a33 - a23 * a30) + a13 * (a20 * a31 - a21 * a30));
			Result.columns[1][2] = invDet * -(a00 * (a21 * a33 - a23 * a31) - a01 * (a20 * a33 - a23 * a30) + a03 * (a20 * a31 - a21 * a30));
			Result.columns[2][2] = invDet * (a00 * (a11 * a33 - a13 * a31) - a01 * (a10 * a33 - a13 * a30) + a03 * (a10 * a31 - a11 * a30));
			Result.columns[3][2] = invDet * -(a00 * (a11 * a23 - a13 * a21) - a01 * (a10 * a23 - a13 * a20) + a03 * (a10 * a21 - a11 * a20));

			Result.columns[0][3] = invDet * -(a10 * (a21 * a32 - a22 * a31) - a11 * (a20 * a32 - a22 * a30) + a12 * (a20 * a31 - a21 * a30));
			Result.columns[1][3] = invDet * (a00 * (a21 * a32 - a22 * a31) - a01 * (a20 * a32 - a22 * a30) + a02 * (a20 * a31 - a21 * a30));
			Result.columns[2][3] = invDet * -(a00 * (a11 * a32 - a12 * a31) - a01 * (a10 * a32 - a12 * a30) + a02 * (a10 * a31 - a11 * a30));
			Result.columns[3][3] = invDet * (a00 * (a11 * a22 - a12 * a21) - a01 * (a10 * a22 - a12 * a20) + a02 * (a10 * a21 - a11 * a20));

			return Result;
#endif
		}

		void invert() noexcept
		{
			*this = inverted();
		}

		// --- Статические фабричные методы ---

		// Матрица переноса
		static Matrix4x4 translation(float x, float y, float z) noexcept
		{
			Matrix4x4 Result;
			Result.columns[3] = Vector4(x, y, z, 1.0f);
			return Result;
		}

		static Matrix4x4 translation(const Vector4& v) noexcept
		{
			return translation(v[0], v[1], v[2]);
		}

		// Матрица масштабирования
		static Matrix4x4 scale(float x, float y, float z) noexcept
		{
			Matrix4x4 Result;
			Result.columns[0][0] = x;
			Result.columns[1][1] = y;
			Result.columns[2][2] = z;
			return Result;
		}

		static Matrix4x4 scale(float uniform) noexcept
		{
			return scale(uniform, uniform, uniform);
		}

		static Matrix4x4 scale(const Vector4& v) noexcept
		{
			return scale(v[0], v[1], v[2]);
		}

		// Вращение вокруг оси X
		static Matrix4x4 rotationX(float angleRadians) noexcept
		{
			float c = std::cos(angleRadians);
			float s = std::sin(angleRadians);

			Matrix4x4 Result;
			Result.columns[1][1] = c;
			Result.columns[1][2] = s;
			Result.columns[2][1] = -s;
			Result.columns[2][2] = c;
			return Result;
		}

		// Вращение вокруг оси Y
		static Matrix4x4 rotationY(float angleRadians) noexcept
		{
			float c = std::cos(angleRadians);
			float s = std::sin(angleRadians);

			Matrix4x4 Result;
			Result.columns[0][0] = c;
			Result.columns[0][2] = -s;
			Result.columns[2][0] = s;
			Result.columns[2][2] = c;
			return Result;
		}

		// Вращение вокруг оси Z
		static Matrix4x4 rotationZ(float angleRadians) noexcept
		{
			float c = std::cos(angleRadians);
			float s = std::sin(angleRadians);

			Matrix4x4 Result;
			Result.columns[0][0] = c;
			Result.columns[0][1] = s;
			Result.columns[1][0] = -s;
			Result.columns[1][1] = c;
			return Result;
		}

		// Вращение вокруг произвольной оси
		static Matrix4x4 rotation(const Vector4& axis, float angleRadians) noexcept
		{
			Vector4 a = axis.normalized();
			float c = std::cos(angleRadians);
			float s = std::sin(angleRadians);
			float t = 1.0f - c;

			float x = a[0], y = a[1], z = a[2];

			return Matrix4x4(
				t * x * x + c, t * x * y + z * s, t * x * z - y * s, 0.0f,
				t * x * y - z * s, t * y * y + c, t * y * z + x * s, 0.0f,
				t * x * z + y * s, t * y * z - x * s, t * z * z + c, 0.0f,
				0.0f, 0.0f, 0.0f, 1.0f
			);
		}

		// Look-At матрица (камера)
		static Matrix4x4 lookAt(const Vector4& eye, const Vector4& target, const Vector4& up) noexcept
		{
#if defined(ZRENDER_API_D3D12)
			// DirectX использует left-handed систему координат
			Vector4 zAxis = (target - eye).normalized();  // forward (к цели)
			Vector4 xAxis = zAxis.cross3(up).normalized();  // right (ПОМЕНЯЛИ ПОРЯДОК!)
			Vector4 yAxis = zAxis.cross3(xAxis);  // up

			Matrix4x4 Result;
			Result.columns[0] = Vector4(xAxis[0], xAxis[1], xAxis[2], 0.0f);
			Result.columns[1] = Vector4(yAxis[0], yAxis[1], yAxis[2], 0.0f);
			Result.columns[2] = Vector4(zAxis[0], zAxis[1], zAxis[2], 0.0f);
			Result.columns[3] = Vector4(-xAxis.dot(eye), -yAxis.dot(eye), -zAxis.dot(eye), 1.0f);
#else
			// Metal / Vulkan используют right-handed систему координат
			Vector4 zAxis = (eye - target).normalized();  // forward (от цели)
			Vector4 xAxis = up.cross3(zAxis).normalized();  // right
			Vector4 yAxis = zAxis.cross3(xAxis);  // up

			Matrix4x4 Result;
			Result.columns[0] = Vector4(xAxis[0], xAxis[1], xAxis[2], 0.0f);
			Result.columns[1] = Vector4(yAxis[0], yAxis[1], yAxis[2], 0.0f);
			Result.columns[2] = Vector4(zAxis[0], zAxis[1], zAxis[2], 0.0f);
			Result.columns[3] = Vector4(-xAxis.dot(eye), -yAxis.dot(eye), -zAxis.dot(eye), 1.0f);
#endif

			return Result;
		}

		// Перспективная проекция
		static Matrix4x4 perspective(float fovYRadians, float aspect, float nearZ, float farZ) noexcept
		{
			float tanHalfFov = std::tan(fovYRadians / 2.0f);
			Matrix4x4 Result(0.0f);

			// Диагональные элементы масштабирования
			Result.columns[0][0] = 1.0f / (aspect * tanHalfFov);
			Result.columns[1][1] = 1.0f / tanHalfFov;

#if defined(ZRENDER_API_D3D12)
			// Left-Handed (DirectX), Z от 0 до 1
			// Стандартная DirectX перспективная матрица:
			// columns[2] = [0, 0, farZ/(farZ-nearZ), 1]
			// columns[3] = [0, 0, -nearZ*farZ/(farZ-nearZ), 0]

			Result.columns[2][2] = farZ / (farZ - nearZ);
			Result.columns[2][3] = 1.0f;  // Перспективное деление (положительное для LH)
			Result.columns[3][2] = -(nearZ * farZ) / (farZ - nearZ);
			Result.columns[3][3] = 0.0f;
#else
			// Right-Handed (Metal/Vulkan/OpenGL), Z от -1 до 1
			// Стандартная OpenGL перспективная матрица:
			// columns[2] = [0, 0, -(farZ+nearZ)/(farZ-nearZ), -1]
			// columns[3] = [0, 0, -2*farZ*nearZ/(farZ-nearZ), 0]

			Result.columns[2][2] = -(farZ + nearZ) / (farZ - nearZ);
			Result.columns[2][3] = -1.0f;  // Перспективное деление (отрицательное для RH)
			Result.columns[3][2] = -(2.0f * farZ * nearZ) / (farZ - nearZ);
			Result.columns[3][3] = 0.0f;
#endif

			return Result;
		}

		static Matrix4x4 orthographic(float left, float right, float bottom,
			float top, float nearZ, float farZ) noexcept
		{
			Matrix4x4 Result(0.0f);
			Result.at(0, 0) = 2.0f / (right - left);
			Result.at(1, 1) = 2.0f / (top - bottom);

#if defined(ZRENDER_API_D3D12)
			// DirectX: Z от 0 до 1
			Result.at(2, 2) = 1.0f / (farZ - nearZ);
			Result.at(3, 0) = -(right + left) / (right - left);
			Result.at(3, 1) = -(top + bottom) / (top - bottom);
			Result.at(3, 2) = -nearZ / (farZ - nearZ);
			Result.at(3, 3) = 1.0f;
#else
			// Vulkan/Metal: Z от -1 до 1
			Result.at(2, 2) = -2.0f / (farZ - nearZ);
			Result.at(3, 0) = -(right + left) / (right - left);
			Result.at(3, 1) = -(top + bottom) / (top - bottom);
			Result.at(3, 2) = -(farZ + nearZ) / (farZ - nearZ);
			Result.at(3, 3) = 1.0f;
#endif

			return Result;
		}

		// Сравнение
		bool operator==(const Matrix4x4& rhs) const noexcept
		{
			return columns[0] == rhs.columns[0] &&
				columns[1] == rhs.columns[1] &&
				columns[2] == rhs.columns[2] &&
				columns[3] == rhs.columns[3];
		}

		bool operator!=(const Matrix4x4& rhs) const noexcept
		{
			return !(*this == rhs);
		}

		// Строковое представление
		std::string to_string() const
		{
			std::string Result = "[\n";
			for (int row = 0; row < 4; ++row)
			{
				Result += "  [";
				for (int col = 0; col < 4; ++col)
				{
					Result += std::to_string(at(row, col));
					if (col < 3) Result += ", ";
				}
				Result += "]";
				if (row < 3) Result += ",";
				Result += "\n";
			}
			Result += "]";
			return Result;
		}

		Vector4 row(size_t rowIndex) const noexcept
		{
			return Vector4(
				columns[0][rowIndex],
				columns[1][rowIndex],
				columns[2][rowIndex],
				columns[3][rowIndex]
			);
		}

		// Получение колонки (работает для column-major хранения)
		Vector4 column(size_t colIndex) const noexcept
		{
			return columns[colIndex];
		}

		// Установка строки
		void setRow(size_t rowIndex, const Vector4& rowData) noexcept
		{
			columns[0][rowIndex] = rowData[0];
			columns[1][rowIndex] = rowData[1];
			columns[2][rowIndex] = rowData[2];
			columns[3][rowIndex] = rowData[3];
		}

		// Установка колонки
		void setColumn(size_t colIndex, const Vector4& colData) noexcept
		{
			columns[colIndex] = colData;
		}

	private:
		// Вспомогательная функция для cross product (только для 3D векторов)
		static Vector4 cross3(const Vector4& a, const Vector4& b) noexcept
		{
			return Vector4(
				a[1] * b[2] - a[2] * b[1],
				a[2] * b[0] - a[0] * b[2],
				a[0] * b[1] - a[1] * b[0],
				0.0f
			);
		}

		static Matrix4x4 finalize_for_api(const Matrix4x4& m) noexcept
		{
#if defined(ZRENDER_API_D3D12)
			return m.transposed();
#else
			return m;
#endif
		}
	};

	// --- Операции с float слева ---
	inline Matrix4x4 operator*(float s, const Matrix4x4& m) noexcept { return m * s; }

	// --- Вывод в поток ---
	inline std::ostream& operator<<(std::ostream& os, const Matrix4x4& m)
	{
		return os << m.to_string();
	}
}