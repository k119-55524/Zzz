
#include "pch.h"

#include "../headers/headerSIMD.h"

export module matrix4x4;

import vector4;

export namespace zzz::math
{
	// Матрица 4x4 в column-major формате (как OpenGL, Metal, Vulkan)
	struct alignas(16) matrix4x4
	{
		// 4 колонки, каждая - vector4
		vector4 columns[4];

		// --- Конструкторы ---
		matrix4x4() noexcept
		{
			// Единичная матрица по умолчанию
			columns[0] = vector4(1.0f, 0.0f, 0.0f, 0.0f);
			columns[1] = vector4(0.0f, 1.0f, 0.0f, 0.0f);
			columns[2] = vector4(0.0f, 0.0f, 1.0f, 0.0f);
			columns[3] = vector4(0.0f, 0.0f, 0.0f, 1.0f);
		}

		// Конструктор из 4 колонок
		matrix4x4(const vector4& col0, const vector4& col1,
			const vector4& col2, const vector4& col3) noexcept
		{
			columns[0] = col0;
			columns[1] = col1;
			columns[2] = col2;
			columns[3] = col3;
		}

		// Конструктор из 16 значений (row-major порядок для удобства)
		matrix4x4(float m00, float m01, float m02, float m03,
			float m10, float m11, float m12, float m13,
			float m20, float m21, float m22, float m23,
			float m30, float m31, float m32, float m33) noexcept
		{
			// Транспонируем при записи (row -> column major)
			columns[0] = vector4(m00, m10, m20, m30);
			columns[1] = vector4(m01, m11, m21, m31);
			columns[2] = vector4(m02, m12, m22, m32);
			columns[3] = vector4(m03, m13, m23, m33);
		}

		// Диагональная матрица
		explicit matrix4x4(float diagonal) noexcept
		{
			columns[0] = vector4(diagonal, 0.0f, 0.0f, 0.0f);
			columns[1] = vector4(0.0f, diagonal, 0.0f, 0.0f);
			columns[2] = vector4(0.0f, 0.0f, diagonal, 0.0f);
			columns[3] = vector4(0.0f, 0.0f, 0.0f, diagonal);
		}

		// --- Доступ к элементам ---
		vector4& operator[](size_t col) noexcept { return columns[col]; }
		const vector4& operator[](size_t col) const noexcept { return columns[col]; }

		// Доступ к элементу [row][col]
		float& at(size_t row, size_t col) noexcept { return columns[col][row]; }
		const float& at(size_t row, size_t col) const noexcept { return columns[col][row]; }

		// --- Арифметика ---
		matrix4x4 operator+(const matrix4x4& rhs) const noexcept
		{
			return matrix4x4(
				columns[0] + rhs.columns[0],
				columns[1] + rhs.columns[1],
				columns[2] + rhs.columns[2],
				columns[3] + rhs.columns[3]
			);
		}

		matrix4x4 operator-(const matrix4x4& rhs) const noexcept
		{
			return matrix4x4(
				columns[0] - rhs.columns[0],
				columns[1] - rhs.columns[1],
				columns[2] - rhs.columns[2],
				columns[3] - rhs.columns[3]
			);
		}

		matrix4x4 operator*(float scalar) const noexcept
		{
			return matrix4x4(
				columns[0] * scalar,
				columns[1] * scalar,
				columns[2] * scalar,
				columns[3] * scalar
			);
		}

		matrix4x4 operator/(float scalar) const noexcept
		{
			return matrix4x4(
				columns[0] / scalar,
				columns[1] / scalar,
				columns[2] / scalar,
				columns[3] / scalar
			);
		}

		// Умножение матриц (SIMD-оптимизированное)
		matrix4x4 operator*(const matrix4x4& rhs) const noexcept
		{
			matrix4x4 result;

#if defined(__APPLE__)
			// Metal напрямую поддерживает умножение матриц
			simd_float4x4 lhs_mat, rhs_mat;
			std::memcpy(&lhs_mat, this, sizeof(matrix4x4));
			std::memcpy(&rhs_mat, &rhs, sizeof(matrix4x4));
			simd_float4x4 res = ::simd::operator*(lhs_mat, rhs_mat);
			std::memcpy(&result, &res, sizeof(matrix4x4));
#else
			// Умножаем каждую колонку правой матрицы на левую
			for (int i = 0; i < 4; ++i)
			{
				vector4 col;
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
				result.columns[i] = col;
			}
#endif
			return result;
		}

		// Умножение матрицы на вектор
		vector4 operator*(const vector4& v) const noexcept
		{
			vector4 result;

#if defined(__APPLE__)
			result.data = ::simd::operator*(
				*reinterpret_cast<const simd_float4x4*>(this),
				v.data
				);
#elif defined(_M_X64) || defined(__x86_64__)
			__m128 c0 = _mm_mul_ps(columns[0].data, _mm_set1_ps(v[0]));
			__m128 c1 = _mm_mul_ps(columns[1].data, _mm_set1_ps(v[1]));
			__m128 c2 = _mm_mul_ps(columns[2].data, _mm_set1_ps(v[2]));
			__m128 c3 = _mm_mul_ps(columns[3].data, _mm_set1_ps(v[3]));
			result.data = _mm_add_ps(_mm_add_ps(c0, c1), _mm_add_ps(c2, c3));
#elif defined(_M_ARM64) || defined(__aarch64__)
			float32x4_t c0 = vmulq_n_f32(columns[0].data, v[0]);
			float32x4_t c1 = vmulq_n_f32(columns[1].data, v[1]);
			float32x4_t c2 = vmulq_n_f32(columns[2].data, v[2]);
			float32x4_t c3 = vmulq_n_f32(columns[3].data, v[3]);
			result.data = vaddq_f32(vaddq_f32(c0, c1), vaddq_f32(c2, c3));
#endif
			return result;
		}

		matrix4x4& operator+=(const matrix4x4& rhs) noexcept
		{
			columns[0] += rhs.columns[0];
			columns[1] += rhs.columns[1];
			columns[2] += rhs.columns[2];
			columns[3] += rhs.columns[3];
			return *this;
		}

		matrix4x4& operator-=(const matrix4x4& rhs) noexcept
		{
			columns[0] -= rhs.columns[0];
			columns[1] -= rhs.columns[1];
			columns[2] -= rhs.columns[2];
			columns[3] -= rhs.columns[3];
			return *this;
		}

		matrix4x4& operator*=(float scalar) noexcept
		{
			columns[0] *= scalar;
			columns[1] *= scalar;
			columns[2] *= scalar;
			columns[3] *= scalar;
			return *this;
		}

		matrix4x4& operator*=(const matrix4x4& rhs) noexcept
		{
			*this = *this * rhs;
			return *this;
		}

		// --- Транспонирование ---
		matrix4x4 transposed() const noexcept
		{
			matrix4x4 result;

#if defined(__APPLE__)
			simd_float4x4 mat;
			std::memcpy(&mat, this, sizeof(matrix4x4));
			simd_float4x4 trans = ::simd::transpose(mat);
			std::memcpy(&result, &trans, sizeof(matrix4x4));
#elif defined(_M_X64) || defined(__x86_64__)
			// Транспонируем 4x4 матрицу через SSE
			__m128 tmp0 = _mm_unpacklo_ps(columns[0].data, columns[1].data);
			__m128 tmp2 = _mm_unpacklo_ps(columns[2].data, columns[3].data);
			__m128 tmp1 = _mm_unpackhi_ps(columns[0].data, columns[1].data);
			__m128 tmp3 = _mm_unpackhi_ps(columns[2].data, columns[3].data);

			result.columns[0].data = _mm_movelh_ps(tmp0, tmp2);
			result.columns[1].data = _mm_movehl_ps(tmp2, tmp0);
			result.columns[2].data = _mm_movelh_ps(tmp1, tmp3);
			result.columns[3].data = _mm_movehl_ps(tmp3, tmp1);
#elif defined(_M_ARM64) || defined(__aarch64__)
			// NEON транспонирование через vtrn и vzip
			float32x4x2_t t01 = vtrnq_f32(columns[0].data, columns[1].data);
			float32x4x2_t t23 = vtrnq_f32(columns[2].data, columns[3].data);

			result.columns[0].data = vcombine_f32(vget_low_f32(t01.val[0]), vget_low_f32(t23.val[0]));
			result.columns[1].data = vcombine_f32(vget_low_f32(t01.val[1]), vget_low_f32(t23.val[1]));
			result.columns[2].data = vcombine_f32(vget_high_f32(t01.val[0]), vget_high_f32(t23.val[0]));
			result.columns[3].data = vcombine_f32(vget_high_f32(t01.val[1]), vget_high_f32(t23.val[1]));
#endif
			return result;
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
			std::memcpy(&mat, this, sizeof(matrix4x4));
			return ::simd::determinant(mat);
#elif defined(_M_X64) || defined(__x86_64__)
			// Intel SSE оптимизация
			__m128 minor0, minor1, minor2, minor3;
			__m128 row0, row1, row2, row3;
			__m128 det, tmp1;

			// Транспонируем для удобства
			tmp1 = _mm_unpacklo_ps(columns[0].data, columns[1].data);
			row1 = _mm_unpackhi_ps(columns[0].data, columns[1].data);
			row0 = _mm_movelh_ps(tmp1, _mm_unpacklo_ps(columns[2].data, columns[3].data));
			row1 = _mm_movehl_ps(_mm_unpacklo_ps(columns[2].data, columns[3].data), tmp1);
			tmp1 = _mm_unpackhi_ps(columns[2].data, columns[3].data);
			row2 = _mm_movelh_ps(row1, tmp1);
			row3 = _mm_movehl_ps(tmp1, row1);

			// Вычисляем sub-детерминанты
			tmp1 = _mm_mul_ps(row2, row3);
			tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0xB1);
			minor0 = _mm_mul_ps(row1, tmp1);
			minor1 = _mm_mul_ps(row0, tmp1);
			tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0x4E);
			minor0 = _mm_sub_ps(_mm_mul_ps(row1, tmp1), minor0);
			minor1 = _mm_sub_ps(_mm_mul_ps(row0, tmp1), minor1);
			minor1 = _mm_shuffle_ps(minor1, minor1, 0x4E);

			// Продолжаем вычисления
			tmp1 = _mm_mul_ps(row1, row2);
			tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0xB1);
			minor0 = _mm_add_ps(_mm_mul_ps(row3, tmp1), minor0);
			minor3 = _mm_mul_ps(row0, tmp1);
			tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0x4E);
			minor0 = _mm_sub_ps(minor0, _mm_mul_ps(row3, tmp1));
			minor3 = _mm_sub_ps(_mm_mul_ps(row0, tmp1), minor3);
			minor3 = _mm_shuffle_ps(minor3, minor3, 0x4E);

			tmp1 = _mm_mul_ps(_mm_shuffle_ps(row1, row1, 0x4E), row3);
			tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0xB1);
			row2 = _mm_shuffle_ps(row2, row2, 0x4E);
			minor0 = _mm_add_ps(_mm_mul_ps(row2, tmp1), minor0);
			minor2 = _mm_mul_ps(row0, tmp1);
			tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0x4E);
			minor0 = _mm_sub_ps(minor0, _mm_mul_ps(row2, tmp1));
			minor2 = _mm_sub_ps(_mm_mul_ps(row0, tmp1), minor2);
			minor2 = _mm_shuffle_ps(minor2, minor2, 0x4E);

			tmp1 = _mm_mul_ps(row0, row1);
			tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0xB1);
			minor2 = _mm_add_ps(_mm_mul_ps(row3, tmp1), minor2);
			minor3 = _mm_sub_ps(_mm_mul_ps(row2, tmp1), minor3);
			tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0x4E);
			minor2 = _mm_sub_ps(_mm_mul_ps(row3, tmp1), minor2);
			minor3 = _mm_sub_ps(minor3, _mm_mul_ps(row2, tmp1));

			tmp1 = _mm_mul_ps(row0, row3);
			tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0xB1);
			minor1 = _mm_sub_ps(minor1, _mm_mul_ps(row2, tmp1));
			minor2 = _mm_add_ps(_mm_mul_ps(row1, tmp1), minor2);
			tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0x4E);
			minor1 = _mm_add_ps(_mm_mul_ps(row2, tmp1), minor1);
			minor2 = _mm_sub_ps(minor2, _mm_mul_ps(row1, tmp1));

			tmp1 = _mm_mul_ps(row0, row2);
			tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0xB1);
			minor1 = _mm_add_ps(_mm_mul_ps(row3, tmp1), minor1);
			minor3 = _mm_sub_ps(minor3, _mm_mul_ps(row1, tmp1));
			tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0x4E);
			minor1 = _mm_sub_ps(minor1, _mm_mul_ps(row3, tmp1));
			minor3 = _mm_add_ps(_mm_mul_ps(row1, tmp1), minor3);

			// Вычисляем детерминант
			det = _mm_mul_ps(row0, minor0);
			det = _mm_add_ps(_mm_shuffle_ps(det, det, 0x4E), det);
			det = _mm_add_ss(_mm_shuffle_ps(det, det, 0xB1), det);

			return _mm_cvtss_f32(det);
#elif defined(_M_ARM64) || defined(__aarch64__)
			// ARM NEON оптимизация
			// Используем упрощённую версию для NEON
			float32x4_t row0 = columns[0].data;
			float32x4_t row1 = columns[1].data;
			float32x4_t row2 = columns[2].data;
			float32x4_t row3 = columns[3].data;

			// Вычисляем sub-факторы
			float32x4_t tmp0 = vmulq_f32(
				vzip2q_f32(row2, row2),
				vzip2q_f32(row3, row3)
			);

			float32x4_t tmp1 = vmulq_f32(
				vzip1q_f32(row2, row2),
				vzip2q_f32(row3, row3)
			);

			float32x4_t minor0 = vsubq_f32(tmp0, tmp1);

			// Dot product для финального детерминанта
			float32x4_t det = vmulq_f32(row0, minor0);
			float32x2_t sum = vadd_f32(vget_low_f32(det), vget_high_f32(det));
			return vget_lane_f32(vpadd_f32(sum, sum), 0);
#endif
		}

		// --- Обратная матрица (SIMD-оптимизированная) ---
		matrix4x4 inverted() const noexcept
		{
#if defined(__APPLE__)
			simd_float4x4 mat;
			std::memcpy(&mat, this, sizeof(matrix4x4));
			simd_float4x4 inv = ::simd::inverse(mat);

			matrix4x4 result;
			std::memcpy(&result, &inv, sizeof(matrix4x4));
			return result;
#elif defined(_M_X64) || defined(__x86_64__)
			// Intel SSE оптимизированная инверсия матрицы
			__m128 minor0, minor1, minor2, minor3;
			__m128 row0, row1, row2, row3;
			__m128 det, tmp1;

			// Транспонируем матрицу
			tmp1 = _mm_unpacklo_ps(columns[0].data, columns[1].data);
			row1 = _mm_unpackhi_ps(columns[0].data, columns[1].data);
			row0 = _mm_movelh_ps(tmp1, _mm_unpacklo_ps(columns[2].data, columns[3].data));
			row1 = _mm_movehl_ps(_mm_unpacklo_ps(columns[2].data, columns[3].data), tmp1);
			tmp1 = _mm_unpackhi_ps(columns[2].data, columns[3].data);
			row2 = _mm_movelh_ps(row1, tmp1);
			row3 = _mm_movehl_ps(tmp1, row1);

			// Вычисляем все миноры параллельно
			tmp1 = _mm_mul_ps(row2, row3);
			tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0xB1);
			minor0 = _mm_mul_ps(row1, tmp1);
			minor1 = _mm_mul_ps(row0, tmp1);
			tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0x4E);
			minor0 = _mm_sub_ps(_mm_mul_ps(row1, tmp1), minor0);
			minor1 = _mm_sub_ps(_mm_mul_ps(row0, tmp1), minor1);
			minor1 = _mm_shuffle_ps(minor1, minor1, 0x4E);

			tmp1 = _mm_mul_ps(row1, row2);
			tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0xB1);
			minor0 = _mm_add_ps(_mm_mul_ps(row3, tmp1), minor0);
			minor3 = _mm_mul_ps(row0, tmp1);
			tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0x4E);
			minor0 = _mm_sub_ps(minor0, _mm_mul_ps(row3, tmp1));
			minor3 = _mm_sub_ps(_mm_mul_ps(row0, tmp1), minor3);
			minor3 = _mm_shuffle_ps(minor3, minor3, 0x4E);

			tmp1 = _mm_mul_ps(_mm_shuffle_ps(row1, row1, 0x4E), row3);
			tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0xB1);
			row2 = _mm_shuffle_ps(row2, row2, 0x4E);
			minor0 = _mm_add_ps(_mm_mul_ps(row2, tmp1), minor0);
			minor2 = _mm_mul_ps(row0, tmp1);
			tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0x4E);
			minor0 = _mm_sub_ps(minor0, _mm_mul_ps(row2, tmp1));
			minor2 = _mm_sub_ps(_mm_mul_ps(row0, tmp1), minor2);
			minor2 = _mm_shuffle_ps(minor2, minor2, 0x4E);

			tmp1 = _mm_mul_ps(row0, row1);
			tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0xB1);
			minor2 = _mm_add_ps(_mm_mul_ps(row3, tmp1), minor2);
			minor3 = _mm_sub_ps(_mm_mul_ps(row2, tmp1), minor3);
			tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0x4E);
			minor2 = _mm_sub_ps(_mm_mul_ps(row3, tmp1), minor2);
			minor3 = _mm_sub_ps(minor3, _mm_mul_ps(row2, tmp1));

			tmp1 = _mm_mul_ps(row0, row3);
			tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0xB1);
			minor1 = _mm_sub_ps(minor1, _mm_mul_ps(row2, tmp1));
			minor2 = _mm_add_ps(_mm_mul_ps(row1, tmp1), minor2);
			tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0x4E);
			minor1 = _mm_add_ps(_mm_mul_ps(row2, tmp1), minor1);
			minor2 = _mm_sub_ps(minor2, _mm_mul_ps(row1, tmp1));

			tmp1 = _mm_mul_ps(row0, row2);
			tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0xB1);
			minor1 = _mm_add_ps(_mm_mul_ps(row3, tmp1), minor1);
			minor3 = _mm_sub_ps(minor3, _mm_mul_ps(row1, tmp1));
			tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0x4E);
			minor1 = _mm_sub_ps(minor1, _mm_mul_ps(row3, tmp1));
			minor3 = _mm_add_ps(_mm_mul_ps(row1, tmp1), minor3);

			// Вычисляем детерминант
			det = _mm_mul_ps(row0, minor0);
			det = _mm_add_ps(_mm_shuffle_ps(det, det, 0x4E), det);
			det = _mm_add_ss(_mm_shuffle_ps(det, det, 0xB1), det);

			// Проверяем на вырожденность
			tmp1 = _mm_rcp_ss(det);
			det = _mm_sub_ss(_mm_add_ss(tmp1, tmp1), _mm_mul_ss(det, _mm_mul_ss(tmp1, tmp1)));
			det = _mm_shuffle_ps(det, det, 0x00);

			// Умножаем миноры на 1/det
			matrix4x4 result;
			result.columns[0].data = _mm_mul_ps(det, minor0);
			result.columns[1].data = _mm_mul_ps(det, minor1);
			result.columns[2].data = _mm_mul_ps(det, minor2);
			result.columns[3].data = _mm_mul_ps(det, minor3);

			return result;
#elif defined(_M_ARM64) || defined(__aarch64__)
			// ARM NEON оптимизированная инверсия
			// Для полной SIMD-версии нужен более сложный код
			// Используем гибридный подход: SIMD где возможно

			matrix4x4 result;
			float det = determinant();

			if (std::abs(det) < 1e-8f)
			{
				return matrix4x4(); // Единичная матрица
			}

			float invDet = 1.0f / det;

			// Вычисляем миноры используя NEON
			for (int col = 0; col < 4; ++col)
			{
				for (int row = 0; row < 4; ++row)
				{
					// Извлекаем 3x3 минор
					float m[9];
					int idx = 0;
					for (int j = 0; j < 4; ++j)
					{
						if (j == col) continue;
						for (int i = 0; i < 4; ++i)
						{
							if (i == row) continue;
							m[idx++] = at(i, j);
						}
					}

					// Детерминант 3x3 через NEON
					float32x4_t a = vld1q_f32(&m[0]); // m[0..3]
					float32x4_t b = vld1q_f32(&m[3]); // m[3..6]
					float32x4_t c = vld1q_f32(&m[6]); // m[6..8, padding]

					float det3 = m[0] * (m[4] * m[8] - m[5] * m[7]) -
						m[1] * (m[3] * m[8] - m[5] * m[6]) +
						m[2] * (m[3] * m[7] - m[4] * m[6]);

					float sign = ((row + col) % 2 == 0) ? 1.0f : -1.0f;
					result.at(col, row) = sign * det3 * invDet;
				}
			}

			return result;
#endif
		}

		void invert() noexcept
		{
			*this = inverted();
		}

		// --- Статические фабричные методы ---

		// Матрица переноса
		static matrix4x4 translation(float x, float y, float z) noexcept
		{
			matrix4x4 result;
			result.columns[3] = vector4(x, y, z, 1.0f);
			return result;
		}

		static matrix4x4 translation(const vector4& v) noexcept
		{
			return translation(v[0], v[1], v[2]);
		}

		// Матрица масштабирования
		static matrix4x4 scale(float x, float y, float z) noexcept
		{
			matrix4x4 result;
			result.columns[0][0] = x;
			result.columns[1][1] = y;
			result.columns[2][2] = z;
			return result;
		}

		static matrix4x4 scale(float uniform) noexcept
		{
			return scale(uniform, uniform, uniform);
		}

		static matrix4x4 scale(const vector4& v) noexcept
		{
			return scale(v[0], v[1], v[2]);
		}

		// Вращение вокруг оси X
		static matrix4x4 rotationX(float angleRadians) noexcept
		{
			float c = std::cos(angleRadians);
			float s = std::sin(angleRadians);

			matrix4x4 result;
			result.columns[1][1] = c;
			result.columns[1][2] = s;
			result.columns[2][1] = -s;
			result.columns[2][2] = c;
			return result;
		}

		// Вращение вокруг оси Y
		static matrix4x4 rotationY(float angleRadians) noexcept
		{
			float c = std::cos(angleRadians);
			float s = std::sin(angleRadians);

			matrix4x4 result;
			result.columns[0][0] = c;
			result.columns[0][2] = -s;
			result.columns[2][0] = s;
			result.columns[2][2] = c;
			return result;
		}

		// Вращение вокруг оси Z
		static matrix4x4 rotationZ(float angleRadians) noexcept
		{
			float c = std::cos(angleRadians);
			float s = std::sin(angleRadians);

			matrix4x4 result;
			result.columns[0][0] = c;
			result.columns[0][1] = s;
			result.columns[1][0] = -s;
			result.columns[1][1] = c;
			return result;
		}

		// Вращение вокруг произвольной оси
		static matrix4x4 rotation(const vector4& axis, float angleRadians) noexcept
		{
			vector4 a = axis.normalized();
			float c = std::cos(angleRadians);
			float s = std::sin(angleRadians);
			float t = 1.0f - c;

			float x = a[0], y = a[1], z = a[2];

			return matrix4x4(
				t * x * x + c, t * x * y + z * s, t * x * z - y * s, 0.0f,
				t * x * y - z * s, t * y * y + c, t * y * z + x * s, 0.0f,
				t * x * z + y * s, t * y * z - x * s, t * z * z + c, 0.0f,
				0.0f, 0.0f, 0.0f, 1.0f
			);
		}

		// Look-At матрица (камера)
		static matrix4x4 lookAt(const vector4& eye, const vector4& target, const vector4& up) noexcept
		{
#if defined(ZRENDER_API_D3D12)
			// DirectX использует left-handed систему координат
			vector4 zAxis = (target - eye).normalized();  // forward
			vector4 xAxis = cross3(up, zAxis).normalized();  // right
			vector4 yAxis = cross3(zAxis, xAxis);  // up

			matrix4x4 result;
			// Строим матрицу напрямую в row-major порядке для DirectX
			result.at(0, 0) = xAxis[0];
			result.at(0, 1) = yAxis[0];
			result.at(0, 2) = zAxis[0];
			result.at(0, 3) = 0.0f;

			result.at(1, 0) = xAxis[1];
			result.at(1, 1) = yAxis[1];
			result.at(1, 2) = zAxis[1];
			result.at(1, 3) = 0.0f;

			result.at(2, 0) = xAxis[2];
			result.at(2, 1) = yAxis[2];
			result.at(2, 2) = zAxis[2];
			result.at(2, 3) = 0.0f;

			result.at(3, 0) = -xAxis.dot(eye);
			result.at(3, 1) = -yAxis.dot(eye);
			result.at(3, 2) = -zAxis.dot(eye);
			result.at(3, 3) = 1.0f;
#else
			// Metal / Vulkan используют right-handed систему координат
			vector4 zAxis = (eye - target).normalized();  // forward
			vector4 xAxis = cross3(up, zAxis).normalized();  // right
			vector4 yAxis = cross3(zAxis, xAxis);  // up

			matrix4x4 result;
			result.columns[0] = vector4(xAxis[0], yAxis[0], zAxis[0], 0.0f);
			result.columns[1] = vector4(xAxis[1], yAxis[1], zAxis[1], 0.0f);
			result.columns[2] = vector4(xAxis[2], yAxis[2], zAxis[2], 0.0f);
			result.columns[3] = vector4(-xAxis.dot(eye), -yAxis.dot(eye), -zAxis.dot(eye), 1.0f);
#endif

			return finalize_for_api(result);
		}

		// Перспективная проекция
		static matrix4x4 perspective(float fovYRadians, float aspect, float nearZ, float farZ) noexcept
		{
			float tanHalfFov = std::tan(fovYRadians / 2.0f);

			matrix4x4 result(0.0f);
			result.at(0, 0) = 1.0f / (aspect * tanHalfFov);
			result.at(1, 1) = 1.0f / tanHalfFov;
#if defined(ZRENDER_API_D3D12)
			// Left-Handed (DirectX) - строим в column-major, потом транспонируем
			result.at(2, 2) = farZ / (farZ - nearZ);
			result.at(3, 2) = -(nearZ * farZ) / (farZ - nearZ);
			result.at(2, 3) = 1.0f;
			result.at(3, 3) = 0.0f;
#else
			// Right-Handed (Metal/Vulkan) - остаётся column-major
			result.at(2, 2) = -(farZ + nearZ) / (farZ - nearZ);
			result.at(3, 2) = -(2.0f * farZ * nearZ) / (farZ - nearZ);
			result.at(2, 3) = -1.0f;
#endif

			return finalize_for_api(result);
		}

		static matrix4x4 orthographic(float left, float right, float bottom,
			float top, float nearZ, float farZ) noexcept
		{
			matrix4x4 result(0.0f);
			result.at(0, 0) = 2.0f / (right - left);
			result.at(1, 1) = 2.0f / (top - bottom);

#if defined(ZRENDER_API_D3D12)
			// DirectX: Z от 0 до 1
			result.at(2, 2) = 1.0f / (farZ - nearZ);
			result.at(3, 0) = -(right + left) / (right - left);
			result.at(3, 1) = -(top + bottom) / (top - bottom);
			result.at(3, 2) = -nearZ / (farZ - nearZ);
			result.at(3, 3) = 1.0f;
#else
			// Vulkan/Metal: Z от -1 до 1
			result.at(2, 2) = -2.0f / (farZ - nearZ);
			result.at(3, 0) = -(right + left) / (right - left);
			result.at(3, 1) = -(top + bottom) / (top - bottom);
			result.at(3, 2) = -(farZ + nearZ) / (farZ - nearZ);
			result.at(3, 3) = 1.0f;
#endif

			return finalize_for_api(result);
		}

		// Сравнение
		bool operator==(const matrix4x4& rhs) const noexcept
		{
			return columns[0] == rhs.columns[0] &&
				columns[1] == rhs.columns[1] &&
				columns[2] == rhs.columns[2] &&
				columns[3] == rhs.columns[3];
		}

		bool operator!=(const matrix4x4& rhs) const noexcept
		{
			return !(*this == rhs);
		}

		// Строковое представление
		std::string to_string() const
		{
			std::string result = "[\n";
			for (int row = 0; row < 4; ++row)
			{
				result += "  [";
				for (int col = 0; col < 4; ++col)
				{
					result += std::to_string(at(row, col));
					if (col < 3) result += ", ";
				}
				result += "]";
				if (row < 3) result += ",";
				result += "\n";
			}
			result += "]";
			return result;
		}

	private:
		// Вспомогательная функция для cross product (только для 3D векторов)
		static vector4 cross3(const vector4& a, const vector4& b) noexcept
		{
			return vector4(
				a[1] * b[2] - a[2] * b[1],
				a[2] * b[0] - a[0] * b[2],
				a[0] * b[1] - a[1] * b[0],
				0.0f
			);
		}

		static matrix4x4 finalize_for_api(const matrix4x4& m) noexcept
		{
#if defined(ZRENDER_API_D3D12)
			return m.transposed();
#else
			return m;
#endif
		}
	};

	// --- Операции с float слева ---
	inline matrix4x4 operator*(float s, const matrix4x4& m) noexcept { return m * s; }

	// --- Вывод в поток ---
	inline std::ostream& operator<<(std::ostream& os, const matrix4x4& m)
	{
		return os << m.to_string();
	}
}