
#include "test_Vector4.h"

import Math;

using namespace zzz::math;
using namespace zzz::ztests;

test_Vector4::test_Vector4()
{
}

bool test_Vector4::Initialize()
{
	return true;
}

Result<std::string> test_Vector4::Run()
{
	const float epsilon = 1e-6f;

	auto floatEquals = [epsilon](float a, float b) -> bool
		{
			return std::abs(a - b) < epsilon;
		};

	auto vectorEquals = [&floatEquals](const Vector4& a, const Vector4& b) -> bool
		{
			return
				floatEquals(a.x(), b.x()) &&
				floatEquals(a.y(), b.y()) &&
				floatEquals(a.z(), b.z()) &&
				floatEquals(a.w(), b.w());
		};

	// Test 1: Конструкторы
	{
		Vector4 v1;
		if (!vectorEquals(v1, Vector4(0, 0, 0, 0)))
			return Unexpected(L"Failed: Default constructor" );

		Vector4 v2(5.0f);
		if (!vectorEquals(v2, Vector4(5, 5, 5, 5)))
			return Unexpected(L"Failed: Single value constructor");

		Vector4 v3(1, 2, 3, 4);
		if (!floatEquals(v3.x(), 1) || !floatEquals(v3.y(), 2) ||
			!floatEquals(v3.z(), 3) || !floatEquals(v3.w(), 4))
			return Unexpected(L"Failed: Component constructor");
	}

	// Test 2: Getters и setters
	{
		Vector4 v(1, 2, 3, 4);

		if (!floatEquals(v.r(), 1) || !floatEquals(v.g(), 2) ||
			!floatEquals(v.b(), 3) || !floatEquals(v.a(), 4))
			return Unexpected(L"Failed: RGBA getters");

		v.set_x(10);
		v.set_y(20);
		v.set_z(30);
		v.set_w(40);

		if (!vectorEquals(v, Vector4(10, 20, 30, 40)))
			return Unexpected(L"Failed: Component setters");

		v.set_r(5);
		v.set_g(6);
		v.set_b(7);
		v.set_a(8);

		if (!vectorEquals(v, Vector4(5, 6, 7, 8)))
			return Unexpected(L"Failed: RGBA setters");
	}

	// Test 3: Оператор индексации
	{
		Vector4 v(1, 2, 3, 4);

		if (!floatEquals(v[0], 1) || !floatEquals(v[1], 2) ||
			!floatEquals(v[2], 3) || !floatEquals(v[3], 4))
			return Unexpected(L"Failed: Index operator read");

		v[0] = 10;
		v[1] = 20;
		v[2] = 30;
		v[3] = 40;

		if (!vectorEquals(v, Vector4(10, 20, 30, 40)))
			return Unexpected(L"Failed: Index operator write");
	}

	// Test 4: Арифметические операции
	{
		Vector4 v1(1, 2, 3, 4);
		Vector4 v2(5, 6, 7, 8);

		// Сложение
		Vector4 sum = v1 + v2;
		if (!vectorEquals(sum, Vector4(6, 8, 10, 12)))
			return Unexpected(L"Failed: Addition");

		// Вычитание
		Vector4 diff = v2 - v1;
		if (!vectorEquals(diff, Vector4(4, 4, 4, 4)))
			return Unexpected(L"Failed: Subtraction");

		// Умножение на скаляр
		Vector4 scaled = v1 * 2.0f;
		if (!vectorEquals(scaled, Vector4(2, 4, 6, 8)))
			return Unexpected(L"Failed: Scalar multiplication");

		// Деление на скаляр
		Vector4 divided = v1 / 2.0f;
		if (!vectorEquals(divided, Vector4(0.5f, 1.0f, 1.5f, 2.0f)))
			return Unexpected(L"Failed: Scalar division");

		// Покомпонентное умножение
		Vector4 compMul = v1 * v2;
		if (!vectorEquals(compMul, Vector4(5, 12, 21, 32)))
			return Unexpected(L"Failed: Component-wise multiplication");

		// Покомпонентное деление
		Vector4 compDiv = v2 / v1;
		if (!vectorEquals(compDiv, Vector4(5, 3, 7.0f / 3.0f, 2)))
			return Unexpected(L"Failed: Component-wise division");

		// Унарный минус
		Vector4 negated = -v1;
		if (!vectorEquals(negated, Vector4(-1, -2, -3, -4)))
			return Unexpected(L"Failed: Unary negation");
	}

	// Test 5: Операции присваивания
	{
		Vector4 v(1, 2, 3, 4);

		v += Vector4(1, 1, 1, 1);
		if (!vectorEquals(v, Vector4(2, 3, 4, 5)))
			return Unexpected(L"Failed: += operator");

		v -= Vector4(1, 1, 1, 1);
		if (!vectorEquals(v, Vector4(1, 2, 3, 4)))
			return Unexpected(L"Failed: -= operator");

		v *= 2.0f;
		if (!vectorEquals(v, Vector4(2, 4, 6, 8)))
			return Unexpected(L"Failed: *= scalar operator");

		v /= 2.0f;
		if (!vectorEquals(v, Vector4(1, 2, 3, 4)))
			return Unexpected(L"Failed: /= scalar operator");

		v *= Vector4(2, 2, 2, 2);
		if (!vectorEquals(v, Vector4(2, 4, 6, 8)))
			return Unexpected(L"Failed: *= vector operator");

		v /= Vector4(2, 2, 2, 2);
		if (!vectorEquals(v, Vector4(1, 2, 3, 4)))
			return Unexpected(L"Failed: /= vector operator");
	}

	// Test 6: Операции сравнения
	{
		Vector4 v1(1, 2, 3, 4);
		Vector4 v2(1, 2, 3, 4);
		Vector4 v3(5, 6, 7, 8);

		if (!(v1 == v2))
			return Unexpected(L"Failed: Equality operator (equal vectors)");

		if (v1 == v3)
			return Unexpected(L"Failed: Equality operator (different vectors)");

		if (v1 != v2)
			return Unexpected(L"Failed: Inequality operator (equal vectors)");

		if (!(v1 != v3))
			return Unexpected(L"Failed: Inequality operator (different vectors)");
	}

	// Test 7: Длина и нормализация
	{
		Vector4 v(3, 4, 0, 0);

		float lenSq = v.lengthSq();
		if (!floatEquals(lenSq, 25.0f))
			return Unexpected(L"Failed: lengthSq");

		float len = v.length();
		if (!floatEquals(len, 5.0f))
			return Unexpected(L"Failed: length");

		Vector4 normalized = v.normalized();
		if (!floatEquals(normalized.length(), 1.0f))
			return Unexpected(L"Failed: normalized (length not 1)");

		if (!vectorEquals(normalized, Vector4(0.6f, 0.8f, 0, 0)))
			return Unexpected(L"Failed: normalized (wrong direction)");

		Vector4 v2(3, 4, 0, 0);
		v2.normalize();
		if (!floatEquals(v2.length(), 1.0f))
			return Unexpected(L"Failed: normalize (length not 1)");

		// Нормализация нулевого вектора
		Vector4 zero;
		Vector4 normalizedZero = zero.normalized();
		if (!vectorEquals(normalizedZero, Vector4(0, 0, 0, 0)))
			return Unexpected(L"Failed: normalized zero vector");
	}

	// Test 8: Скалярное произведение
	{
		Vector4 v1(1, 2, 3, 4);
		Vector4 v2(5, 6, 7, 8);

		float dotProduct = v1.dot(v2);
		// 1*5 + 2*6 + 3*7 + 4*8 = 5 + 12 + 21 + 32 = 70
		if (!floatEquals(dotProduct, 70.0f))
			return Unexpected(L"Failed: dot product");

		// Перпендикулярные векторы
		Vector4 v3(1, 0, 0, 0);
		Vector4 v4(0, 1, 0, 0);
		if (!floatEquals(v3.dot(v4), 0.0f))
			return Unexpected(L"Failed: dot product (perpendicular)");
	}

	// Test 9: Линейная интерполяция
	{
		Vector4 v1(0, 0, 0, 0);
		Vector4 v2(10, 10, 10, 10);

		Vector4 lerp0 = Vector4::lerp(v1, v2, 0.0f);
		if (!vectorEquals(lerp0, v1))
			return Unexpected(L"Failed: lerp at t=0");

		Vector4 lerp1 = Vector4::lerp(v1, v2, 1.0f);
		if (!vectorEquals(lerp1, v2))
			return Unexpected(L"Failed: lerp at t=1");

		Vector4 lerp05 = Vector4::lerp(v1, v2, 0.5f);
		if (!vectorEquals(lerp05, Vector4(5, 5, 5, 5)))
			return Unexpected(L"Failed: lerp at t=0.5");
	}

	// Test 10: Clamp
	{
		Vector4 v(5, 15, 25, 35);
		Vector4 minVal(10, 10, 10, 10);
		Vector4 maxVal(20, 20, 20, 20);

		Vector4 clamped = v.clamp(minVal, maxVal);
		if (!vectorEquals(clamped, Vector4(10, 15, 20, 20)))
			return Unexpected(L"Failed: clamp (vector bounds)");

		Vector4 clamped2 = v.clamp(10.0f, 20.0f);
		if (!vectorEquals(clamped2, Vector4(10, 15, 20, 20)))
			return Unexpected(L"Failed: clamp (scalar bounds)");
	}

	// Test 11: Component-wise min/max
	{
		Vector4 v1(1, 5, 3, 7);
		Vector4 v2(4, 2, 6, 1);

		Vector4 minVec = v1.compMin(v2);
		if (!vectorEquals(minVec, Vector4(1, 2, 3, 1)))
			return Unexpected(L"Failed: compMin");

		Vector4 maxVec = v1.compMax(v2);
		if (!vectorEquals(maxVec, Vector4(4, 5, 6, 7)))
			return Unexpected(L"Failed: compMax");
	}

	// Test 12: Abs
	{
		Vector4 v(-1, 2, -3, 4);
		Vector4 absVec = v.abs();
		if (!vectorEquals(absVec, Vector4(1, 2, 3, 4)))
			return Unexpected(L"Failed: abs");
	}

	// Test 13: Min/Max component
	{
		Vector4 v(5, 2, 8, 3);

		float minComp = v.minComponent();
		if (!floatEquals(minComp, 2.0f))
			return Unexpected(L"Failed: minComponent");

		float maxComp = v.maxComponent();
		if (!floatEquals(maxComp, 8.0f))
			return Unexpected(L"Failed: maxComponent");
	}

	// Test 14: Reciprocal
	{
		Vector4 v(2, 4, 5, 10);
		Vector4 recip = v.reciprocal();
		if (!vectorEquals(recip, Vector4(0.5f, 0.25f, 0.2f, 0.1f)))
			return Unexpected(L"Failed: reciprocal");
	}

	// Test 15: Sqrt
	{
		Vector4 v(4, 9, 16, 25);
		Vector4 sqrtVec = v.sqrt();
		if (!vectorEquals(sqrtVec, Vector4(2, 3, 4, 5)))
			return Unexpected(L"Failed: sqrt" );
	}

	// Test 16: Rsqrt (обратный квадратный корень)
	{
		Vector4 v(4, 9, 16, 25);
		Vector4 rsqrtVec = v.rsqrt();
		// rsqrt использует приближённые вычисления, поэтому проверяем с большей погрешностью
		const float rsqrt_epsilon = 1e-3f; // Для приближённых SIMD операций

		auto rsqrtEquals = [rsqrt_epsilon](float a, float b) -> bool
			{
				return std::abs(a - b) < rsqrt_epsilon;
			};

		if (!rsqrtEquals(rsqrtVec[0], 0.5f) ||
			!rsqrtEquals(rsqrtVec[1], 1.0f / 3.0f) ||
			!rsqrtEquals(rsqrtVec[2], 0.25f) ||
			!rsqrtEquals(rsqrtVec[3], 0.2f))
			return Unexpected(L"Failed: rsqrt");
	}

	// Test 17: Distance
	{
		Vector4 v1(1, 2, 3, 4);
		Vector4 v2(4, 6, 3, 4);

		float dist = v1.distance(v2);
		// sqrt((4-1) in 2 + (6-2) in 2 + 0 + 0) = sqrt(9 + 16) = 5
		if (!floatEquals(dist, 5.0f))
			return Unexpected(L"Failed: distance");

		float distSq = v1.distanceSq(v2);
		if (!floatEquals(distSq, 25.0f))
			return Unexpected(L"Failed: distanceSq");
	}

	// Test 18: Cross product 3D
	{
		Vector4 v1(1, 0, 0, 0);
		Vector4 v2(0, 1, 0, 0);

		Vector4 cross = v1.cross3(v2);
		if (!vectorEquals(cross, Vector4(0, 0, 1, 0)))
			return Unexpected(L"Failed: cross3 (basic)");

		// Проверка антикоммутативности: a х b = -(b х a)
		Vector4 crossRev = v2.cross3(v1);
		if (!vectorEquals(crossRev, Vector4(0, 0, -1, 0)))
			return Unexpected(L"Failed: cross3 (anticommutativity)");

		// Более сложный случай
		Vector4 v3(1, 2, 3, 0);
		Vector4 v4(4, 5, 6, 0);
		Vector4 cross2 = v3.cross3(v4);
		// (2*6 - 3*5, 3*4 - 1*6, 1*5 - 2*4) = (-3, 6, -3)
		if (!vectorEquals(cross2, Vector4(-3, 6, -3, 0)))
			return Unexpected(L"Failed: cross3 (complex)");
	}

	// Test 19: Scalar multiplication from left
	{
		Vector4 v(1, 2, 3, 4);
		Vector4 scaled = 2.0f * v;
		if (!vectorEquals(scaled, Vector4(2, 4, 6, 8)))
			return Unexpected(L"Failed: left scalar multiplication");
	}

	// Test 20: String representation
	{
		Vector4 v(1.5f, 2.5f, 3.5f, 4.5f);
		std::string str = v.to_string();
		if (str.find("1.5") == std::string::npos ||
			str.find("2.5") == std::string::npos ||
			str.find("3.5") == std::string::npos ||
			str.find("4.5") == std::string::npos)
			return Unexpected(L"Failed: to_string");
	}

	return { "Success" };
}

bool test_Vector4::Kill()
{
	return true;
}