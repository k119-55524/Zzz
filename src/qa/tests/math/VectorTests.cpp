#include "qa/tests/TestsConfig.h"

#ifdef Z_TEST_MATH_VECTORS

#include <gtest/gtest.h>
#include "math/Math.h"

using namespace zzz;
using namespace zzz::math;

TEST(Vec2Test, BasicConstructionAndAccess)
{
	Vec2f v0;
	EXPECT_FLOAT_EQ(v0.x, 0.0f);
	EXPECT_FLOAT_EQ(v0.y, 0.0f);

	Vec2f v1(5.0f);
	EXPECT_FLOAT_EQ(v1.x, 5.0f);
	EXPECT_FLOAT_EQ(v1.y, 5.0f);

	Vec2f v2(1.0f, 2.0f);
	EXPECT_FLOAT_EQ(v2.x, 1.0f);
	EXPECT_FLOAT_EQ(v2.y, 2.0f);

	EXPECT_FLOAT_EQ(v2[0], 1.0f);
	EXPECT_FLOAT_EQ(v2[1], 2.0f);
	EXPECT_EQ(v2.data(), &v2.x);

	Vec2i vi(10, 20);
	Vec2d vd(vi);
	EXPECT_DOUBLE_EQ(vd.x, 10.0);
	EXPECT_DOUBLE_EQ(vd.y, 20.0);
}

TEST(Vec2Test, ArithmeticOperators)
{
	Vec2f a(1.0f, 2.0f);
	Vec2f b(3.0f, 4.0f);

	Vec2f add = a + b;
	EXPECT_FLOAT_EQ(add.x, 4.0f);
	EXPECT_FLOAT_EQ(add.y, 6.0f);

	Vec2f sub = b - a;
	EXPECT_FLOAT_EQ(sub.x, 2.0f);
	EXPECT_FLOAT_EQ(sub.y, 2.0f);

	Vec2f mul = a * b;
	EXPECT_FLOAT_EQ(mul.x, 3.0f);
	EXPECT_FLOAT_EQ(mul.y, 8.0f);

	Vec2f div = b / a;
	EXPECT_FLOAT_EQ(div.x, 3.0f);
	EXPECT_FLOAT_EQ(div.y, 2.0f);

	Vec2f scaled = a * 2.0f;
	EXPECT_FLOAT_EQ(scaled.x, 2.0f);
	EXPECT_FLOAT_EQ(scaled.y, 4.0f);

	Vec2f neg = -a;
	EXPECT_FLOAT_EQ(neg.x, -1.0f);
	EXPECT_FLOAT_EQ(neg.y, -2.0f);
}

TEST(Vec2Test, GeometricFunctions)
{
	Vec2f a(3.0f, 4.0f);
	EXPECT_FLOAT_EQ(a.LengthSquared(), 25.0f);
	EXPECT_FLOAT_EQ(a.Length(), 5.0f);

	Vec2f n = a.Normalized();
	EXPECT_FLOAT_EQ(n.Length(), 1.0f);
	EXPECT_FLOAT_EQ(n.x, 0.6f);
	EXPECT_FLOAT_EQ(n.y, 0.8f);

	Vec2f zero(0.0f, 0.0f);
	zero.Normalize();
	EXPECT_FLOAT_EQ(zero.x, 0.0f);
	EXPECT_FLOAT_EQ(zero.y, 0.0f);

	Vec2f b(1.0f, 0.0f);
	Vec2f c(0.0f, 1.0f);
	EXPECT_FLOAT_EQ(Dot(b, c), 0.0f);

	Vec2f lerped = Lerp(Vec2f(0.0f, 0.0f), Vec2f(10.0f, 20.0f), 0.5f);
	EXPECT_FLOAT_EQ(lerped.x, 5.0f);
	EXPECT_FLOAT_EQ(lerped.y, 10.0f);

	std::string str = std::format("{}", a);
	EXPECT_EQ(str, "Vec2(3, 4)");
}

TEST(Vec3Test, DirectionConstantsAndLeftHandedCross)
{
	EXPECT_EQ(Vec3f::Zero(), Vec3f(0.0f, 0.0f, 0.0f));
	EXPECT_EQ(Vec3f::One(), Vec3f(1.0f, 1.0f, 1.0f));
	EXPECT_EQ(Vec3f::Up(), Vec3f(0.0f, 1.0f, 0.0f));
	EXPECT_EQ(Vec3f::Down(), Vec3f(0.0f, -1.0f, 0.0f));
	EXPECT_EQ(Vec3f::Left(), Vec3f(-1.0f, 0.0f, 0.0f));
	EXPECT_EQ(Vec3f::Right(), Vec3f(1.0f, 0.0f, 0.0f));
	EXPECT_EQ(Vec3f::Forward(), Vec3f(0.0f, 0.0f, 1.0f));
	EXPECT_EQ(Vec3f::Back(), Vec3f(0.0f, 0.0f, -1.0f));

	// В левосторонней системе: Right(1,0,0) x Up(0,1,0) = Forward(0,0,1)
	Vec3f right = Vec3f::Right();
	Vec3f up = Vec3f::Up();
	Vec3f crossRes = Cross(right, up);
	EXPECT_EQ(crossRes, Vec3f::Forward());

	// Up(0,1,0) x Forward(0,0,1) = Right(1,0,0)
	EXPECT_EQ(Cross(up, Vec3f::Forward()), right);
}

TEST(Vec3Test, ArithmeticAndNormalization)
{
	Vec3f v(2.0f, 3.0f, 6.0f);
	EXPECT_FLOAT_EQ(v.LengthSquared(), 49.0f);
	EXPECT_FLOAT_EQ(v.Length(), 7.0f);

	Vec3f norm = v.Normalized();
	EXPECT_FLOAT_EQ(norm.Length(), 1.0f);
	EXPECT_NEAR(norm.x, 2.0f / 7.0f, 1e-5f);
	EXPECT_NEAR(norm.y, 3.0f / 7.0f, 1e-5f);
	EXPECT_NEAR(norm.z, 6.0f / 7.0f, 1e-5f);

	Vec3f reflected = Vec3f(1.0f, -1.0f, 0.0f).Reflect(Vec3f::Up());
	EXPECT_NEAR(reflected.x, 1.0f, 1e-5f);
	EXPECT_NEAR(reflected.y, 1.0f, 1e-5f);
	EXPECT_NEAR(reflected.z, 0.0f, 1e-5f);
}

TEST(Vec4Test, BasicOperations)
{
	Vec4f v(1.0f, 2.0f, 3.0f, 4.0f);
	EXPECT_FLOAT_EQ(v.x, 1.0f);
	EXPECT_FLOAT_EQ(v.y, 2.0f);
	EXPECT_FLOAT_EQ(v.z, 3.0f);
	EXPECT_FLOAT_EQ(v.w, 4.0f);

	Vec3f v3(1.0f, 2.0f, 3.0f);
	Vec4f v4From3(v3, 1.0f);
	EXPECT_EQ(v4From3, Vec4f(1.0f, 2.0f, 3.0f, 1.0f));

	Vec4f lerped = Lerp(Vec4f(0.0f), Vec4f(10.0f, 20.0f, 30.0f, 40.0f), 0.25f);
	EXPECT_FLOAT_EQ(lerped.x, 2.5f);
	EXPECT_FLOAT_EQ(lerped.y, 5.0f);
	EXPECT_FLOAT_EQ(lerped.z, 7.5f);
	EXPECT_FLOAT_EQ(lerped.w, 10.0f);
}

#endif // Z_TEST_MATH_VECTORS
