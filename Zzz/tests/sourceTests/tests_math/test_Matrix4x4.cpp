
#include "test_Matrix4x4.h"

import Math;
import Vector4;
import Matrix4x4;

using namespace zzz::math;
using namespace zzz::ztests;

test_Matrix4x4::test_Matrix4x4()
{
}

bool test_Matrix4x4::Initialize()
{
	return true;
}

Result<std::string> test_Matrix4x4::Run()
{
	const float epsilon = 1e-5f;

	auto floatEquals = [epsilon](float a, float b) -> bool
		{
			return std::abs(a - b) < epsilon;
		};

	auto vec4Equals = [&floatEquals](const Vector4& a, const Vector4& b) -> bool
		{
			return floatEquals(a.x(), b.x()) &&
				floatEquals(a.y(), b.y()) &&
				floatEquals(a.z(), b.z()) &&
				floatEquals(a.w(), b.w());
		};

	// Test 1: Конструктор по умолчанию (единичная матрица)
	{
		Matrix4x4 m;
		if (!floatEquals(m[0][0], 1.0f) || !floatEquals(m[1][1], 1.0f) ||
			!floatEquals(m[2][2], 1.0f) || !floatEquals(m[3][3], 1.0f))
			return Unexpected(L"Failed: Default constructor (identity)");

		if (!floatEquals(m[0][1], 0.0f) || !floatEquals(m[1][0], 0.0f))
			return Unexpected(L"Failed: Default constructor (zeros)");
	}

	// Test 2: Конструктор из векторов-строк
	{
		Vector4 r0(1, 2, 3, 4);
		Vector4 r1(5, 6, 7, 8);
		Vector4 r2(9, 10, 11, 12);
		Vector4 r3(13, 14, 15, 16);
		Matrix4x4 m(r0, r1, r2, r3);

		if (!floatEquals(m[0][0], 1.0f) || !floatEquals(m[0][3], 4.0f) ||
			!floatEquals(m[3][0], 13.0f) || !floatEquals(m[3][3], 16.0f))
			return Unexpected(L"Failed: Row vector constructor");
	}

	// Test 3: Конструктор из 16 скаляров
	{
		Matrix4x4 m(
			1, 2, 3, 4,
			5, 6, 7, 8,
			9, 10, 11, 12,
			13, 14, 15, 16);

		if (!floatEquals(m[0][0], 1.0f) || !floatEquals(m[1][1], 6.0f) ||
			!floatEquals(m[2][2], 11.0f) || !floatEquals(m[3][3], 16.0f))
			return Unexpected(L"Failed: Scalar constructor");
	}

	// Test 4: Доступ к элементам
	{
		Matrix4x4 m;
		m[0][0] = 5.0f;
		m[1][2] = 7.0f;
		m[3][3] = 9.0f;

		if (!floatEquals(m[0][0], 5.0f))
			return Unexpected(L"Failed: Element access [0][0]");

		if (!floatEquals(m[1][2], 7.0f))
			return Unexpected(L"Failed: Element access [1][2]");

		if (!floatEquals(m[3][3], 9.0f))
			return Unexpected(L"Failed: Element access [3][3]");
	}

	// Test 5: Получение строк
	{
		Matrix4x4 m(
			1, 2, 3, 4,
			5, 6, 7, 8,
			9, 10, 11, 12,
			13, 14, 15, 16);

		Vector4 row0 = m.row(0);
		Vector4 row2 = m.row(2);

		if (!vec4Equals(row0, Vector4(1, 2, 3, 4)))
			return Unexpected(L"Failed: row(0)");

		if (!vec4Equals(row2, Vector4(9, 10, 11, 12)))
			return Unexpected(L"Failed: row(2)");
	}

	// Test 6: Получение столбцов
	{
		Matrix4x4 m(
			1, 2, 3, 4,
			5, 6, 7, 8,
			9, 10, 11, 12,
			13, 14, 15, 16);

		Vector4 col0 = m.column(0);
		Vector4 col3 = m.column(3);

		if (!vec4Equals(col0, Vector4(1, 5, 9, 13)))
			return Unexpected(L"Failed: column(0)");

		if (!vec4Equals(col3, Vector4(4, 8, 12, 16)))
			return Unexpected(L"Failed: column(3)");
	}

	// Test 7: Установка строк
	{
		Matrix4x4 m;
		m.setRow(0, Vector4(1, 2, 3, 4));
		m.setRow(2, Vector4(9, 10, 11, 12));

		if (!vec4Equals(m.row(0), Vector4(1, 2, 3, 4)))
			return Unexpected(L"Failed: setRow(0)");

		if (!vec4Equals(m.row(2), Vector4(9, 10, 11, 12)))
			return Unexpected(L"Failed: setRow(2)");
	}

	// Test 8: Установка столбцов
	{
		Matrix4x4 m;
		m.setColumn(0, Vector4(1, 5, 9, 13));
		m.setColumn(3, Vector4(4, 8, 12, 16));

		if (!vec4Equals(m.column(0), Vector4(1, 5, 9, 13)))
			return Unexpected(L"Failed: setColumn(0)");

		if (!vec4Equals(m.column(3), Vector4(4, 8, 12, 16)))
			return Unexpected(L"Failed: setColumn(3)");
	}

	// Test 9: Сложение матриц
	{
		Matrix4x4 m1(
			1, 2, 3, 4,
			5, 6, 7, 8,
			9, 10, 11, 12,
			13, 14, 15, 16);

		Matrix4x4 m2(
			16, 15, 14, 13,
			12, 11, 10, 9,
			8, 7, 6, 5,
			4, 3, 2, 1);

		Matrix4x4 sum = m1 + m2;

		if (!floatEquals(sum[0][0], 17.0f) || !floatEquals(sum[1][1], 17.0f) ||
			!floatEquals(sum[2][2], 17.0f) || !floatEquals(sum[3][3], 17.0f))
			return Unexpected(L"Failed: Matrix addition");
	}

	// Test 10: Вычитание матриц
	{
		Matrix4x4 m1(
			10, 20, 30, 40,
			50, 60, 70, 80,
			90, 100, 110, 120,
			130, 140, 150, 160);

		Matrix4x4 m2(
			1, 2, 3, 4,
			5, 6, 7, 8,
			9, 10, 11, 12,
			13, 14, 15, 16);

		Matrix4x4 diff = m1 - m2;

		if (!floatEquals(diff[0][0], 9.0f) || !floatEquals(diff[1][1], 54.0f) ||
			!floatEquals(diff[2][2], 99.0f) || !floatEquals(diff[3][3], 144.0f))
			return Unexpected(L"Failed: Matrix subtraction");
	}

	// Test 11: Умножение матрицы на скаляр
	{
		Matrix4x4 m(
			1, 2, 3, 4,
			5, 6, 7, 8,
			9, 10, 11, 12,
			13, 14, 15, 16);

		Matrix4x4 scaled = m * 2.0f;

		if (!floatEquals(scaled[0][0], 2.0f) || !floatEquals(scaled[1][1], 12.0f) ||
			!floatEquals(scaled[2][2], 22.0f) || !floatEquals(scaled[3][3], 32.0f))
			return Unexpected(L"Failed: Scalar multiplication");
	}

	// Test 12: Умножение матрицы на вектор
	{
		Matrix4x4 m(
			1, 0, 0, 0,
			0, 2, 0, 0,
			0, 0, 3, 0,
			0, 0, 0, 1);

		Vector4 v(2, 3, 4, 1);
		Vector4 result = m * v;

		if (!vec4Equals(result, Vector4(2, 6, 12, 1)))
			return Unexpected(L"Failed: Matrix-vector multiplication");
	}

	// Test 13: Умножение матриц (единичная матрица)
	{
		Matrix4x4 m(
			1, 2, 3, 4,
			5, 6, 7, 8,
			9, 10, 11, 12,
			13, 14, 15, 16);

		Matrix4x4 identity;
		Matrix4x4 result = m * identity;

		if (!floatEquals(result[0][0], 1.0f) || !floatEquals(result[1][1], 6.0f) ||
			!floatEquals(result[2][2], 11.0f) || !floatEquals(result[3][3], 16.0f))
			return Unexpected(L"Failed: Matrix multiplication with identity");
	}

	// Test 14: Умножение матриц (общий случай)
	{
		Matrix4x4 m1(
			1, 2, 3, 4,
			5, 6, 7, 8,
			9, 10, 11, 12,
			13, 14, 15, 16);

		Matrix4x4 m2(
			2, 0, 0, 0,
			0, 2, 0, 0,
			0, 0, 2, 0,
			0, 0, 0, 2);

		Matrix4x4 result = m1 * m2;

		if (!floatEquals(result[0][0], 2.0f) || !floatEquals(result[1][1], 12.0f) ||
			!floatEquals(result[2][2], 22.0f) || !floatEquals(result[3][3], 32.0f))
			return Unexpected(L"Failed: Matrix multiplication (scaling)");
	}

	// Test 15: Транспонирование
	{
		Matrix4x4 m(
			1, 2, 3, 4,
			5, 6, 7, 8,
			9, 10, 11, 12,
			13, 14, 15, 16);

		Matrix4x4 t = m.transposed();

		if (!floatEquals(t[0][0], 1.0f) || !floatEquals(t[0][1], 2.0f) ||
			!floatEquals(t[1][0], 5.0f) || !floatEquals(t[3][2], 15.0f))
			return Unexpected(L"Failed: Transpose");

		// Проверка, что исходная матрица не изменилась
		if (!floatEquals(m[0][1], 5.0f))
			return Unexpected(L"Failed: Transpose should not modify original");
	}

	// Test 16: Детерминант (единичная матрица)
	{
		Matrix4x4 identity;
		float det = identity.determinant();

		if (!floatEquals(det, 1.0f))
			return Unexpected(L"Failed: Determinant of identity");
	}

	// Test 17: Детерминант (диагональная матрица)
	{
		Matrix4x4 m(
			2, 0, 0, 0,
			0, 3, 0, 0,
			0, 0, 4, 0,
			0, 0, 0, 5);

		float det = m.determinant();

		if (!floatEquals(det, 120.0f)) // 2*3*4*5 = 120
			return Unexpected(L"Failed: Determinant of diagonal matrix");
	}

	// Test 18: Обратная матрица (единичная)
	{
		Matrix4x4 identity;
		Matrix4x4 inv = identity.inverted();

		if (!floatEquals(inv[0][0], 1.0f) || !floatEquals(inv[1][1], 1.0f) ||
			!floatEquals(inv[2][2], 1.0f) || !floatEquals(inv[3][3], 1.0f))
			return Unexpected(L"Failed: Inverse of identity");
	}

	// Test 19: Обратная матрица (масштабирование)
	{
		Matrix4x4 m(
			2, 0, 0, 0,
			0, 2, 0, 0,
			0, 0, 2, 0,
			0, 0, 0, 1);

		Matrix4x4 inv = m.inverted();

		if (!floatEquals(inv[0][0], 0.5f) || !floatEquals(inv[1][1], 0.5f) ||
			!floatEquals(inv[2][2], 0.5f) || !floatEquals(inv[3][3], 1.0f))
			return Unexpected(L"Failed: Inverse of scale matrix");
	}

	// Test 20: Произведение матрицы и её обратной
	{
		Matrix4x4 m(
			2, 1, 0, 0,
			0, 3, 1, 0,
			0, 0, 4, 1,
			0, 0, 0, 5);

		Matrix4x4 inv = m.inverted();
		Matrix4x4 product = m * inv;

		if (!floatEquals(product[0][0], 1.0f) || !floatEquals(product[1][1], 1.0f) ||
			!floatEquals(product[2][2], 1.0f) || !floatEquals(product[3][3], 1.0f))
			return Unexpected(L"Failed: M * M^-1 diagonal");

		if (!floatEquals(product[0][1], 0.0f) || !floatEquals(product[1][0], 0.0f))
			return Unexpected(L"Failed: M * M^-1 off-diagonal");
	}

	// Test 21: Создание матрицы переноса
	{
		Matrix4x4 t = Matrix4x4::translation(5, 10, 15);
		Vector4 point(1, 2, 3, 1);
		Vector4 result = t * point;

		if (!vec4Equals(result, Vector4(6, 12, 18, 1)))
			return Unexpected(L"Failed: Translation matrix");
	}

	// Test 22: Создание матрицы масштабирования
	{
		Matrix4x4 s = Matrix4x4::scale(2, 3, 4);
		Vector4 point(1, 1, 1, 1);
		Vector4 result = s * point;

		if (!vec4Equals(result, Vector4(2, 3, 4, 1)))
			return Unexpected(L"Failed: Scaling matrix");
	}

	// Test 23: Матрица вращения вокруг X
	{
		Matrix4x4 rx = Matrix4x4::rotationX(Pi / 2.0f); // 90 градусов
		Vector4 point(0, 1, 0, 1);
		Vector4 result = rx * point;

		if (!floatEquals(result.x(), 0.0f) || !floatEquals(result.y(), 0.0f) ||
			!floatEquals(result.z(), 1.0f))
			return Unexpected(L"Failed: Rotation X matrix");
	}

	// Test 24: Матрица вращения вокруг Y
	{
		Matrix4x4 ry = Matrix4x4::rotationY(Pi / 2.0f); // 90 градусов
		Vector4 point(1, 0, 0, 1);
		Vector4 result = ry * point;

		if (!floatEquals(result.x(), 0.0f) || !floatEquals(result.y(), 0.0f) ||
			!floatEquals(result.z(), -1.0f))
			return Unexpected(L"Failed: Rotation Y matrix");
	}

	// Test 25: Матрица вращения вокруг Z
	{
		Matrix4x4 rz = Matrix4x4::rotationZ(Pi / 2.0f); // 90 градусов
		Vector4 point(1, 0, 0, 1);
		Vector4 result = rz * point;

		if (!floatEquals(result.x(), 0.0f) || !floatEquals(result.y(), 1.0f) ||
			!floatEquals(result.z(), 0.0f))
			return Unexpected(L"Failed: Rotation Z matrix");
	}

	// Test 26: Операции присваивания
	{
		Matrix4x4 m(
			1, 2, 3, 4,
			5, 6, 7, 8,
			9, 10, 11, 12,
			13, 14, 15, 16);

		Matrix4x4 m2;
		m2 += m;

		if (!floatEquals(m2[0][0], 2.0f) || !floatEquals(m2[1][1], 7.0f))
			return Unexpected(L"Failed: += operator");

		m2 -= m;

		if (!floatEquals(m2[0][0], 1.0f) || !floatEquals(m2[1][1], 1.0f))
			return Unexpected(L"Failed: -= operator");

		m2 *= 2.0f;

		if (!floatEquals(m2[0][0], 2.0f) || !floatEquals(m2[1][1], 2.0f))
			return Unexpected(L"Failed: *= scalar operator");
	}

	// Test 27: Перспективная матрица (базовая проверка)
	{
		Matrix4x4 proj = Matrix4x4::perspective(Pi / 2.0f, 16.0f / 9.0f, 0.1f, 100.0f);

		// Проверяем, что это не нулевая матрица
		if (floatEquals(proj[0][0], 0.0f) && floatEquals(proj[1][1], 0.0f))
			return Unexpected(L"Failed: Perspective matrix (zero)");

#if defined(ZRENDER_API_D3D12)
		// Для DirectX проверяем proj[2][3] = 1 (перспективное деление для left-handed)
		if (!floatEquals(proj[2][3], 1.0f))
			return Unexpected(L"Failed: Perspective matrix [2][3] (DirectX)");
#else
		// Для OpenGL/Vulkan/Metal проверяем proj[2][3] = -1 (перспективное деление для right-handed)
		if (!floatEquals(proj[2][3], -1.0f))
			return Unexpected(L"Failed: Perspective matrix [2][3] (OpenGL)");
#endif

		// Проверяем что proj[3][3] = 0 (характерно для перспективной проекции)
		if (!floatEquals(proj[3][3], 0.0f))
			return Unexpected(L"Failed: Perspective matrix [3][3]");
	}

	// Test 28: Ортографическая матрица
	{
		Matrix4x4 ortho = Matrix4x4::orthographic(-10, 10, -10, 10, 0.1f, 100.0f);

		// Проверяем диагональные элементы масштабирования
		if (floatEquals(ortho[0][0], 0.0f) || floatEquals(ortho[1][1], 0.0f))
			return Unexpected(L"Failed: Orthographic matrix");
	}

	// Test 29: LookAt матрица
	{
		Vector4 eye(0, 0, 5, 1);
		Vector4 center(0, 0, 0, 1);
		Vector4 up(0, 1, 0, 0);
		Matrix4x4 view = Matrix4x4::lookAt(eye, center, up);

		// Проверяем, что точка в центре правильно трансформирована
		Vector4 transformed = view * center;

#if defined(ZRENDER_API_D3D12)
		// DirectX (left-handed): камера в (0,0,5) смотрит вдоль +Z
		// Точка (0,0,0) находится ВПЕРЕДИ камеры на расстоянии 5
		// В view space: z = +5
		if (!floatEquals(transformed.z(), 5.0f))
			return Unexpected(L"Failed: LookAt matrix (DirectX)");
#else
		// Metal/Vulkan (right-handed): камера смотрит вдоль -Z
		// Точка (0,0,0) находится ВПЕРЕДИ камеры на расстоянии 5
		// В view space: z = -5
		if (!floatEquals(transformed.z(), -5.0f))
			return Unexpected(L"Failed: LookAt matrix (OpenGL)");
#endif
	}

	// Test 30: Сравнение матриц
	{
		Matrix4x4 m1(
			1, 2, 3, 4,
			5, 6, 7, 8,
			9, 10, 11, 12,
			13, 14, 15, 16);

		Matrix4x4 m2(
			1, 2, 3, 4,
			5, 6, 7, 8,
			9, 10, 11, 12,
			13, 14, 15, 16);

		Matrix4x4 m3(
			16, 15, 14, 13,
			12, 11, 10, 9,
			8, 7, 6, 5,
			4, 3, 2, 1);

		if (!(m1 == m2))
			return Unexpected(L"Failed: Matrix equality (equal)");

		if (m1 == m3)
			return Unexpected(L"Failed: Matrix equality (different)");

		if (m1 != m2)
			return Unexpected(L"Failed: Matrix inequality (equal)");

		if (!(m1 != m3))
			return Unexpected(L"Failed: Matrix inequality (different)");
	}

	return { "Success" };
}

bool test_Matrix4x4::Kill()
{
	return true;
}