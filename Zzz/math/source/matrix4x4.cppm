#include "../headers/headerSIMD.h"

export module Matrix4x4;
import Vector4;

export namespace zzz::math
{
	struct alignas(16) Matrix4x4
	{
		Vector4 columns[4]; // Row-major хранение: columns[i] = i-я строка

		// --- Конструкторы ---

		Matrix4x4() noexcept
		{
			// Единичная матрица
			columns[0] = Vector4(1.0f, 0.0f, 0.0f, 0.0f);
			columns[1] = Vector4(0.0f, 1.0f, 0.0f, 0.0f);
			columns[2] = Vector4(0.0f, 0.0f, 1.0f, 0.0f);
			columns[3] = Vector4(0.0f, 0.0f, 0.0f, 1.0f);
		}

		explicit Matrix4x4(const Vector4& col0, const Vector4& col1,
			const Vector4& col2, const Vector4& col3) noexcept
		{
			columns[0] = col0;
			columns[1] = col1;
			columns[2] = col2;
			columns[3] = col3;
		}

		// Конструктор из 16 значений в row-major порядке (стандарт для DirectX)
		Matrix4x4(float m00, float m01, float m02, float m03,
			float m10, float m11, float m12, float m13,
			float m20, float m21, float m22, float m23,
			float m30, float m31, float m32, float m33) noexcept
		{
			columns[0] = Vector4(m00, m01, m02, m03); // строка 0
			columns[1] = Vector4(m10, m11, m12, m13); // строка 1
			columns[2] = Vector4(m20, m21, m22, m23); // строка 2
			columns[3] = Vector4(m30, m31, m32, m33); // строка 3
		}

		explicit Matrix4x4(float diagonal) noexcept
		{
			columns[0] = Vector4(diagonal, 0.0f, 0.0f, 0.0f);
			columns[1] = Vector4(0.0f, diagonal, 0.0f, 0.0f);
			columns[2] = Vector4(0.0f, 0.0f, diagonal, 0.0f);
			columns[3] = Vector4(0.0f, 0.0f, 0.0f, diagonal);
		}

		// --- Доступ к элементам ---

		Vector4& operator[](size_t row) noexcept { return columns[row]; }
		const Vector4& operator[](size_t row) const noexcept { return columns[row]; }

		float& at(size_t row, size_t col) noexcept { return columns[row][col]; }
		const float& at(size_t row, size_t col) const noexcept { return columns[row][col]; }

		Vector4& row(size_t index) noexcept { return columns[index]; }
		const Vector4& row(size_t index) const noexcept { return columns[index]; }

		// Доступ к столбцу (извлекает столбец из row-major матрицы)
		Vector4 column(size_t index) const noexcept
		{
			return Vector4(
				columns[0][index],
				columns[1][index],
				columns[2][index],
				columns[3][index]
			);
		}

		// Установка столбца
		void setColumn(size_t index, const Vector4& v) noexcept
		{
			columns[0][index] = v[0];
			columns[1][index] = v[1];
			columns[2][index] = v[2];
			columns[3][index] = v[3];
		}

		// Установка строки
		void setRow(size_t index, const Vector4& v) noexcept
		{
			columns[index] = v;
		}

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

		// Умножение матриц - стандартное для row-major
		Matrix4x4 operator*(const Matrix4x4& rhs) const noexcept
		{
			Matrix4x4 result(0.0f);

			for (int row = 0; row < 4; ++row)
			{
				for (int col = 0; col < 4; ++col)
				{
					float sum = 0.0f;
					for (int k = 0; k < 4; ++k)
					{
						// Стандартное умножение: this[row][k] * rhs[k][col]
						sum += columns[row][k] * rhs.columns[k][col];
					}
					result.columns[row][col] = sum;
				}
			}

			return result;
		}

		// Умножение M * v (вектор-столбец справа)
		// Для DirectX row-major с вектором-столбцом
		Vector4 operator*(const Vector4& v) const noexcept
		{
			// Извлекаем столбцы матрицы и умножаем на компоненты вектора
			return Vector4(
				columns[0][0] * v[0] + columns[1][0] * v[1] + columns[2][0] * v[2] + columns[3][0] * v[3],
				columns[0][1] * v[0] + columns[1][1] * v[1] + columns[2][1] * v[2] + columns[3][1] * v[3],
				columns[0][2] * v[0] + columns[1][2] * v[1] + columns[2][2] * v[2] + columns[3][2] * v[3],
				columns[0][3] * v[0] + columns[1][3] * v[1] + columns[2][3] * v[2] + columns[3][3] * v[3]
			);
		}

		// Умножение v * M (вектор-строка слева) - стандарт DirectX
		friend Vector4 operator*(const Vector4& v, const Matrix4x4& m) noexcept
		{
			// Вектор-строка умножается на строки матрицы
			return Vector4(
				v[0] * m.columns[0][0] + v[1] * m.columns[1][0] + v[2] * m.columns[2][0] + v[3] * m.columns[3][0],
				v[0] * m.columns[0][1] + v[1] * m.columns[1][1] + v[2] * m.columns[2][1] + v[3] * m.columns[3][1],
				v[0] * m.columns[0][2] + v[1] * m.columns[1][2] + v[2] * m.columns[2][2] + v[3] * m.columns[3][2],
				v[0] * m.columns[0][3] + v[1] * m.columns[1][3] + v[2] * m.columns[2][3] + v[3] * m.columns[3][3]
			);
		}

		Matrix4x4& operator+=(const Matrix4x4& rhs) noexcept { columns[0] += rhs.columns[0]; columns[1] += rhs.columns[1]; columns[2] += rhs.columns[2]; columns[3] += rhs.columns[3]; return *this; }
		Matrix4x4& operator-=(const Matrix4x4& rhs) noexcept { columns[0] -= rhs.columns[0]; columns[1] -= rhs.columns[1]; columns[2] -= rhs.columns[2]; columns[3] -= rhs.columns[3]; return *this; }
		Matrix4x4& operator*=(float scalar) noexcept { columns[0] *= scalar; columns[1] *= scalar; columns[2] *= scalar; columns[3] *= scalar; return *this; }
		Matrix4x4& operator*=(const Matrix4x4& rhs) noexcept { *this = *this * rhs; return *this; }

		// --- Транспонирование (для row-major -> column-major при передаче в шейдеры, если нужно) ---

		Matrix4x4 transposed() const noexcept
		{
			Matrix4x4 result;
			__m128 tmp0 = _mm_unpacklo_ps(columns[0].data, columns[1].data);
			__m128 tmp1 = _mm_unpackhi_ps(columns[0].data, columns[1].data);
			__m128 tmp2 = _mm_unpacklo_ps(columns[2].data, columns[3].data);
			__m128 tmp3 = _mm_unpackhi_ps(columns[2].data, columns[3].data);

			result.columns[0].data = _mm_movelh_ps(tmp0, tmp2);
			result.columns[1].data = _mm_movehl_ps(tmp2, tmp0);
			result.columns[2].data = _mm_movelh_ps(tmp1, tmp3);
			result.columns[3].data = _mm_movehl_ps(tmp3, tmp1);
			return result;
		}

		void transpose() noexcept { *this = transposed(); }

		// --- Статические фабрики (DirectX: left-handed, Z [0,1]) ---

		static Matrix4x4 translation(float x, float y, float z) noexcept
		{
			Matrix4x4 m;
			// DirectX row-major с вектором-строкой: v * M
			// Translation в последней строке
			m.columns[3][0] = x;
			m.columns[3][1] = y;
			m.columns[3][2] = z;
			return m;
		}

		static Matrix4x4 scale(float x, float y, float z) noexcept
		{
			Matrix4x4 m;
			m.columns[0][0] = x;
			m.columns[1][1] = y;
			m.columns[2][2] = z;
			return m;
		}

		static Matrix4x4 scale(float s) noexcept { return scale(s, s, s); }

		static Matrix4x4 rotationX(float angle) noexcept
		{
			float c = std::cos(angle), s = std::sin(angle);
			Matrix4x4 m;
			// DirectX left-handed rotation around X-axis
			// Row-major matrix layout
			m.columns[1][1] = c;
			m.columns[1][2] = s;
			m.columns[2][1] = -s;
			m.columns[2][2] = c;
			return m;
		}

		static Matrix4x4 rotationY(float angle) noexcept
		{
			float c = std::cos(angle), s = std::sin(angle);
			Matrix4x4 m;
			// DirectX left-handed rotation around Y-axis
			m.columns[0][0] = c;
			m.columns[0][2] = -s;
			m.columns[2][0] = s;
			m.columns[2][2] = c;
			return m;
		}

		static Matrix4x4 rotationZ(float angle) noexcept
		{
			float c = std::cos(angle), s = std::sin(angle);
			Matrix4x4 m;
			// DirectX left-handed rotation around Z-axis
			m.columns[0][0] = c;
			m.columns[0][1] = s;
			m.columns[1][0] = -s;
			m.columns[1][1] = c;
			return m;
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

			// Формула Родрига для row-major, left-handed
			Matrix4x4 m;
			m.columns[0] = Vector4(t * x * x + c, t * x * y + z * s, t * x * z - y * s, 0.0f);
			m.columns[1] = Vector4(t * x * y - z * s, t * y * y + c, t * y * z + x * s, 0.0f);
			m.columns[2] = Vector4(t * x * z + y * s, t * y * z - x * s, t * z * z + c, 0.0f);
			m.columns[3] = Vector4(0.0f, 0.0f, 0.0f, 1.0f);
			return m;
		}

		static Matrix4x4 lookAt(const Vector4& eye, const Vector4& target, const Vector4& up) noexcept
		{
			// DirectX left-handed view matrix для M * v (вектор-столбец)
			Vector4 zAxis = (target - eye).normalized();   // forward (look direction)
			Vector4 xAxis = up.cross3(zAxis).normalized(); // right
			Vector4 yAxis = zAxis.cross3(xAxis);           // up

			// Для M * v нужна транспонированная структура
			Matrix4x4 m;
			m[0] = Vector4(xAxis.x(), yAxis.x(), zAxis.x(), 0.0f);
			m[1] = Vector4(xAxis.y(), yAxis.y(), zAxis.y(), 0.0f);
			m[2] = Vector4(xAxis.z(), yAxis.z(), zAxis.z(), 0.0f);
			m[3] = Vector4(-xAxis.dot(eye), -yAxis.dot(eye), -zAxis.dot(eye), 1.0f);

			return m;
		}

		static Matrix4x4 perspective(float fovY, float aspect, float nearZ, float farZ) noexcept
		{
			// DirectX left-handed perspective projection, Z в [0,1]
			float tanHalfFov = std::tan(fovY * 0.5f);
			Matrix4x4 m(0.0f);

			m.columns[0][0] = 1.0f / (aspect * tanHalfFov);
			m.columns[1][1] = 1.0f / tanHalfFov;
			m.columns[2][2] = farZ / (farZ - nearZ);
			m.columns[2][3] = 1.0f;
			m.columns[3][2] = -(nearZ * farZ) / (farZ - nearZ);
			m.columns[3][3] = 0.0f;

			return m;
		}

		static Matrix4x4 orthographic(float left, float right, float bottom, float top, float nearZ, float farZ) noexcept
		{
			// DirectX left-handed orthographic projection, Z в [0,1]
			Matrix4x4 m;
			m.columns[0][0] = 2.0f / (right - left);
			m.columns[1][1] = 2.0f / (top - bottom);
			m.columns[2][2] = 1.0f / (farZ - nearZ);
			m.columns[3][0] = -(right + left) / (right - left);
			m.columns[3][1] = -(top + bottom) / (top - bottom);
			m.columns[3][2] = -nearZ / (farZ - nearZ);
			m.columns[3][3] = 1.0f;
			return m;
		}

		float determinant() const noexcept
		{
			float m00 = columns[0][0], m01 = columns[0][1], m02 = columns[0][2], m03 = columns[0][3];
			float m10 = columns[1][0], m11 = columns[1][1], m12 = columns[1][2], m13 = columns[1][3];
			float m20 = columns[2][0], m21 = columns[2][1], m22 = columns[2][2], m23 = columns[2][3];
			float m30 = columns[3][0], m31 = columns[3][1], m32 = columns[3][2], m33 = columns[3][3];

			float minor00 = m11 * (m22 * m33 - m23 * m32) - m12 * (m21 * m33 - m23 * m31) + m13 * (m21 * m32 - m22 * m31);
			float minor01 = m10 * (m22 * m33 - m23 * m32) - m12 * (m20 * m33 - m23 * m30) + m13 * (m20 * m32 - m22 * m30);
			float minor02 = m10 * (m21 * m33 - m23 * m31) - m11 * (m20 * m33 - m23 * m30) + m13 * (m20 * m31 - m21 * m30);
			float minor03 = m10 * (m21 * m32 - m22 * m31) - m11 * (m20 * m32 - m22 * m30) + m12 * (m20 * m31 - m21 * m30);

			return m00 * minor00 - m01 * minor01 + m02 * minor02 - m03 * minor03;
		}

		Matrix4x4 inverted() const noexcept
		{
			float m00 = columns[0][0], m01 = columns[0][1], m02 = columns[0][2], m03 = columns[0][3];
			float m10 = columns[1][0], m11 = columns[1][1], m12 = columns[1][2], m13 = columns[1][3];
			float m20 = columns[2][0], m21 = columns[2][1], m22 = columns[2][2], m23 = columns[2][3];
			float m30 = columns[3][0], m31 = columns[3][1], m32 = columns[3][2], m33 = columns[3][3];

			// Вычисляем кофакторы
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
				return Matrix4x4(); // Вырожденная матрица

			float invDet = 1.0f / det;

			Matrix4x4 result;
			result.columns[0] = Vector4(c00 * invDet, c10 * invDet, c20 * invDet, c30 * invDet);
			result.columns[1] = Vector4(c01 * invDet, c11 * invDet, c21 * invDet, c31 * invDet);
			result.columns[2] = Vector4(c02 * invDet, c12 * invDet, c22 * invDet, c32 * invDet);
			result.columns[3] = Vector4(c03 * invDet, c13 * invDet, c23 * invDet, c33 * invDet);

			return result;
		}

		// --- Утилиты ---

		bool operator==(const Matrix4x4& rhs) const noexcept
		{
			return columns[0] == rhs.columns[0] && columns[1] == rhs.columns[1] &&
				columns[2] == rhs.columns[2] && columns[3] == rhs.columns[3];
		}

		bool operator!=(const Matrix4x4& rhs) const noexcept { return !(*this == rhs); }

		std::string to_string() const
		{
			std::string s = "[\n";
			for (int r = 0; r < 4; ++r)
			{
				s += " [";
				for (int c = 0; c < 4; ++c)
				{
					s += std::to_string(columns[r][c]);
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