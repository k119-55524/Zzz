#include "test_Vector3.h"

import Math;

using namespace zzz::math;
using namespace zzz::ztests;

test_Vector3::test_Vector3() = default;

bool test_Vector3::Initialize()
{
	return true;
}

Result<std::string> test_Vector3::Run()
{
	const float epsilon = 1e-6f;

	auto floatEquals = [epsilon](float a, float b) -> bool
		{
			return std::abs(a - b) < epsilon;
		};

	auto vectorEquals = [&floatEquals](const Vector3& a, const Vector3& b) -> bool
		{
			return
				floatEquals(a.x(), b.x()) &&
				floatEquals(a.y(), b.y()) &&
				floatEquals(a.z(), b.z());
		};

	// ---------------- Test 1: Constructors ----------------
	{
		Vector3 v1;
		if (!vectorEquals(v1, Vector3(0, 0, 0)))
			return Unexpected(L"Failed: Default constructor");

		Vector3 v2(5.0f);
		if (!vectorEquals(v2, Vector3(5, 5, 5)))
			return Unexpected(L"Failed: Single value constructor");

		Vector3 v3(1, 2, 3);
		if (!vectorEquals(v3, Vector3(1, 2, 3)))
			return Unexpected(L"Failed: Component constructor");
	}

	// ---------------- Test 2: Getters / Setters ----------------
	{
		Vector3 v(1, 2, 3);

		if (!floatEquals(v.x(), 1) ||
			!floatEquals(v.y(), 2) ||
			!floatEquals(v.z(), 3))
			return Unexpected(L"Failed: Getters");

		v.set_x(10);
		v.set_y(20);
		v.set_z(30);

		if (!vectorEquals(v, Vector3(10, 20, 30)))
			return Unexpected(L"Failed: Setters");
	}

	// ---------------- Test 3: Index operator ----------------
	{
		Vector3 v(1, 2, 3);

		if (!floatEquals(v[0], 1) ||
			!floatEquals(v[1], 2) ||
			!floatEquals(v[2], 3))
			return Unexpected(L"Failed: Index operator read");

		v[0] = 7;
		v[1] = 8;
		v[2] = 9;

		if (!vectorEquals(v, Vector3(7, 8, 9)))
			return Unexpected(L"Failed: Index operator write");
	}

	// ---------------- Test 4: Arithmetic ----------------
	{
		Vector3 v1(1, 2, 3);
		Vector3 v2(4, 5, 6);

		if (!vectorEquals(v1 + v2, Vector3(5, 7, 9)))
			return Unexpected(L"Failed: Addition");

		if (!vectorEquals(v2 - v1, Vector3(3, 3, 3)))
			return Unexpected(L"Failed: Subtraction");

		if (!vectorEquals(v1 * 2.0f, Vector3(2, 4, 6)))
			return Unexpected(L"Failed: Scalar multiplication");

		if (!vectorEquals(v2 / 2.0f, Vector3(2, 2.5f, 3)))
			return Unexpected(L"Failed: Scalar division");
	}

	// ---------------- Test 5: Assignment operators ----------------
	{
		Vector3 v(1, 2, 3);

		v += Vector3(1, 1, 1);
		if (!vectorEquals(v, Vector3(2, 3, 4)))
			return Unexpected(L"Failed: +=");

		v -= Vector3(1, 1, 1);
		if (!vectorEquals(v, Vector3(1, 2, 3)))
			return Unexpected(L"Failed: -=");

		v *= 2.0f;
		if (!vectorEquals(v, Vector3(2, 4, 6)))
			return Unexpected(L"Failed: *=");

		v /= 2.0f;
		if (!vectorEquals(v, Vector3(1, 2, 3)))
			return Unexpected(L"Failed: /=");
	}

	// ---------------- Test 6: Comparison ----------------
	{
		Vector3 a(1, 2, 3);
		Vector3 b(1, 2, 3);
		Vector3 c(4, 5, 6);

		if (!(a == b))
			return Unexpected(L"Failed: Equality");

		if (a == c)
			return Unexpected(L"Failed: Equality false positive");

		if (a != b)
			return Unexpected(L"Failed: Inequality");

		if (!(a != c))
			return Unexpected(L"Failed: Inequality false negative");
	}

	// ---------------- Test 7: Length & normalization ----------------
	{
		Vector3 v(3, 4, 0);

		if (!floatEquals(v.lengthSq(), 25.0f))
			return Unexpected(L"Failed: lengthSq");

		if (!floatEquals(v.length(), 5.0f))
			return Unexpected(L"Failed: length");

		Vector3 n = v.normalized();
		if (!floatEquals(n.length(), 1.0f))
			return Unexpected(L"Failed: normalized length");

		if (!vectorEquals(n, Vector3(0.6f, 0.8f, 0)))
			return Unexpected(L"Failed: normalized direction");

		Vector3 zero;
		if (!vectorEquals(zero.normalized(), Vector3(0, 0, 0)))
			return Unexpected(L"Failed: normalize zero");
	}

	// ---------------- Test 8: Dot product ----------------
	{
		Vector3 a(1, 2, 3);
		Vector3 b(4, 5, 6);

		// 1*4 + 2*5 + 3*6 = 32
		if (!floatEquals(a.dot(b), 32.0f))
			return Unexpected(L"Failed: dot");

		Vector3 x(1, 0, 0);
		Vector3 y(0, 1, 0);
		if (!floatEquals(x.dot(y), 0.0f))
			return Unexpected(L"Failed: dot perpendicular");
	}

	// ---------------- Test 9: Cross product ----------------
	{
		Vector3 x(1, 0, 0);
		Vector3 y(0, 1, 0);

		Vector3 z = x.cross(y);
		if (!vectorEquals(z, Vector3(0, 0, 1)))
			return Unexpected(L"Failed: cross basic");

		Vector3 z2 = y.cross(x);
		if (!vectorEquals(z2, Vector3(0, 0, -1)))
			return Unexpected(L"Failed: cross anti-commutative");

		Vector3 a(1, 2, 3);
		Vector3 b(4, 5, 6);
		Vector3 c = a.cross(b);

		// (-3, 6, -3)
		if (!vectorEquals(c, Vector3(-3, 6, -3)))
			return Unexpected(L"Failed: cross complex");
	}

	// ---------------- Test 10: Distance ----------------
	{
		Vector3 a(1, 2, 3);
		Vector3 b(4, 6, 3);

		if (!floatEquals(a.distance(b), 5.0f))
			return Unexpected(L"Failed: distance");

		if (!floatEquals(a.distanceSq(b), 25.0f))
			return Unexpected(L"Failed: distanceSq");
	}

	// ---------------- Test 11: Left scalar multiply ----------------
	{
		Vector3 v(1, 2, 3);
		if (!vectorEquals(2.0f * v, Vector3(2, 4, 6)))
			return Unexpected(L"Failed: left scalar multiply");
	}

	// ---------------- Test 12: to_string ----------------
	{
		Vector3 v(1.5f, 2.5f, 3.5f);
		std::string s = v.to_string();

		if (s.find("1.5") == std::string::npos ||
			s.find("2.5") == std::string::npos ||
			s.find("3.5") == std::string::npos)
			return Unexpected(L"Failed: to_string");
	}

	return { "Success" };
}

bool test_Vector3::Kill()
{
	return true;
}
