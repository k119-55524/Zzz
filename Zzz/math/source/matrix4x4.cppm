#include "../headers/headerSIMD.h"

export module Matrix4x4;
import Vector4;

export namespace zzz::math
{
	// Row-major
	struct alignas(16) Matrix4x4
	{
		Vector4 rows[4]; // Row-major хранение: rows[i] = i-я строка

		// --- Конструкторы ---

		Matrix4x4() noexcept
		{
			// Единичная матрица
			rows[0] = Vector4(1.0f, 0.0f, 0.0f, 0.0f);
			rows[1] = Vector4(0.0f, 1.0f, 0.0f, 0.0f);
			rows[2] = Vector4(0.0f, 0.0f, 1.0f, 0.0f);
			rows[3] = Vector4(0.0f, 0.0f, 0.0f, 1.0f);
		}

		explicit Matrix4x4(const Vector4& row0, const Vector4& row1,
			const Vector4& row2, const Vector4& row3) noexcept
		{
			rows[0] = row0;
			rows[1] = row1;
			rows[2] = row2;
			rows[3] = row3;
		}

		// Конструктор из 16 значений в row-major порядке
		Matrix4x4(float m00, float m01, float m02, float m03,
			float m10, float m11, float m12, float m13,
			float m20, float m21, float m22, float m23,
			float m30, float m31, float m32, float m33) noexcept
		{
			rows[0] = Vector4(m00, m01, m02, m03);
			rows[1] = Vector4(m10, m11, m12, m13);
			rows[2] = Vector4(m20, m21, m22, m23);
			rows[3] = Vector4(m30, m31, m32, m33);
		}

		explicit Matrix4x4(float diagonal) noexcept
		{
			rows[0] = Vector4(diagonal, 0.0f, 0.0f, 0.0f);
			rows[1] = Vector4(0.0f, diagonal, 0.0f, 0.0f);
			rows[2] = Vector4(0.0f, 0.0f, diagonal, 0.0f);
			rows[3] = Vector4(0.0f, 0.0f, 0.0f, diagonal);
		}

		// --- Доступ к элементам ---

		Vector4& operator[](size_t row) noexcept { return rows[row]; }
		const Vector4& operator[](size_t row) const noexcept { return rows[row]; }

		float& at(size_t row, size_t col) noexcept { return rows[row][col]; }
		const float& at(size_t row, size_t col) const noexcept { return rows[row][col]; }

		Vector4& row(size_t index) noexcept { return rows[index]; }
		const Vector4& row(size_t index) const noexcept { return rows[index]; }

		// Доступ к столбцу (SIMD оптимизированный для всех платформ)
		Vector4 column(size_t index) const noexcept
		{
#if defined(__APPLE__)
			// Metal/SIMD approach
			return Vector4(rows[0][index], rows[1][index], rows[2][index], rows[3][index]);
#elif defined(_M_X64) || defined(__x86_64__)
			// SSE shuffle для извлечения столбца
			alignas(16) float temp[4];
			temp[0] = rows[0][index];
			temp[1] = rows[1][index];
			temp[2] = rows[2][index];
			temp[3] = rows[3][index];
			Vector4 result;
			result.data = _mm_load_ps(temp);
			return result;
#elif defined(_M_ARM64) || defined(__aarch64__)
			// NEON approach
			return Vector4(rows[0][index], rows[1][index], rows[2][index], rows[3][index]);
#endif
		}

		void setColumn(size_t index, const Vector4& v) noexcept
		{
			rows[0][index] = v[0];
			rows[1][index] = v[1];
			rows[2][index] = v[2];
			rows[3][index] = v[3];
		}

		void setRow(size_t index, const Vector4& v) noexcept
		{
			rows[index] = v;
		}

		// --- Арифметика (SIMD оптимизированная) ---

		Matrix4x4 operator+(const Matrix4x4& rhs) const noexcept
		{
			return Matrix4x4(
				rows[0] + rhs.rows[0],
				rows[1] + rhs.rows[1],
				rows[2] + rhs.rows[2],
				rows[3] + rhs.rows[3]
			);
		}

		Matrix4x4 operator-(const Matrix4x4& rhs) const noexcept
		{
			return Matrix4x4(
				rows[0] - rhs.rows[0],
				rows[1] - rhs.rows[1],
				rows[2] - rhs.rows[2],
				rows[3] - rhs.rows[3]
			);
		}

		Matrix4x4 operator*(float scalar) const noexcept
		{
			return Matrix4x4(
				rows[0] * scalar,
				rows[1] * scalar,
				rows[2] * scalar,
				rows[3] * scalar
			);
		}

		// Умножение матриц - SIMD оптимизированное для всех платформ
		Matrix4x4 operator*(const Matrix4x4& rhs) const noexcept
		{
			Matrix4x4 result;

#if defined(__APPLE__)
			// Metal/SIMD оптимизация
			for (int i = 0; i < 4; ++i)
			{
				simd_float4 row = rows[i].data;
				simd_float4 r0 = row[0] * rhs.rows[0].data;
				simd_float4 r1 = row[1] * rhs.rows[1].data;
				simd_float4 r2 = row[2] * rhs.rows[2].data;
				simd_float4 r3 = row[3] * rhs.rows[3].data;
				result.rows[i].data = r0 + r1 + r2 + r3;
			}
#elif defined(_M_X64) || defined(__x86_64__)
			// SSE оптимизация
			for (int i = 0; i < 4; ++i)
			{
				__m128 row = rows[i].data;
				__m128 brod0 = _mm_shuffle_ps(row, row, _MM_SHUFFLE(0, 0, 0, 0));
				__m128 brod1 = _mm_shuffle_ps(row, row, _MM_SHUFFLE(1, 1, 1, 1));
				__m128 brod2 = _mm_shuffle_ps(row, row, _MM_SHUFFLE(2, 2, 2, 2));
				__m128 brod3 = _mm_shuffle_ps(row, row, _MM_SHUFFLE(3, 3, 3, 3));

				__m128 res = _mm_mul_ps(brod0, rhs.rows[0].data);
				res = _mm_add_ps(res, _mm_mul_ps(brod1, rhs.rows[1].data));
				res = _mm_add_ps(res, _mm_mul_ps(brod2, rhs.rows[2].data));
				res = _mm_add_ps(res, _mm_mul_ps(brod3, rhs.rows[3].data));

				result.rows[i].data = res;
			}
#elif defined(_M_ARM64) || defined(__aarch64__)
			// NEON оптимизация
			for (int i = 0; i < 4; ++i)
			{
				float32x4_t row = rows[i].data;

				float32x4_t brod0 = vdupq_n_f32(vgetq_lane_f32(row, 0));
				float32x4_t brod1 = vdupq_n_f32(vgetq_lane_f32(row, 1));
				float32x4_t brod2 = vdupq_n_f32(vgetq_lane_f32(row, 2));
				float32x4_t brod3 = vdupq_n_f32(vgetq_lane_f32(row, 3));

				float32x4_t res = vmulq_f32(brod0, rhs.rows[0].data);
				res = vmlaq_f32(res, brod1, rhs.rows[1].data);
				res = vmlaq_f32(res, brod2, rhs.rows[2].data);
				res = vmlaq_f32(res, brod3, rhs.rows[3].data);

				result.rows[i].data = res;
			}
#endif
			return result;
		}

		// Умножение v * M (вектор-строка слева) - SIMD оптимизировано
		friend Vector4 operator*(const Vector4& v, const Matrix4x4& m) noexcept
		{
			Vector4 result;

#if defined(__APPLE__)
			// Metal/SIMD версия
			simd_float4 vdata = v.data;
			result.data = vdata[0] * m.rows[0].data +
				vdata[1] * m.rows[1].data +
				vdata[2] * m.rows[2].data +
				vdata[3] * m.rows[3].data;
#elif defined(_M_X64) || defined(__x86_64__)
			// SSE версия
			__m128 x = _mm_shuffle_ps(v.data, v.data, _MM_SHUFFLE(0, 0, 0, 0));
			__m128 y = _mm_shuffle_ps(v.data, v.data, _MM_SHUFFLE(1, 1, 1, 1));
			__m128 z = _mm_shuffle_ps(v.data, v.data, _MM_SHUFFLE(2, 2, 2, 2));
			__m128 w = _mm_shuffle_ps(v.data, v.data, _MM_SHUFFLE(3, 3, 3, 3));

			__m128 res = _mm_mul_ps(x, m.rows[0].data);
			res = _mm_add_ps(res, _mm_mul_ps(y, m.rows[1].data));
			res = _mm_add_ps(res, _mm_mul_ps(z, m.rows[2].data));
			res = _mm_add_ps(res, _mm_mul_ps(w, m.rows[3].data));

			result.data = res;
#elif defined(_M_ARM64) || defined(__aarch64__)
			// NEON версия
			float32x4_t x = vdupq_n_f32(vgetq_lane_f32(v.data, 0));
			float32x4_t y = vdupq_n_f32(vgetq_lane_f32(v.data, 1));
			float32x4_t z = vdupq_n_f32(vgetq_lane_f32(v.data, 2));
			float32x4_t w = vdupq_n_f32(vgetq_lane_f32(v.data, 3));

			float32x4_t res = vmulq_f32(x, m.rows[0].data);
			res = vmlaq_f32(res, y, m.rows[1].data);
			res = vmlaq_f32(res, z, m.rows[2].data);
			res = vmlaq_f32(res, w, m.rows[3].data);

			result.data = res;
#endif
			return result;
		}

		Vector4 operator*(const Vector4& v) const noexcept
		{
			// Транспонированное умножение
			return Vector4(
				rows[0][0] * v[0] + rows[0][1] * v[1] + rows[0][2] * v[2] + rows[0][3] * v[3],
				rows[1][0] * v[0] + rows[1][1] * v[1] + rows[1][2] * v[2] + rows[1][3] * v[3],
				rows[2][0] * v[0] + rows[2][1] * v[1] + rows[2][2] * v[2] + rows[2][3] * v[3],
				rows[3][0] * v[0] + rows[3][1] * v[1] + rows[3][2] * v[2] + rows[3][3] * v[3]
			);
		}

		Matrix4x4& operator+=(const Matrix4x4& rhs) noexcept
		{
			rows[0] += rhs.rows[0];
			rows[1] += rhs.rows[1];
			rows[2] += rhs.rows[2];
			rows[3] += rhs.rows[3];
			return *this;
		}

		Matrix4x4& operator-=(const Matrix4x4& rhs) noexcept
		{
			rows[0] -= rhs.rows[0];
			rows[1] -= rhs.rows[1];
			rows[2] -= rhs.rows[2];
			rows[3] -= rhs.rows[3];
			return *this;
		}

		Matrix4x4& operator*=(float scalar) noexcept
		{
			rows[0] *= scalar;
			rows[1] *= scalar;
			rows[2] *= scalar;
			rows[3] *= scalar;
			return *this;
		}

		Matrix4x4& operator*=(const Matrix4x4& rhs) noexcept
		{
			*this = *this * rhs;
			return *this;
		}

		// --- Транспонирование (SIMD оптимизированное) ---

		Matrix4x4 transposed() const noexcept
		{
			Matrix4x4 result;

#if defined(__APPLE__)
			// Metal/SIMD transpose
			simd_float4x4 temp;
			temp.columns[0] = rows[0].data;
			temp.columns[1] = rows[1].data;
			temp.columns[2] = rows[2].data;
			temp.columns[3] = rows[3].data;
			simd_float4x4 transposed = ::simd::transpose(temp);
			result.rows[0].data = transposed.columns[0];
			result.rows[1].data = transposed.columns[1];
			result.rows[2].data = transposed.columns[2];
			result.rows[3].data = transposed.columns[3];
#elif defined(_M_X64) || defined(__x86_64__)
			// SSE optimized transpose
			__m128 tmp0 = _mm_unpacklo_ps(rows[0].data, rows[1].data);
			__m128 tmp1 = _mm_unpackhi_ps(rows[0].data, rows[1].data);
			__m128 tmp2 = _mm_unpacklo_ps(rows[2].data, rows[3].data);
			__m128 tmp3 = _mm_unpackhi_ps(rows[2].data, rows[3].data);

			result.rows[0].data = _mm_movelh_ps(tmp0, tmp2);
			result.rows[1].data = _mm_movehl_ps(tmp2, tmp0);
			result.rows[2].data = _mm_movelh_ps(tmp1, tmp3);
			result.rows[3].data = _mm_movehl_ps(tmp3, tmp1);
#elif defined(_M_ARM64) || defined(__aarch64__)
			// NEON transpose
			float32x4x2_t tmp01 = vtrnq_f32(rows[0].data, rows[1].data);
			float32x4x2_t tmp23 = vtrnq_f32(rows[2].data, rows[3].data);

			result.rows[0].data = vcombine_f32(vget_low_f32(tmp01.val[0]), vget_low_f32(tmp23.val[0]));
			result.rows[1].data = vcombine_f32(vget_low_f32(tmp01.val[1]), vget_low_f32(tmp23.val[1]));
			result.rows[2].data = vcombine_f32(vget_high_f32(tmp01.val[0]), vget_high_f32(tmp23.val[0]));
			result.rows[3].data = vcombine_f32(vget_high_f32(tmp01.val[1]), vget_high_f32(tmp23.val[1]));
#endif
			return result;
		}

		void transpose() noexcept { *this = transposed(); }

		// --- Статические фабрики ---

		static Matrix4x4 translation(float x, float y, float z) noexcept
		{
			Matrix4x4 m;
			m.rows[3] = Vector4(x, y, z, 1.0f);
			return m;
		}

		static Matrix4x4 translation(const Vector4& v) noexcept
		{
			return translation(v.x(), v.y(), v.z());
		}

		static Matrix4x4 scale(float x, float y, float z) noexcept
		{
			return Matrix4x4(
				x, 0.0f, 0.0f, 0.0f,
				0.0f, y, 0.0f, 0.0f,
				0.0f, 0.0f, z, 0.0f,
				0.0f, 0.0f, 0.0f, 1.0f);
		}

		static Matrix4x4 scale(float s) noexcept { return scale(s, s, s); }

		static Matrix4x4 scale(const Vector4& v) noexcept
		{
			return scale(v.x(), v.y(), v.z());
		}

		static Matrix4x4 rotationX(float angle) noexcept
		{
			float c = std::cos(angle), s = std::sin(angle);
			return Matrix4x4(
				1.0f, 0.0f, 0.0f, 0.0f,
				0.0f, c, s, 0.0f,
				0.0f, -s, c, 0.0f,
				0.0f, 0.0f, 0.0f, 1.0f
			);
		}

		static Matrix4x4 rotationY(float angle) noexcept
		{
			float c = std::cos(angle), s = std::sin(angle);
			return Matrix4x4(
				c, 0.0f, -s, 0.0f,
				0.0f, 1.0f, 0.0f, 0.0f,
				s, 0.0f, c, 0.0f,
				0.0f, 0.0f, 0.0f, 1.0f
			);
		}

		static Matrix4x4 rotationZ(float angle) noexcept
		{
			float c = std::cos(angle), s = std::sin(angle);
			return Matrix4x4(
				c, s, 0.0f, 0.0f,
				-s, c, 0.0f, 0.0f,
				0.0f, 0.0f, 1.0f, 0.0f,
				0.0f, 0.0f, 0.0f, 1.0f
			);
		}

		static Matrix4x4 rotation(const Vector4& axis, float angleRadians) noexcept
		{
			Vector4 a = axis.normalized();
			float c = std::cos(angleRadians);
			float s = std::sin(angleRadians);
			float t = 1.0f - c;

			float x = a.x();
			float y = a.y();
			float z = a.z();

			return Matrix4x4(
				t * x * x + c, t * x * y + z * s, t * x * z - y * s, 0.0f,
				t * x * y - z * s, t * y * y + c, t * y * z + x * s, 0.0f,
				t * x * z + y * s, t * y * z - x * s, t * z * z + c, 0.0f,
				0.0f, 0.0f, 0.0f, 1.0f
			);
		}

		static Matrix4x4 lookAt(const Vector4& eye, const Vector4& target, const Vector4& up) noexcept
		{
			Vector4 zAxis = (target - eye).normalized();
			Vector4 xAxis = up.cross3(zAxis).normalized();
			Vector4 yAxis = zAxis.cross3(xAxis);

			return Matrix4x4(
				xAxis.x(), xAxis.y(), xAxis.z(), 0.0f,
				yAxis.x(), yAxis.y(), yAxis.z(), 0.0f,
				zAxis.x(), zAxis.y(), zAxis.z(), 0.0f,
				-xAxis.dot(eye), -yAxis.dot(eye), -zAxis.dot(eye), 1.0f
			);
		}

		static Matrix4x4 perspective(float fovY, float aspect, float nearZ, float farZ) noexcept
		{
			float tanHalfFov = std::tan(fovY * 0.5f);
			float f = 1.0f / tanHalfFov;
			float rangeInv = 1.0f / (farZ - nearZ);

			Matrix4x4 m;
			// Row-major формат для DirectX
			m.rows[0] = Vector4(f / aspect, 0.0f, 0.0f, 0.0f);
			m.rows[1] = Vector4(0.0f, f, 0.0f, 0.0f);
			m.rows[2] = Vector4(0.0f, 0.0f, farZ * rangeInv, 1.0f);  // w компонента
			m.rows[3] = Vector4(0.0f, 0.0f, -nearZ * farZ * rangeInv, 0.0f);

			return m;
		}

		static Matrix4x4 orthographic(float left, float right, float bottom, float top, float nearZ, float farZ) noexcept
		{
			return Matrix4x4(
				2.0f / (right - left), 0.0f, 0.0f, 0.0f,
				0.0f, 2.0f / (top - bottom), 0.0f, 0.0f,
				0.0f, 0.0f, 1.0f / (farZ - nearZ), 0.0f,
				-(right + left) / (right - left),
				-(top + bottom) / (top - bottom),
				-nearZ / (farZ - nearZ),
				1.0f
			);
		}

		// --- Детерминант и инверсия ---

		float determinant() const noexcept
		{
			float m00 = rows[0][0], m01 = rows[0][1], m02 = rows[0][2], m03 = rows[0][3];
			float m10 = rows[1][0], m11 = rows[1][1], m12 = rows[1][2], m13 = rows[1][3];
			float m20 = rows[2][0], m21 = rows[2][1], m22 = rows[2][2], m23 = rows[2][3];
			float m30 = rows[3][0], m31 = rows[3][1], m32 = rows[3][2], m33 = rows[3][3];

			float minor00 = m11 * (m22 * m33 - m23 * m32) - m12 * (m21 * m33 - m23 * m31) + m13 * (m21 * m32 - m22 * m31);
			float minor01 = m10 * (m22 * m33 - m23 * m32) - m12 * (m20 * m33 - m23 * m30) + m13 * (m20 * m32 - m22 * m30);
			float minor02 = m10 * (m21 * m33 - m23 * m31) - m11 * (m20 * m33 - m23 * m30) + m13 * (m20 * m31 - m21 * m30);
			float minor03 = m10 * (m21 * m32 - m22 * m31) - m11 * (m20 * m32 - m22 * m30) + m12 * (m20 * m31 - m21 * m30);

			return m00 * minor00 - m01 * minor01 + m02 * minor02 - m03 * minor03;
		}

		Matrix4x4 inverted() const noexcept
		{
			float m00 = rows[0][0], m01 = rows[0][1], m02 = rows[0][2], m03 = rows[0][3];
			float m10 = rows[1][0], m11 = rows[1][1], m12 = rows[1][2], m13 = rows[1][3];
			float m20 = rows[2][0], m21 = rows[2][1], m22 = rows[2][2], m23 = rows[2][3];
			float m30 = rows[3][0], m31 = rows[3][1], m32 = rows[3][2], m33 = rows[3][3];

			float c00 = (m11 * (m22 * m33 - m23 * m32) - m12 * (m21 * m33 - m23 * m31) + m13 * (m21 * m32 - m22 * m31));
			float c01 = -(m01 * (m22 * m33 - m23 * m32) - m02 * (m21 * m33 - m23 * m31) + m03 * (m21 * m32 - m22 * m31));
			float c02 = (m01 * (m12 * m33 - m13 * m32) - m02 * (m11 * m33 - m13 * m31) + m03 * (m11 * m32 - m12 * m31));
			float c03 = -(m01 * (m12 * m23 - m13 * m22) - m02 * (m11 * m23 - m13 * m21) + m03 * (m11 * m22 - m12 * m21));

			float c10 = -(m10 * (m22 * m33 - m23 * m32) - m12 * (m20 * m33 - m23 * m30) + m13 * (m20 * m32 - m22 * m30));
			float c11 = (m00 * (m22 * m33 - m23 * m32) - m02 * (m20 * m33 - m23 * m30) + m03 * (m20 * m32 - m22 * m30));
			float c12 = -(m00 * (m12 * m33 - m13 * m32) - m02 * (m10 * m33 - m13 * m30) + m03 * (m10 * m32 - m12 * m30));
			float c13 = (m00 * (m12 * m23 - m13 * m22) - m02 * (m10 * m23 - m13 * m20) + m03 * (m10 * m22 - m12 * m20));

			float c20 = (m10 * (m21 * m33 - m23 * m31) - m11 * (m20 * m33 - m23 * m30) + m13 * (m20 * m31 - m21 * m30));
			float c21 = -(m00 * (m21 * m33 - m23 * m31) - m01 * (m20 * m33 - m23 * m30) + m03 * (m20 * m31 - m21 * m30));
			float c22 = (m00 * (m11 * m33 - m13 * m31) - m01 * (m10 * m33 - m13 * m30) + m03 * (m10 * m31 - m11 * m30));
			float c23 = -(m00 * (m11 * m23 - m13 * m21) - m01 * (m10 * m23 - m13 * m20) + m03 * (m10 * m21 - m11 * m20));

			float c30 = -(m10 * (m21 * m32 - m22 * m31) - m11 * (m20 * m32 - m22 * m30) + m12 * (m20 * m31 - m21 * m30));
			float c31 = (m00 * (m21 * m32 - m22 * m31) - m01 * (m20 * m32 - m22 * m30) + m02 * (m20 * m31 - m21 * m30));
			float c32 = -(m00 * (m11 * m32 - m12 * m31) - m01 * (m10 * m32 - m12 * m30) + m02 * (m10 * m31 - m11 * m30));
			float c33 = (m00 * (m11 * m22 - m12 * m21) - m01 * (m10 * m22 - m12 * m20) + m02 * (m10 * m21 - m11 * m20));

			float det = m00 * c00 + m01 * c10 + m02 * c20 + m03 * c30;

			if (std::abs(det) < 1e-8f)
				return Matrix4x4();

			float invDet = 1.0f / det;

			return Matrix4x4(
				c00 * invDet, c10 * invDet, c20 * invDet, c30 * invDet,
				c01 * invDet, c11 * invDet, c21 * invDet, c31 * invDet,
				c02 * invDet, c12 * invDet, c22 * invDet, c32 * invDet,
				c03 * invDet, c13 * invDet, c23 * invDet, c33 * invDet
			);
		}

		// --- Утилиты ---

		bool operator==(const Matrix4x4& rhs) const noexcept { return rows[0] == rhs.rows[0] && rows[1] == rhs.rows[1] && rows[2] == rhs.rows[2] && rows[3] == rhs.rows[3]; }
		bool operator!=(const Matrix4x4& rhs) const noexcept { return !(*this == rhs); }

		std::string to_string() const
		{
			std::string s = "[\n";
			for (int r = 0; r < 4; ++r)
			{
				s += " [";
				for (int c = 0; c < 4; ++c)
				{
					s += std::to_string(rows[r][c]);
					if (c < 3) s += ", ";
				}
				s += (r < 3) ? "],\n" : "]";
			}
			s += "\n]";

			return s;
		}
	};

	inline Matrix4x4 operator*(float s, const Matrix4x4& m) noexcept { return m * s; }
	inline std::ostream& operator<<(std::ostream& os, const Matrix4x4& m) { return os << m.to_string(); }
}