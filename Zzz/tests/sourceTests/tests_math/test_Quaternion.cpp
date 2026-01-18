
#include "test_Quaternion.h"

import Math;

using namespace zzz::math;
using namespace zzz::ztests;

test_Quaternion::test_Quaternion()
{
}

bool test_Quaternion::Initialize()
{
	return true;
}

Result<std::string> test_Quaternion::Run()
{
	const float epsilon = 1e-6f;
	auto floatEquals = [epsilon](float a, float b)
		{
			return std::abs(a - b) < epsilon;
		};
	auto quatEquals = [&](const Quaternion& a, const Quaternion& b)
		{
			return
				floatEquals(a.x, b.x) &&
				floatEquals(a.y, b.y) &&
				floatEquals(a.z, b.z) &&
				floatEquals(a.w, b.w);
		};

	// Test 1: Конструкторы
	{
		Quaternion q1;
		if (!quatEquals(q1, Quaternion(0, 0, 0, 1)))
			return Unexpected(L"Failed: default constructor (identity)");

		Quaternion q2(2.0f);
		if (!quatEquals(q2, Quaternion(2, 2, 2, 2)))
			return Unexpected(L"Failed: single value constructor");

		Quaternion q3(1, 2, 3, 4);
		if (!quatEquals(q3, Quaternion(1, 2, 3, 4)))
			return Unexpected(L"Failed: component constructor");
	}

	// Test 2: Identity
	{
		Quaternion id = Quaternion::identity();
		if (!quatEquals(id, Quaternion(0, 0, 0, 1)))
			return Unexpected(L"Failed: identity()");
	}

	// Test 3: Length
	{
		Quaternion q(1, 2, 3, 4);
		if (!floatEquals(q.lengthSq(), 30.0f))
			return Unexpected(L"Failed: lengthSq");
		if (!floatEquals(q.length(), std::sqrt(30.0f)))
			return Unexpected(L"Failed: length");
	}

	// Test 4: Normalization
	{
		Quaternion q(0, 0, 0, 10);
		Quaternion n = q.normalized();
		if (!floatEquals(n.length(), 1.0f))
			return Unexpected(L"Failed: normalized length");
		if (!quatEquals(n, Quaternion(0, 0, 0, 1)))
			return Unexpected(L"Failed: normalized direction");

		// Нулевой кватернион нормализуется в identity (безопасное поведение)
		Quaternion zero(0, 0, 0, 0);
		Quaternion zn = zero.normalized();
		if (!quatEquals(zn, Quaternion::identity()))
			return Unexpected(L"Failed: normalize zero quaternion");
	}

	// Test 5: Dot product
	{
		Quaternion q1(1, 2, 3, 4);
		Quaternion q2(5, 6, 7, 8);
		// 1*5 + 2*6 + 3*7 + 4*8 = 70
		if (!floatEquals(q1.dot(q2), 70.0f))
			return Unexpected(L"Failed: dot");
	}

	// Test 6: Conjugate
	{
		Quaternion q(1, 2, 3, 4);
		Quaternion c = q.conjugate();
		if (!quatEquals(c, Quaternion(-1, -2, -3, 4)))
			return Unexpected(L"Failed: conjugate");
	}

	// Test 7: Inverse
	{
		Quaternion q = Quaternion::fromAxisAngle(Vector3(0, 1, 0), 1.0f);
		Quaternion inv = q.inverse();
		Quaternion prod = q * inv;
		if (!quatEquals(prod, Quaternion::identity()))
			return Unexpected(L"Failed: inverse");
	}

	// Test 8: Quaternion multiplication (identity)
	{
		Quaternion q = Quaternion::fromAxisAngle(Vector3(1, 0, 0), 0.5f);
		Quaternion r = q * Quaternion::identity();
		if (!quatEquals(r, q))
			return Unexpected(L"Failed: q * identity");

		Quaternion l = Quaternion::identity() * q;
		if (!quatEquals(l, q))
			return Unexpected(L"Failed: identity * q");
	}

	// Test 9: Axis-angle
	{
		Quaternion q = Quaternion::fromAxisAngle(Vector3(0, 0, 1), 0.0f);
		if (!quatEquals(q, Quaternion::identity()))
			return Unexpected(L"Failed: axis-angle zero rotation");
	}

	// Test 10: Vector rotation (используем operator*)
	{
		Vector3 v(1, 0, 0);
		Quaternion q = Quaternion::fromAxisAngle(Vector3(0, 0, 1), 1.57079632679f); // PI/2
		Vector3 r = q * v;
		if (!floatEquals(r.x(), 0.0f) ||
			!floatEquals(r.y(), 1.0f) ||
			!floatEquals(r.z(), 0.0f))
			return Unexpected(L"Failed: vector rotation");
	}

	// Test 11: Euler angles (используем fromEulerXYZ)
	{
		Quaternion q = Quaternion::fromEulerXYZ(0, 1.57079632679f, 0); // pitch, yaw, roll
		Vector3 fwd = q * Vector3(0, 0, 1);
		if (!floatEquals(fwd.x(), 1.0f) ||
			!floatEquals(fwd.z(), 0.0f))
			return Unexpected(L"Failed: fromEulerXYZ");
	}

	// Test 12: Equality
	{
		Quaternion a(1, 2, 3, 4);
		Quaternion b(1, 2, 3, 4);
		Quaternion c(4, 3, 2, 1);
		if (!(a == b))
			return Unexpected(L"Failed: equality");
		if (a != b)
			return Unexpected(L"Failed: inequality (equal)");
		if (a == c)
			return Unexpected(L"Failed: inequality (different)");
	}

	// Test 13: Arithmetic operators
	{
		Quaternion a(1, 2, 3, 4);
		Quaternion b(5, 6, 7, 8);

		// Addition
		Quaternion sum = a + b;
		if (!quatEquals(sum, Quaternion(6, 8, 10, 12)))
			return Unexpected(L"Failed: operator+");

		// Subtraction
		Quaternion diff = b - a;
		if (!quatEquals(diff, Quaternion(4, 4, 4, 4)))
			return Unexpected(L"Failed: operator-");

		// Negation
		Quaternion neg = -a;
		if (!quatEquals(neg, Quaternion(-1, -2, -3, -4)))
			return Unexpected(L"Failed: operator- (unary)");

		// Scalar multiplication
		Quaternion scaled = a * 2.0f;
		if (!quatEquals(scaled, Quaternion(2, 4, 6, 8)))
			return Unexpected(L"Failed: operator* (scalar)");

		// Scalar division
		Quaternion divided = scaled / 2.0f;
		if (!quatEquals(divided, a))
			return Unexpected(L"Failed: operator/ (scalar)");
	}

	// Test 14: Assignment operators
	{
		Quaternion a(1, 2, 3, 4);
		Quaternion b(5, 6, 7, 8);

		a += b;
		if (!quatEquals(a, Quaternion(6, 8, 10, 12)))
			return Unexpected(L"Failed: operator+=");

		a -= Quaternion(1, 2, 3, 4);
		if (!quatEquals(a, Quaternion(5, 6, 7, 8)))
			return Unexpected(L"Failed: operator-=");

		a *= 2.0f;
		if (!quatEquals(a, Quaternion(10, 12, 14, 16)))
			return Unexpected(L"Failed: operator*=");

		a /= 2.0f;
		if (!quatEquals(a, Quaternion(5, 6, 7, 8)))
			return Unexpected(L"Failed: operator/=");
	}

	// Test 15: toEulerAngles roundtrip via vectors (ZYX order: yaw, pitch, roll)
	{
		auto checkEulerZYX = [](float yaw, float pitch, float roll) -> bool
			{
				Quaternion q1 = Quaternion::fromYawPitchRoll(yaw, pitch, roll);
				Vector3 e = q1.toEulerAngles(); // returns { yaw, pitch, roll }
				Quaternion q2 = Quaternion::fromYawPitchRoll(e.x(), e.y(), e.z());

				// Базовые и диагональные векторы
				Vector3 testVectors[] = {
					Vector3(1,0,0), Vector3(0,1,0), Vector3(0,0,1),
					Vector3(1,1,0).normalized(),
					Vector3(0,1,1).normalized(),
					Vector3(1,0,1).normalized()
				};

				const float vecEps = 0.15f; // допустимая погрешность из-за численных ошибок
				for (const Vector3& v : testVectors)
				{
					Vector3 r1 = q1 * v;
					Vector3 r2 = q2 * v;

					float diffSq =
						(r1.x() - r2.x()) * (r1.x() - r2.x()) +
						(r1.y() - r2.y()) * (r1.y() - r2.y()) +
						(r1.z() - r2.z()) * (r1.z() - r2.z());

					if (diffSq > vecEps * vecEps)
						return false;
				}
				return true;
			};

		if (!checkEulerZYX(0.0f, 0.5f, 0.0f)) // pure pitch
			return Unexpected(L"Failed: toEulerAngles pitch");

		if (!checkEulerZYX(0.5f, 0.0f, 0.0f)) // pure yaw
			return Unexpected(L"Failed: toEulerAngles yaw");

		if (!checkEulerZYX(0.0f, 0.0f, 0.5f)) // pure roll
			return Unexpected(L"Failed: toEulerAngles roll");

		if (!checkEulerZYX(0.3f, 0.5f, 0.7f)) // combined angles
			return Unexpected(L"Failed: toEulerAngles combined vectors");

		if (!checkEulerZYX(0.0f, 0.0f, 0.0f)) // identity
			return Unexpected(L"Failed: toEulerAngles identity");
	}

	// Test 16: getAxis / getAngle
	{
		Vector3 axis(0, 1, 0);
		float angle = 1.0f;
		Quaternion q = Quaternion::fromAxisAngle(axis, angle);

		Vector3 extractedAxis = q.getAxis();
		float extractedAngle = q.getAngle();

		if (!floatEquals(extractedAngle, angle))
			return Unexpected(L"Failed: getAngle");

		if (!floatEquals(extractedAxis.x(), axis.x()) ||
			!floatEquals(extractedAxis.y(), axis.y()) ||
			!floatEquals(extractedAxis.z(), axis.z()))
			return Unexpected(L"Failed: getAxis");
	}

	// Test 17: getForward / getUp / getRight
	{
		Quaternion q = Quaternion::identity();

		Vector3 fwd = q.getForward();
		if (!floatEquals(fwd.x(), 0.0f) || !floatEquals(fwd.y(), 0.0f) || !floatEquals(fwd.z(), 1.0f))
			return Unexpected(L"Failed: getForward (identity)");

		Vector3 up = q.getUp();
		if (!floatEquals(up.x(), 0.0f) || !floatEquals(up.y(), 1.0f) || !floatEquals(up.z(), 0.0f))
			return Unexpected(L"Failed: getUp (identity)");

		Vector3 right = q.getRight();
		if (!floatEquals(right.x(), 1.0f) || !floatEquals(right.y(), 0.0f) || !floatEquals(right.z(), 0.0f))
			return Unexpected(L"Failed: getRight (identity)");
	}

	// Test 18: rotateX/Y/Z
	{
		Quaternion qx = Quaternion::rotateX(1.57079632679f); // 90 degrees
		Vector3 vx = qx * Vector3(0, 1, 0);
		if (!floatEquals(vx.y(), 0.0f) || !floatEquals(vx.z(), 1.0f))
			return Unexpected(L"Failed: rotateX");

		Quaternion qy = Quaternion::rotateY(1.57079632679f);
		Vector3 vy = qy * Vector3(1, 0, 0);
		if (!floatEquals(vy.x(), 0.0f) || !floatEquals(vy.z(), -1.0f))
			return Unexpected(L"Failed: rotateY");

		Quaternion qz = Quaternion::rotateZ(1.57079632679f);
		Vector3 vz = qz * Vector3(1, 0, 0);
		if (!floatEquals(vz.x(), 0.0f) || !floatEquals(vz.y(), 1.0f))
			return Unexpected(L"Failed: rotateZ");
	}

	// Test 19: fromToRotation
	{
		Vector3 from(1, 0, 0);
		Vector3 to(0, 1, 0);
		Quaternion q = Quaternion::fromToRotation(from, to);
		Vector3 result = q * from;

		if (!floatEquals(result.x(), to.x()) ||
			!floatEquals(result.y(), to.y()) ||
			!floatEquals(result.z(), to.z()))
			return Unexpected(L"Failed: fromToRotation");
	}

	// Test 20: slerp
	{
		Quaternion a = Quaternion::identity();
		Quaternion b = Quaternion::fromAxisAngle(Vector3(0, 1, 0), 1.57079632679f);

		Quaternion mid = Quaternion::slerp(a, b, 0.5f);
		if (!mid.isNormalized())
			return Unexpected(L"Failed: slerp normalization");

		Quaternion start = Quaternion::slerp(a, b, 0.0f);
		if (!quatEquals(start, a))
			return Unexpected(L"Failed: slerp t=0");

		Quaternion end = Quaternion::slerp(a, b, 1.0f);
		if (!quatEquals(end, b))
			return Unexpected(L"Failed: slerp t=1");
	}

	// Test 21: nlerp
	{
		Quaternion a = Quaternion::identity();
		Quaternion b = Quaternion::fromAxisAngle(Vector3(0, 1, 0), 1.57079632679f);

		Quaternion mid = Quaternion::nlerp(a, b, 0.5f);
		if (!mid.isNormalized())
			return Unexpected(L"Failed: nlerp normalization");
	}

	// Test 22: isValid
	{
		Quaternion valid(1, 2, 3, 4);
		if (!valid.isValid())
			return Unexpected(L"Failed: isValid (valid quaternion)");

		Quaternion invalid(std::numeric_limits<float>::quiet_NaN(), 0, 0, 1);
		if (invalid.isValid())
			return Unexpected(L"Failed: isValid (NaN quaternion)");
	}

	// Test 23: fromLookRotation
	{
		Vector3 forward(1, 0, 0);
		Vector3 up(0, 1, 0);
		Quaternion q = Quaternion::fromLookRotation(forward, up);

		Vector3 resultFwd = q.getForward();
		if (!floatEquals(resultFwd.x(), forward.x()) ||
			!floatEquals(resultFwd.y(), forward.y()) ||
			!floatEquals(resultFwd.z(), forward.z()))
			return Unexpected(L"Failed: fromLookRotation");
	}

	return { "Success" };
}

bool test_Quaternion::Kill()
{
	return true;
}