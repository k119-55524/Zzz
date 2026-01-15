#include "../headers/headerSIMD.h"

export module Matrix4x4;
import Vector4;

export namespace zzz::math
{
	// Row-major
	struct alignas(16) Matrix4x4
	{
		Vector4 rows[4];

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

		Vector4& operator[](int i) noexcept { return rows[i]; }
		const Vector4& operator[](int i) const noexcept { return rows[i]; }

		float& at(size_t row, size_t col) noexcept { return rows[row][col]; }
		const float& at(size_t row, size_t col) const noexcept { return rows[row][col]; }

		inline Matrix4x4 inverted() const noexcept
		{
#if defined(__APPLE__)
			// Apple имеет встроенную функцию инверсии
			simd_float4x4 mat;
			mat.columns[0] = rows[0].data;
			mat.columns[1] = rows[1].data;
			mat.columns[2] = rows[2].data;
			mat.columns[3] = rows[3].data;

			simd_float4x4 inv = ::simd::inverse(mat);

			return Matrix4x4(
				Vector4(inv.columns[0]),
				Vector4(inv.columns[1]),
				Vector4(inv.columns[2]),
				Vector4(inv.columns[3])
			);

#elif defined(_M_X64) || defined(__x86_64__)
			// SSE оптимизированная инверсия (Intel метод)
			__m128 row0 = rows[0].data;
			__m128 row1 = rows[1].data;
			__m128 row2 = rows[2].data;
			__m128 row3 = rows[3].data;

			// Транспонируем для column-major обработки
			_MM_TRANSPOSE4_PS(row0, row1, row2, row3);

			// Вычисляем пары для первых 8 элементов (кофакторов)
			__m128 tmp1, minor0, minor1, minor2, minor3;

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
			__m128 det = _mm_mul_ps(row0, minor0);
			det = _mm_add_ps(_mm_shuffle_ps(det, det, 0x4E), det);
			det = _mm_add_ss(_mm_shuffle_ps(det, det, 0xB1), det);

			// Проверяем на вырожденность
			float det_val = _mm_cvtss_f32(det);
			if (std::abs(det_val) < 1e-8f)
				return Matrix4x4(); // identity

			// Делим на детерминант
			det = _mm_div_ps(_mm_set1_ps(1.0f), det);
			minor0 = _mm_mul_ps(minor0, det);
			minor1 = _mm_mul_ps(minor1, det);
			minor2 = _mm_mul_ps(minor2, det);
			minor3 = _mm_mul_ps(minor3, det);

			// Транспонируем обратно для row-major
			_MM_TRANSPOSE4_PS(minor0, minor1, minor2, minor3);

			return Matrix4x4(Vector4(minor0), Vector4(minor1), Vector4(minor2), Vector4(minor3));

#elif defined(_M_ARM64) || defined(__aarch64__)
			// NEON версия - используем скалярный алгоритм, т.к. NEON инверсия сложна
			// Можно оптимизировать позже, если критично для производительности
			const float* m = &rows[0][0];
			float inv[16];

			// Кофакторы (ваш оригинальный код)
			inv[0] = m[5] * m[10] * m[15] - m[5] * m[11] * m[14] - m[9] * m[6] * m[15] +
				m[9] * m[7] * m[14] + m[13] * m[6] * m[11] - m[13] * m[7] * m[10];
			inv[4] = -m[4] * m[10] * m[15] + m[4] * m[11] * m[14] + m[8] * m[6] * m[15] -
				m[8] * m[7] * m[14] - m[12] * m[6] * m[11] + m[12] * m[7] * m[10];
			inv[8] = m[4] * m[9] * m[15] - m[4] * m[11] * m[13] - m[8] * m[5] * m[15] +
				m[8] * m[7] * m[13] + m[12] * m[5] * m[11] - m[12] * m[7] * m[9];
			inv[12] = -m[4] * m[9] * m[14] + m[4] * m[10] * m[13] + m[8] * m[5] * m[14] -
				m[8] * m[6] * m[13] - m[12] * m[5] * m[10] + m[12] * m[6] * m[9];
			inv[1] = -m[1] * m[10] * m[15] + m[1] * m[11] * m[14] + m[9] * m[2] * m[15] -
				m[9] * m[3] * m[14] - m[13] * m[2] * m[11] + m[13] * m[3] * m[10];
			inv[5] = m[0] * m[10] * m[15] - m[0] * m[11] * m[14] - m[8] * m[2] * m[15] +
				m[8] * m[3] * m[14] + m[12] * m[2] * m[11] - m[12] * m[3] * m[10];
			inv[9] = -m[0] * m[9] * m[15] + m[0] * m[11] * m[13] + m[8] * m[1] * m[15] -
				m[8] * m[3] * m[13] - m[12] * m[1] * m[11] + m[12] * m[3] * m[9];
			inv[13] = m[0] * m[9] * m[14] - m[0] * m[10] * m[13] - m[8] * m[1] * m[14] +
				m[8] * m[2] * m[13] + m[12] * m[1] * m[10] - m[12] * m[2] * m[9];
			inv[2] = m[1] * m[6] * m[15] - m[1] * m[7] * m[14] - m[5] * m[2] * m[15] +
				m[5] * m[3] * m[14] + m[13] * m[2] * m[7] - m[13] * m[3] * m[6];
			inv[6] = -m[0] * m[6] * m[15] + m[0] * m[7] * m[14] + m[4] * m[2] * m[15] -
				m[4] * m[3] * m[14] - m[12] * m[2] * m[7] + m[12] * m[3] * m[6];
			inv[10] = m[0] * m[5] * m[15] - m[0] * m[7] * m[13] - m[4] * m[1] * m[15] +
				m[4] * m[3] * m[13] + m[12] * m[1] * m[7] - m[12] * m[3] * m[5];
			inv[14] = -m[0] * m[5] * m[14] + m[0] * m[6] * m[13] + m[4] * m[1] * m[14] -
				m[4] * m[2] * m[13] - m[12] * m[1] * m[6] + m[12] * m[2] * m[5];
			inv[3] = -m[1] * m[6] * m[11] + m[1] * m[7] * m[10] + m[5] * m[2] * m[11] -
				m[5] * m[3] * m[10] - m[9] * m[2] * m[7] + m[9] * m[3] * m[6];
			inv[7] = m[0] * m[6] * m[11] - m[0] * m[7] * m[10] - m[4] * m[2] * m[11] +
				m[4] * m[3] * m[10] + m[8] * m[2] * m[7] - m[8] * m[3] * m[6];
			inv[11] = -m[0] * m[5] * m[11] + m[0] * m[7] * m[9] + m[4] * m[1] * m[11] -
				m[4] * m[3] * m[9] - m[8] * m[1] * m[7] + m[8] * m[3] * m[5];
			inv[15] = m[0] * m[5] * m[10] - m[0] * m[6] * m[9] - m[4] * m[1] * m[10] +
				m[4] * m[2] * m[9] + m[8] * m[1] * m[6] - m[8] * m[2] * m[5];

			// Детерминант
			float det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];
			if (std::abs(det) < 1e-8f)
				return Matrix4x4(); // identity

			// Оптимизируем деление через NEON
			float32x4_t vdet = vdupq_n_f32(1.0f / det);
			float32x4_t row0_inv = vld1q_f32(&inv[0]);
			float32x4_t row1_inv = vld1q_f32(&inv[4]);
			float32x4_t row2_inv = vld1q_f32(&inv[8]);
			float32x4_t row3_inv = vld1q_f32(&inv[12]);

			row0_inv = vmulq_f32(row0_inv, vdet);
			row1_inv = vmulq_f32(row1_inv, vdet);
			row2_inv = vmulq_f32(row2_inv, vdet);
			row3_inv = vmulq_f32(row3_inv, vdet);

			return Matrix4x4(
				Vector4(row0_inv),
				Vector4(row1_inv),
				Vector4(row2_inv),
				Vector4(row3_inv)
			);
#endif
		}

		static Matrix4x4 translation(float x, float y, float z) noexcept
		{
			return Matrix4x4(
				1.0f, 0.0f, 0.0f, 0.0f,
				0.0f, 1.0f, 0.0f, 0.0f,
				0.0f, 0.0f, 1.0f, 0.0f,
				x, y, z, 1.0f
			);
		}

		static Matrix4x4 translation(const Vector4& v) noexcept
		{
			return translation(v.x(), v.y(), v.z());
		}

		static Matrix4x4 scale(float sx, float sy, float sz) noexcept
		{
			return Matrix4x4(
				sx, 0.0f, 0.0f, 0.0f,
				0.0f, sy, 0.0f, 0.0f,
				0.0f, 0.0f, sz, 0.0f,
				0.0f, 0.0f, 0.0f, 1.0f
			);
		}

		static Matrix4x4 scale(const Vector4& v) noexcept
		{
			return scale(v.x(), v.y(), v.z());
		}


		static Matrix4x4 rotationX(float angle) noexcept
		{
			const float c = std::cos(angle);
			const float s = std::sin(angle);

			return Matrix4x4(
				1, 0, 0, 0,
				0, c, s, 0,
				0, -s, c, 0,
				0, 0, 0, 1
			);
		}

		static Matrix4x4 rotationY(float angle) noexcept
		{
			const float c = std::cos(angle);
			const float s = std::sin(angle);

			return Matrix4x4(
				c, 0, -s, 0,
				0, 1, 0, 0,
				s, 0, c, 0,
				0, 0, 0, 1
			);
		}

		static Matrix4x4 rotationZ(float angle) noexcept
		{
			const float c = std::cos(angle);
			const float s = std::sin(angle);

			return Matrix4x4(
				c, s, 0, 0,
				-s, c, 0, 0,
				0, 0, 1, 0,
				0, 0, 0, 1
			);
		}

		static Matrix4x4 rotation(const Vector4& axis, float angleRadians) noexcept
		{
			// Нормализация оси
			const float x = axis.x();
			const float y = axis.y();
			const float z = axis.z();

			const float lenSq = x * x + y * y + z * z;
			if (lenSq == 0.0f)
				return Matrix4x4(); // identity

			const float invLen = 1.0f / std::sqrt(lenSq);
			const float nx = x * invLen;
			const float ny = y * invLen;
			const float nz = z * invLen;

			const float c = std::cos(angleRadians);
			const float s = std::sin(angleRadians);
			const float t = 1.0f - c;

			// Row-major, row-vector, LH (DirectX)
			return Matrix4x4(
				t * nx * nx + c, t * nx * ny + s * nz, t * nx * nz - s * ny, 0.0f,
				t * nx * ny - s * nz, t * ny * ny + c, t * ny * nz + s * nx, 0.0f,
				t * nx * nz + s * ny, t * ny * nz - s * nx, t * nz * nz + c, 0.0f,
				0.0f, 0.0f, 0.0f, 1.0f
			);
		}

		// LookAtLH
		static Matrix4x4 lookAt(const Vector4& eye, const Vector4& target, const Vector4& up) noexcept
		{
			Vector4 forward = (target - eye).normalized();      // R2 (Z axis)
			Vector4 right = up.cross3(forward).normalized();    // R0 (X axis)
			Vector4 upVec = forward.cross3(right);              // R1 (Y axis)

			auto dot = [](const Vector4& a, const Vector4& b) {
				return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
				};

			// DirectX создает матрицу с векторами как строками и dot в 4-м столбце,
			// потом транспонирует. Для row-major это означает:
			// базисные векторы должны быть СТОЛБЦАМИ в финальной матрице
			return Matrix4x4(
				right.x(), upVec.x(), forward.x(), 0.0f,
				right.y(), upVec.y(), forward.y(), 0.0f,
				right.z(), upVec.z(), forward.z(), 0.0f,
				-dot(right, eye), -dot(upVec, eye), -dot(forward, eye), 1.0f
			);
		}

		// perspectiveFovLH
		static Matrix4x4 perspective(
			float fovYRadians,
			float aspectRatio,
			float zn,
			float zf) noexcept
		{
			const float yScale = 1.0f / std::tan(fovYRadians * 0.5f);
			const float xScale = yScale / aspectRatio;

			return Matrix4x4(
				xScale, 0.0f, 0.0f, 0.0f,
				0.0f, yScale, 0.0f, 0.0f,
				0.0f, 0.0f, zf / (zf - zn), 1.0f,
				0.0f, 0.0f, -zn * zf / (zf - zn), 0.0f
			);
		}

		// OrthographicLH
		static Matrix4x4 orthographic(
			float left, float right,
			float bottom, float top,
			float zn, float zf) noexcept
		{
			const float invWidth = 1.0f / (right - left);
			const float invHeight = 1.0f / (top - bottom);
			const float invDepth = 1.0f / (zf - zn);

			const float sx = 2.0f * invWidth;
			const float sy = 2.0f * invHeight;
			const float sz = 1.0f * invDepth; // LH, +Z вперёд

			const float tx = -(right + left) * invWidth;
			const float ty = -(top + bottom) * invHeight;
			const float tz = -zn * invDepth;

			return Matrix4x4(
				sx, 0.0f, 0.0f, 0.0f,
				0.0f, sy, 0.0f, 0.0f,
				0.0f, 0.0f, sz, 0.0f,
				tx, ty, tz, 1.0f
			);
		}

		inline Matrix4x4 operator*(const Matrix4x4& rhs) const noexcept
		{
#if defined(__APPLE__)
			// Apple: используем встроенное умножение матриц
			simd_float4x4 lhs_mat, rhs_mat;
			lhs_mat.columns[0] = rows[0].data;
			lhs_mat.columns[1] = rows[1].data;
			lhs_mat.columns[2] = rows[2].data;
			lhs_mat.columns[3] = rows[3].data;

			rhs_mat.columns[0] = rhs.rows[0].data;
			rhs_mat.columns[1] = rhs.rows[1].data;
			rhs_mat.columns[2] = rhs.rows[2].data;
			rhs_mat.columns[3] = rhs.rows[3].data;

			simd_float4x4 result_mat = ::simd::matrix_multiply(lhs_mat, rhs_mat);

			return Matrix4x4(
				Vector4{ result_mat.columns[0] },
				Vector4{ result_mat.columns[1] },
				Vector4{ result_mat.columns[2] },
				Vector4{ result_mat.columns[3] }
			);

#elif defined(_M_X64) || defined(__x86_64__)
			// SSE: умножение row-major матриц
			Matrix4x4 result;

			// Row 0
			{
				__m128 lhs_row = rows[0].data;
				__m128 brod0 = _mm_shuffle_ps(lhs_row, lhs_row, _MM_SHUFFLE(0, 0, 0, 0));
				__m128 brod1 = _mm_shuffle_ps(lhs_row, lhs_row, _MM_SHUFFLE(1, 1, 1, 1));
				__m128 brod2 = _mm_shuffle_ps(lhs_row, lhs_row, _MM_SHUFFLE(2, 2, 2, 2));
				__m128 brod3 = _mm_shuffle_ps(lhs_row, lhs_row, _MM_SHUFFLE(3, 3, 3, 3));

				result.rows[0].data = _mm_add_ps(
					_mm_add_ps(_mm_mul_ps(brod0, rhs.rows[0].data), _mm_mul_ps(brod1, rhs.rows[1].data)),
					_mm_add_ps(_mm_mul_ps(brod2, rhs.rows[2].data), _mm_mul_ps(brod3, rhs.rows[3].data))
				);
			}

			// Row 1
			{
				__m128 lhs_row = rows[1].data;
				__m128 brod0 = _mm_shuffle_ps(lhs_row, lhs_row, _MM_SHUFFLE(0, 0, 0, 0));
				__m128 brod1 = _mm_shuffle_ps(lhs_row, lhs_row, _MM_SHUFFLE(1, 1, 1, 1));
				__m128 brod2 = _mm_shuffle_ps(lhs_row, lhs_row, _MM_SHUFFLE(2, 2, 2, 2));
				__m128 brod3 = _mm_shuffle_ps(lhs_row, lhs_row, _MM_SHUFFLE(3, 3, 3, 3));

				result.rows[1].data = _mm_add_ps(
					_mm_add_ps(_mm_mul_ps(brod0, rhs.rows[0].data), _mm_mul_ps(brod1, rhs.rows[1].data)),
					_mm_add_ps(_mm_mul_ps(brod2, rhs.rows[2].data), _mm_mul_ps(brod3, rhs.rows[3].data))
				);
			}

			// Row 2
			{
				__m128 lhs_row = rows[2].data;
				__m128 brod0 = _mm_shuffle_ps(lhs_row, lhs_row, _MM_SHUFFLE(0, 0, 0, 0));
				__m128 brod1 = _mm_shuffle_ps(lhs_row, lhs_row, _MM_SHUFFLE(1, 1, 1, 1));
				__m128 brod2 = _mm_shuffle_ps(lhs_row, lhs_row, _MM_SHUFFLE(2, 2, 2, 2));
				__m128 brod3 = _mm_shuffle_ps(lhs_row, lhs_row, _MM_SHUFFLE(3, 3, 3, 3));

				result.rows[2].data = _mm_add_ps(
					_mm_add_ps(_mm_mul_ps(brod0, rhs.rows[0].data), _mm_mul_ps(brod1, rhs.rows[1].data)),
					_mm_add_ps(_mm_mul_ps(brod2, rhs.rows[2].data), _mm_mul_ps(brod3, rhs.rows[3].data))
				);
			}

			// Row 3
			{
				__m128 lhs_row = rows[3].data;
				__m128 brod0 = _mm_shuffle_ps(lhs_row, lhs_row, _MM_SHUFFLE(0, 0, 0, 0));
				__m128 brod1 = _mm_shuffle_ps(lhs_row, lhs_row, _MM_SHUFFLE(1, 1, 1, 1));
				__m128 brod2 = _mm_shuffle_ps(lhs_row, lhs_row, _MM_SHUFFLE(2, 2, 2, 2));
				__m128 brod3 = _mm_shuffle_ps(lhs_row, lhs_row, _MM_SHUFFLE(3, 3, 3, 3));

				result.rows[3].data = _mm_add_ps(
					_mm_add_ps(_mm_mul_ps(brod0, rhs.rows[0].data), _mm_mul_ps(brod1, rhs.rows[1].data)),
					_mm_add_ps(_mm_mul_ps(brod2, rhs.rows[2].data), _mm_mul_ps(brod3, rhs.rows[3].data))
				);
			}

			return result;

#elif defined(_M_ARM64) || defined(__aarch64__)
			// NEON: умножение row-major матриц
			Matrix4x4 result;

			// Для каждой строки lhs
			for (int i = 0; i < 4; ++i)
			{
				float32x4_t lhs_row = rows[i].data;

				// Broadcast каждого элемента строки
				float32x4_t brod0 = vdupq_laneq_f32(lhs_row, 0);
				float32x4_t brod1 = vdupq_laneq_f32(lhs_row, 1);
				float32x4_t brod2 = vdupq_laneq_f32(lhs_row, 2);
				float32x4_t brod3 = vdupq_laneq_f32(lhs_row, 3);

				// Умножаем и складываем
				float32x4_t row0 = vmulq_f32(brod0, rhs.rows[0].data);
				float32x4_t row1 = vmulq_f32(brod1, rhs.rows[1].data);
				float32x4_t row2 = vmulq_f32(brod2, rhs.rows[2].data);
				float32x4_t row3 = vmulq_f32(brod3, rhs.rows[3].data);

				float32x4_t sum01 = vaddq_f32(row0, row1);
				float32x4_t sum23 = vaddq_f32(row2, row3);
				result.rows[i].data = vaddq_f32(sum01, sum23);
			}

			return result;
#endif
		}
	};

	inline Vector4 operator*(const Vector4& v, const Matrix4x4& m) noexcept
	{
#if defined(__APPLE__)
		// Apple: используем встроенное умножение
		simd_float4x4 mat;
		mat.columns[0] = m.rows[0].data;
		mat.columns[1] = m.rows[1].data;
		mat.columns[2] = m.rows[2].data;
		mat.columns[3] = m.rows[3].data;

		// Для row-vector ? matrix нужно транспонировать или использовать другой порядок
		simd_float4 result = ::simd::float4{ 0, 0, 0, 0 };
		result += v.data.x * mat.columns[0];
		result += v.data.y * mat.columns[1];
		result += v.data.z * mat.columns[2];
		result += v.data.w * mat.columns[3];

		return Vector4{ result };

#elif defined(_M_X64) || defined(__x86_64__)
		// SSE: broadcast каждого компонента вектора и умножаем на строки матрицы
		__m128 vec = v.data;

		// Broadcast каждого компонента
		__m128 brod_x = _mm_shuffle_ps(vec, vec, _MM_SHUFFLE(0, 0, 0, 0));
		__m128 brod_y = _mm_shuffle_ps(vec, vec, _MM_SHUFFLE(1, 1, 1, 1));
		__m128 brod_z = _mm_shuffle_ps(vec, vec, _MM_SHUFFLE(2, 2, 2, 2));
		__m128 brod_w = _mm_shuffle_ps(vec, vec, _MM_SHUFFLE(3, 3, 3, 3));

		// Умножаем на соответствующие строки матрицы
		__m128 mul_x = _mm_mul_ps(brod_x, m.rows[0].data);
		__m128 mul_y = _mm_mul_ps(brod_y, m.rows[1].data);
		__m128 mul_z = _mm_mul_ps(brod_z, m.rows[2].data);
		__m128 mul_w = _mm_mul_ps(brod_w, m.rows[3].data);

		// Складываем все результаты
		__m128 sum_xy = _mm_add_ps(mul_x, mul_y);
		__m128 sum_zw = _mm_add_ps(mul_z, mul_w);
		__m128 result = _mm_add_ps(sum_xy, sum_zw);

		return Vector4{ result };

#elif defined(_M_ARM64) || defined(__aarch64__)
		// NEON: broadcast каждого компонента вектора
		float32x4_t vec = v.data;

		// Broadcast каждого компонента
		float32x4_t brod_x = vdupq_laneq_f32(vec, 0);
		float32x4_t brod_y = vdupq_laneq_f32(vec, 1);
		float32x4_t brod_z = vdupq_laneq_f32(vec, 2);
		float32x4_t brod_w = vdupq_laneq_f32(vec, 3);

		// Умножаем на соответствующие строки матрицы
		float32x4_t mul_x = vmulq_f32(brod_x, m.rows[0].data);
		float32x4_t mul_y = vmulq_f32(brod_y, m.rows[1].data);
		float32x4_t mul_z = vmulq_f32(brod_z, m.rows[2].data);
		float32x4_t mul_w = vmulq_f32(brod_w, m.rows[3].data);

		// Складываем все результаты
		float32x4_t sum_xy = vaddq_f32(mul_x, mul_y);
		float32x4_t sum_zw = vaddq_f32(mul_z, mul_w);
		float32x4_t result = vaddq_f32(sum_xy, sum_zw);

		return Vector4{ result };
#endif
	}
}